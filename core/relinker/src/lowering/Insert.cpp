#include <anybox/relinker/Lowering.hpp>
#include <initializer_list>
#include <stdexcept>

namespace {
unsigned Register(const ZydisDecodedOperand& operand) {
    if (operand.type != ZYDIS_OPERAND_TYPE_REGISTER || operand.reg.value < ZYDIS_REGISTER_XMM0 || operand.reg.value > ZYDIS_REGISTER_XMM15) throw std::invalid_argument("INSERTQ operand is not an AMD64 XMM register");
    return static_cast<unsigned>(operand.reg.value - ZYDIS_REGISTER_XMM0);
}

class Builder {
public:
    void Bytes(std::initializer_list<std::uint8_t> bytes) {
        for (auto value : bytes) code.push_back(static_cast<std::byte>(value));
    }

    void Vector(unsigned prefix, unsigned opcode, unsigned destination, unsigned source) {
        Bytes({static_cast<std::uint8_t>(prefix)});
        auto rex = 0x40 | (destination >= 8 ? 4 : 0) | (source >= 8 ? 1 : 0);
        if (rex != 0x40) Bytes({static_cast<std::uint8_t>(rex)});
        Bytes({0x0F, static_cast<std::uint8_t>(opcode), static_cast<std::uint8_t>(0xC0 | ((destination & 7) << 3) | (source & 7))});
    }

    void Spill(unsigned reg, unsigned offset, bool load) {
        Bytes({0xF3, 0x0F, static_cast<std::uint8_t>(load ? 0x6F : 0x7F), static_cast<std::uint8_t>(0x44 | (reg << 3)), 0x24, static_cast<std::uint8_t>(offset)});
    }

    void Shift(unsigned reg, unsigned operation, unsigned count) {
        Bytes({0x66, 0x0F, 0x73, static_cast<std::uint8_t>(0xC0 | (operation << 3) | reg), static_cast<std::uint8_t>(count)});
    }

    std::vector<std::byte> code;
};
}

AnyBox::Relinker::LoweredFunction AnyBox::Relinker::LowerImmediateInsert(const DecodedInstruction& instruction, std::uint8_t stackSize) {
    if (stackSize != 32 && stackSize != 40) throw std::invalid_argument("INSERTQ helper requires a 32-byte or 40-byte stack frame");
    if (instruction.native.mnemonic != ZYDIS_MNEMONIC_INSERTQ || instruction.native.operand_count_visible != 4 || instruction.operands[2].type != ZYDIS_OPERAND_TYPE_IMMEDIATE || instruction.operands[3].type != ZYDIS_OPERAND_TYPE_IMMEDIATE) throw std::invalid_argument("Lowering requires an immediate INSERTQ instruction");
    auto destination = Register(instruction.operands[0]);
    auto source = Register(instruction.operands[1]);
    auto length = static_cast<unsigned>(instruction.operands[2].imm.value.u & 63);
    auto index = static_cast<unsigned>(instruction.operands[3].imm.value.u & 63);
    if (length == 0) length = 64;
    if (length + index > 64) throw std::invalid_argument("INSERTQ field extends beyond its defined low quadword");

    std::array<unsigned, 2> scratch{};
    unsigned count = 0;
    for (unsigned reg = 0; reg < 6 && count < scratch.size(); ++reg) {
        if (reg != destination && reg != source) scratch[count++] = reg;
    }
    if (count != scratch.size()) throw std::logic_error("INSERTQ scratch register selection failed");
    auto value = scratch[0];
    auto mask = scratch[1];
    Builder builder;
    builder.Bytes({0x48, 0x8D, 0x64, 0x24, static_cast<std::uint8_t>(0x100 - stackSize)});
    builder.Spill(value, 0, false);
    builder.Spill(mask, 16, false);
    builder.Vector(0x66, 0x6F, value, source);
    builder.Vector(0x66, 0x76, mask, mask);
    builder.Shift(mask, 2, 64 - length);
    builder.Vector(0x66, 0xDB, value, mask);
    builder.Shift(value, 6, index);
    builder.Shift(mask, 6, index);
    builder.Vector(0x66, 0xDF, mask, destination);
    builder.Vector(0x66, 0xEB, value, mask);
    builder.Vector(0xF3, 0x7E, destination, value);
    builder.Spill(value, 0, true);
    builder.Spill(mask, 16, true);
    builder.Bytes({0x48, 0x8D, 0x64, 0x24, stackSize});
    auto epilog = static_cast<std::uint32_t>(builder.code.size());
    builder.Bytes({0xC3});
    return {std::move(builder.code), {{0, epilog, 5, stackSize}, {epilog, epilog + 1, 0, 0}}};
}
