#include "Target.hpp"
#include "NativeFixtures.hpp"
#include <anybox/shader/NativeCompute.hpp>
#include <Recompiler.hpp>
#include <array>

void BufferTests::CheckNativeGpu(AnyBox::Vulkan::Context& context) {
    auto data = Fixtures::MakeComputeProgram();
    Fixtures::Set(data, 0x13C, 32);
    Fixtures::Set(data, 0x140, 1);
    Fixtures::Set(data, 0x144, 1);
    constexpr std::array<std::uint32_t, 5> code{0x7E040282, 0x34020082, 0xDC708000, 0x007F0001, 0xBF810000};
    for (std::size_t index = 0; index < code.size(); ++index) Fixtures::Set(data, 0x168 + index * 4, code[index]);
    AnyBox::Shader::NativeFrontend frontend(data);
    auto state = AnyBox::Graphics::Container::ReadNativeComputeState(data);
    auto target = Target();
    auto translated = AnyBox::Shader::LowerNativeCompute(frontend, state, ShaderRecompiler::ShaderFloatMode{0xC0, true, true, false}, target);
    auto compiled = AnyBox::Shader::CompileCompute(translated.program, translated.input, target, {0, 0, 0, 128});
    std::array<std::uint32_t, 32> expected{};
    for (unsigned index = 0; index < expected.size(); ++index) expected[index] = index;
    RunGpu(context, compiled, expected, {1, 1, 1});
}
