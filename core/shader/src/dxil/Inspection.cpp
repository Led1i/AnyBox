#include <anybox/shader/Inspection.hpp>
#include <anybox/graphics/container/Dxil.hpp>
#include "Session.hpp"
#include <memory>

namespace {
struct Release {
    void operator()(dxil_spv_parsed_blob value) const noexcept { dxil_spv_parsed_blob_free(value); }
};

void Check(dxil_spv_result result, const char* operation) {
    if (result != DXIL_SPV_SUCCESS) throw std::runtime_error(std::string(operation) + " failed with DXIL compiler status " + std::to_string(result));
}
}

std::string AnyBox::Shader::InspectDxil(std::span<const std::byte> code) {
    Graphics::Container::DxbcView container(code);
    auto part = container.FindPart(0x4C495844);
    if (!part) throw std::invalid_argument("Shader container has no DXIL program");
    Graphics::Container::DxilView program(part->data);
    auto bitcode = program.GetBitcode();
    DxilSession session;
    dxil_spv_parsed_blob output = nullptr;
    auto result = dxil_spv_parse_dxil(bitcode.data(), bitcode.size(), &output);
    std::unique_ptr<dxil_spv_parsed_blob_s, Release> module(output);
    Check(result, "DXIL bitcode parsing");
    if (!module) throw std::runtime_error("DXIL parser returned no module");
    const char* assembly = nullptr;
    Check(dxil_spv_parsed_blob_get_disassembled_ir(module.get(), &assembly), "DXIL disassembly");
    if (!assembly || *assembly == '\0') throw std::runtime_error("DXIL disassembler returned no assembly");
    return assembly;
}
