#include <anybox/shader/NativeCompute.hpp>
#include <anybox/shader/ComputeCompiler.hpp>
#include "../buffer/Target.hpp"
#include "NativeFixtures.hpp"
#include <Recompiler.hpp>

void CheckNativeImages() {
    using namespace Fixtures;
    using namespace AnyBox::Shader;
    for (unsigned dimension : {0u, 1u, 2u, 3u, 4u}) {
        auto data = MakeComputeProgram();
        Set(data, 0x138, (2 << 15) | (2 << 11) | 0x380 | 16);
        Set(data, 0x168, 0xF0200100 | (dimension << 3));
        Set(data, 0x16C, 0);
        Set(data, 0x170, 0xBF810000);
        NativeFrontend frontend(data);
        auto state = AnyBox::Graphics::Container::ReadNativeComputeState(data);
        auto target = BufferTests::Target();
        auto translated = LowerNativeCompute(frontend, state, ShaderRecompiler::ShaderFloatMode{0xC0, true, true, false}, target);
        auto compiled = CompileCompute(translated.program, translated.input, target, {0, 0, 0, 128});
        Require(translated.program.Info().images.size() == 1);
        Require(translated.program.Info().runtimeImageModes.size() == 1);
        Require(!translated.program.Info().runtimeImageModes.front().empty());

        bool imageWrite = false;
        for (std::size_t offset = 5; offset < compiled.code.size();) {
            auto size = compiled.code[offset] >> 16;
            Require(size != 0 && size <= compiled.code.size() - offset);
            imageWrite |= (compiled.code[offset] & 65535) == static_cast<unsigned>(spv::OpImageWrite);
            offset += size;
        }
        Require(imageWrite);
    }
}
