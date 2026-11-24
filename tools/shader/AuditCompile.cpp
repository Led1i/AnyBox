#include "Audit.hpp"
#include "AuditTarget.hpp"
#include "DxilModule.hpp"
#include "ComputeEntry.hpp"
#include "ComputeCompiler.hpp"
#include "lowering/Function.hpp"
#include "arguments/RootBindings.hpp"
#include <anybox/shader/Requirements.hpp>
#include <spirv/unified1/spirv.hpp>
#include <algorithm>

std::size_t AnyBox::Shader::Tools::AuditCompute(std::span<const std::byte> code) {
    using namespace ShaderRecompiler;
    DxilModule module(code);
    auto entry = Dxil::ReadComputeEntry(module.GetModule());
    Dxil::ResourceLayout resources(module.GetModule());
    Graphics::Container::RootSignatureLayout root{};
    root.version = 2;
    if (!resources.GetBindings().empty()) root = Graphics::Container::ReadRootSignatureLayout(code);
    RootArgumentPlan arguments(resources.GetBindings(), root);
    Dxil::RootBindings bindings(arguments);
    IrProgram program;
    program.Resources().userDataCount = arguments.GetUserRegisterCount();
    ShaderComputeInputInfo input;
    std::copy(entry.info.threads.begin(), entry.info.threads.end(), input.threadsNum);
    auto wave = GetComputeWaveSize(code);
    if (wave && entry.info.preferredWaveSize && *wave != entry.info.preferredWaveSize) throw std::runtime_error("Xbox and DXIL wave size requirements disagree");
    input.waveSize = wave.value_or(entry.info.preferredWaveSize ? entry.info.preferredWaveSize : 32);
    input.hostSubgroupSize = 32;
    if (input.waveSize != input.hostSubgroupSize) throw std::runtime_error("Offline audit wave size adaptation is not implemented");
    program.SetWaveSize(input.waveSize);
    Dxil::FunctionLowering(program, resources, bindings).Run(*entry.function);
    auto target = AuditTarget();
    auto result = Dxil::CompileCompute(program, input, target, {0, 0, 0, 128});
    if (result.code.size() < 5 || result.code.front() != 0x07230203) throw std::runtime_error("Shader compiler returned invalid SPIR-V framing");
    if (!arguments.GetRanges().empty()) throw std::runtime_error("DXIL compute SPIR-V generated with reflected root arguments; guest command encoding and runtime root state are not verified");
    return result.code.size();
}
