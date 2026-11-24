#include "Bindings.hpp"
#include <RuntimeAbi.hpp>
#include <stdexcept>

using namespace ShaderRecompiler;
using namespace AnyBox::Shader;
using namespace AnyBox::Shader::Dxil;

namespace {
BufferAddress Read(ShaderRecompiler::IrBuilder& builder, std::uint32_t first) {
    auto word = [&](std::uint32_t index) -> IrValue& { return builder.GetUserData(static_cast<ScalarReg>(first + index)); };
    return {builder.ConstructU64(word(0), word(1)), builder.ConstructU64(word(2), word(3))};
}
}

ArgumentBindings::ArgumentBindings(const BufferArgumentPlan& plan) : plan(plan) {
    static_assert(BufferArgumentPlan::UserRegisterCapacity == RuntimeAbi::UserDataCapacity);
}

BufferAddress ArgumentBindings::ResolveBuffer(const BufferBinding& binding, IrValue& index, bool nonUniform, IrBuilder& builder) {
    if (index.Type() != IrType::U32) throw std::invalid_argument("Buffer argument selection must be a 32-bit unsigned index");
    static_cast<void>(nonUniform);
    const auto& range = plan.Find(binding);
    if (index.HasImmediate()) {
        auto selected = index.ImmediateU32();
        if (selected < binding.firstRegister || selected - binding.firstRegister >= binding.count) throw std::invalid_argument("Buffer argument index is outside its declared range");
        return Read(builder, range.firstUserRegister + (selected - binding.firstRegister) * BufferArgumentPlan::WordsPerBuffer);
    }

    auto& relative = builder.ISub(index, builder.Constant(binding.firstRegister));
    auto* address = &builder.ConstantU64(0);
    auto* length = &builder.ConstantU64(0);
    for (std::uint32_t element = 0; element < binding.count; ++element) {
        auto value = Read(builder, range.firstUserRegister + element * BufferArgumentPlan::WordsPerBuffer);
        auto& selected = builder.IEqual(relative, builder.Constant(element));
        address = &builder.Select(selected, value.address, *address);
        length = &builder.Select(selected, value.byteLength, *length);
    }
    return {*address, *length};
}
