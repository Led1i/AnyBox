#include "Scalar.hpp"
#include <cast.hpp>
#include <function.hpp>
#include <string_view>

using namespace ShaderRecompiler;
using AnyBox::Shader::Dxil::ScalarLowering;

IrValue& ScalarLowering::call(const LLVMBC::CallInst& instruction) {
    auto* function = instruction.getCalledFunction();
    if (!function) throw std::runtime_error("DXIL indirect function call is not implemented");
    const auto& name = function->getName();
    if (std::string_view(name.data(), name.size()) != "xbox.V_ALIGNBYTE_B32") throw std::runtime_error("DXIL intrinsic is not implemented: " + std::string(name.data(), name.size()));
    if (instruction.getNumArgOperands() != 4 || width(instruction) != 32) throw std::runtime_error("Xbox byte alignment intrinsic has an invalid signature");
    for (unsigned index = 0; index < 4; ++index) {
        if (width(*instruction.getArgOperand(index)) != 32) throw std::runtime_error("Xbox byte alignment intrinsic requires 32-bit operands");
    }
    auto* opcode = LLVMBC::dyn_cast<LLVMBC::ConstantInt>(instruction.getArgOperand(0));
    if (!opcode || opcode->getUniqueInteger().getZExtValue() != 332) throw std::runtime_error("Xbox byte alignment intrinsic has an invalid opcode");

    auto& high = Get(*instruction.getArgOperand(1));
    auto& low = Get(*instruction.getArgOperand(2));
    auto& offset = Get(*instruction.getArgOperand(3));
    auto& bytes = builder.BitwiseAnd(offset, builder.Constant(3));
    auto& bits = builder.ShiftLeftLogical(bytes, builder.Constant(3));
    auto& source = builder.ConstructU64(low, high);
    auto& shifted = builder.Emit(IrOpcode::ShiftRightLogical64, IrType::U64, {&source, &bits});
    return builder.CompositeExtract(shifted, 0);
}
