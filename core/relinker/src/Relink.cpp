#include <anybox/relinker/Relink.hpp>
#include <anybox/relinker/Decoder.hpp>
#include <algorithm>
#include <string>

AnyBox::Relinker::RelinkResult AnyBox::Relinker::Relink(Image& image, const Cpu& cpu, bool forceCacheWriteback) {
    Decoder decoder;
    RelinkResult result;
    auto entries = image.EntryPoints();
    std::vector<std::size_t> interiorEntries;
    std::vector<std::uint8_t> visited(image.GetBytes().size());
    while (!entries.empty()) {
        auto address = entries.back();
        entries.pop_back();
        while (image.IsExecutable(address)) {
            auto offset = image.FileOffset(address, 1);
            if (visited[offset] != 0) {
                if (visited[offset] == 2) interiorEntries.push_back(offset);
                break;
            }
            auto code = image.Code(address);
            DecodedInstruction instruction;
            try { instruction = decoder.Decode(code); }
            catch (const std::exception& error) { throw std::runtime_error("PE instruction RVA " + std::to_string(address) + ": " + error.what()); }
            auto length = instruction.native.length;
            for (std::size_t index = 0; index < length; ++index) {
                if (visited[offset + index] != 0) throw std::runtime_error("PE decoded instruction ranges overlap");
                visited[offset + index] = index == 0 ? 1 : 2;
            }
            ++result.instructionCount;
            auto category = instruction.native.meta.category;
            if (category == ZYDIS_CATEGORY_CALL || category == ZYDIS_CATEGORY_COND_BR || category == ZYDIS_CATEGORY_UNCOND_BR) {
                for (std::size_t index = 0; index < instruction.native.operand_count_visible; ++index) {
                    const auto& operand = instruction.operands[index];
                    if (operand.type != ZYDIS_OPERAND_TYPE_IMMEDIATE || !operand.imm.is_relative) continue;
                    ZyanU64 target = 0;
                    if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(&instruction.native, &operand, image.GetBase() + address, &target))) throw std::runtime_error("Cannot resolve PE control flow target");
                    if (target >= image.GetBase() && target - image.GetBase() <= UINT32_MAX && image.IsExecutable(static_cast<std::uint32_t>(target - image.GetBase()))) entries.push_back(static_cast<std::uint32_t>(target - image.GetBase()));
                }
            }
            if (!cpu.Supports(instruction) || (forceCacheWriteback && instruction.native.mnemonic == ZYDIS_MNEMONIC_CLWB)) {
                auto replacement = decoder.LowerWriteback(instruction, code);
                if (!replacement) cpu.Require(instruction, address);
                auto bytes = std::span(replacement->bytes).first(replacement->length);
                cpu.Require(decoder.Decode(bytes), address);
                std::ranges::copy(bytes, image.GetBytes().begin() + static_cast<std::ptrdiff_t>(offset));
                result.writebackAddresses.push_back(address);
            }
            if (category == ZYDIS_CATEGORY_UNCOND_BR || category == ZYDIS_CATEGORY_RET || instruction.native.mnemonic == ZYDIS_MNEMONIC_INT3 || instruction.native.mnemonic == ZYDIS_MNEMONIC_UD2) break;
            if (address > UINT32_MAX - length) throw std::runtime_error("PE instruction address overflows");
            address += length;
        }
    }
    std::ranges::sort(interiorEntries);
    for (auto address : result.writebackAddresses) {
        auto offset = image.FileOffset(address, 1);
        auto length = decoder.Decode(image.Code(address)).native.length;
        auto found = std::ranges::upper_bound(interiorEntries, offset);
        if (found != interiorEntries.end() && *found < offset + length) throw std::runtime_error("A control flow entry overlaps a lowered cache instruction");
    }
    ZydisDecoder scanner{};
    if (!ZYAN_SUCCESS(ZydisDecoderInit(&scanner, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64))) throw std::runtime_error("Cannot initialize the PE cache instruction scan");
    for (const auto& section : image.GetSections()) {
        if ((section.flags & 0x20000000) == 0 || section.size == 0) continue;
        auto code = image.Code(section.address);
        for (std::size_t index = 0; index < code.size(); ++index) {
            if (visited[section.offset + index] != 0 || code[index] != std::byte{0x66}) continue;
            ZydisDecodedInstruction candidate{};
            if (ZYAN_SUCCESS(ZydisDecoderDecodeInstruction(&scanner, nullptr, code.data() + index, code.size() - index, &candidate)) && candidate.mnemonic == ZYDIS_MNEMONIC_CLWB) throw std::runtime_error("Unclassified CLWB at PE RVA " + std::to_string(section.address + index));
        }
    }
    if (!result.writebackAddresses.empty()) image.Finalize();
    return result;
}
