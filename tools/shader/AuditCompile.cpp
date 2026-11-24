#include "Audit.hpp"
#include "DxilModule.hpp"
#include "ComputeEntry.hpp"
#include "ComputeCompiler.hpp"
#include "lowering/Function.hpp"
#include <anybox/shader/Requirements.hpp>
#include <spirv/unified1/spirv.hpp>
#include <algorithm>

namespace {
class AuditBindings : public AnyBox::Shader::Dxil::IResourceBindings {
public:
    AnyBox::Shader::Dxil::BufferAddress ResolveBuffer(const AnyBox::Shader::BufferBinding&, ShaderRecompiler::IrValue&, bool, ShaderRecompiler::IrBuilder&) override {
        throw std::runtime_error("Root resource binding plan is not implemented");
    }
};
}

std::size_t AnyBox::Shader::Tools::AuditCompute(std::span<const std::byte> code) {
    using namespace ShaderRecompiler;
    DxilModule module(code);
    auto entry = Dxil::ReadComputeEntry(module.GetModule());
    Dxil::ResourceLayout resources(module.GetModule());
    AuditBindings bindings;
    IrProgram program;
    Dxil::FunctionLowering(program, resources, bindings).Run(*entry.function);
    ShaderComputeInputInfo input;
    std::copy(entry.info.threads.begin(), entry.info.threads.end(), input.threadsNum);
    auto wave = GetComputeWaveSize(code);
    if (wave && entry.info.preferredWaveSize && *wave != entry.info.preferredWaveSize) throw std::runtime_error("Xbox and DXIL wave size requirements disagree");
    input.waveSize = wave.value_or(entry.info.preferredWaveSize ? entry.info.preferredWaveSize : 32);
    input.hostSubgroupSize = 32;
    if (input.waveSize != input.hostSubgroupSize) throw std::runtime_error("Offline audit wave size adaptation is not implemented");
    static constexpr std::array<std::uint32_t, 5> capabilities{spv::CapabilityShader, spv::CapabilityInt64, spv::CapabilityGroupNonUniform, spv::CapabilityPhysicalStorageBufferAddresses, spv::CapabilityStorageBuffer8BitAccess};
    static constexpr std::array<std::string_view, 3> extensions{"SPV_KHR_storage_buffer_storage_class", "SPV_KHR_physical_storage_buffer", "SPV_KHR_8bit_storage"};
    SpirvTargetOptions target{};
    target.vulkanVersion = 0x00402000;
    target.spirvVersion = 0x00010500;
    target.subgroupSize = 32;
    target.bdaAbiVersion = 1;
    target.supportedCapabilities = capabilities;
    target.supportedExtensions = extensions;
    auto result = Dxil::CompileCompute(program, input, target, {0, 0, 0, 128});
    if (result.code.size() < 5 || result.code.front() != 0x07230203) throw std::runtime_error("Shader compiler returned invalid SPIR-V framing");
    return result.code.size();
}
