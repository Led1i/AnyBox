#include "Scalar.hpp"
#include <cast.hpp>
#include <stdexcept>

using namespace ShaderRecompiler;
using AnyBox::Shader::Dxil::ScalarLowering;

ScalarLowering::ScalarLowering(IrBuilder& builder) : builder(builder) {}

unsigned ScalarLowering::width(const LLVMBC::Value& value) {
    auto* type = value.getType();
    if (!type || !type->isIntegerTy()) throw std::runtime_error("DXIL scalar lowering requires an integer value");
    auto bits = type->getIntegerBitWidth();
    if (bits != 1 && bits != 8 && bits != 16 && bits != 32 && bits != 64) throw std::runtime_error("DXIL integer width is not implemented: " + std::to_string(bits));
    return bits;
}

IrValue& ScalarLowering::unsignedValue(IrValue& value) {
    if (value.Type() == IrType::Bool) return builder.Select(value, builder.Constant(1), builder.Constant(0));
    if (value.Type() != IrType::U32) throw std::logic_error("DXIL integer value has an incompatible IR type");
    return value;
}

IrValue& ScalarLowering::narrow(IrValue& value, unsigned bits) {
    if (bits == 64) {
        if (value.Type() != IrType::U64) throw std::logic_error("DXIL wide integer value has an incompatible IR type");
        return value;
    }
    auto& integer = value.Type() == IrType::U64 ? builder.CompositeExtract(value, 0) : unsignedValue(value);
    if (bits == 32) return integer;
    auto& masked = builder.BitwiseAnd(integer, builder.Constant((1u << bits) - 1));
    if (bits == 1) return builder.INotEqual(masked, builder.Constant(0));
    return masked;
}

IrValue& ScalarLowering::signedValue(IrValue& value, unsigned bits) {
    auto& integer = unsignedValue(value);
    if (bits == 32) return integer;
    auto& shift = builder.Constant(32 - bits);
    return builder.ShiftRightArithmetic(builder.ShiftLeftLogical(integer, shift), shift);
}

void ScalarLowering::Bind(const LLVMBC::Value& source, IrValue& value) {
    if (values.contains(&source)) throw std::logic_error("DXIL value is already bound");
    auto bits = width(source);
    if (value.IsPhi()) {
        if (value.Type() != (bits == 1 ? IrType::Bool : bits == 64 ? IrType::U64 : IrType::U32)) throw std::logic_error("DXIL PHI binding has an incompatible IR type");
        values.emplace(&source, &value);
        return;
    }
    auto& converted = narrow(value, bits);
    values.emplace(&source, &converted);
}

IrValue& ScalarLowering::Get(const LLVMBC::Value& value) {
    if (auto found = values.find(&value); found != values.end()) return *found->second;
    auto bits = width(value);
    IrValue* output = nullptr;
    if (auto* constant = LLVMBC::dyn_cast<LLVMBC::ConstantInt>(&value)) {
        auto data = constant->getUniqueInteger().getZExtValue();
        if (bits == 64) output = &builder.ConstantU64(data);
        else if (bits == 1) output = &builder.ConstantBool((data & 1) != 0);
        else output = &builder.Constant(static_cast<std::uint32_t>(bits == 32 ? data : data & ((1u << bits) - 1)));
    } else if (auto* instruction = LLVMBC::dyn_cast<LLVMBC::BinaryOperator>(&value)) {
        output = &binary(*instruction);
    } else if (auto* instruction = LLVMBC::dyn_cast<LLVMBC::ICmpInst>(&value)) {
        output = &compare(*instruction);
    } else if (auto* instruction = LLVMBC::dyn_cast<LLVMBC::CastInst>(&value)) {
        output = &cast(*instruction);
    } else if (auto* instruction = LLVMBC::dyn_cast<LLVMBC::CallInst>(&value)) {
        output = &call(*instruction);
    } else if (auto* instruction = LLVMBC::dyn_cast<LLVMBC::SelectInst>(&value)) {
        if (width(*instruction->getOperand(0)) != 1 || width(*instruction->getOperand(1)) != bits || width(*instruction->getOperand(2)) != bits) throw std::runtime_error("DXIL select operand types do not match");
        auto& condition = Get(*instruction->getOperand(0));
        if (condition.Type() != IrType::Bool) throw std::runtime_error("DXIL select requires a scalar boolean condition");
        output = &builder.Select(condition, Get(*instruction->getOperand(1)), Get(*instruction->getOperand(2)));
    } else {
        throw std::runtime_error("DXIL scalar instruction or unbound value is not implemented");
    }
    values.emplace(&value, output);
    return *output;
}
