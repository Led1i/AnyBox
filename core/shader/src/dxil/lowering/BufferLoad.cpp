#include "Buffer.hpp"
#include "Scalar.hpp"
#include <cast.hpp>
#include <bit>

using namespace ShaderRecompiler;
using namespace AnyBox::Shader;
using namespace AnyBox::Shader::Dxil;

void BufferLowering::load(const LLVMBC::CallInst& instruction, unsigned bits, ScalarLowering& scalar) {
    if (instruction.getNumArgOperands() != 6) throw std::runtime_error("DXIL raw buffer load has an invalid argument count");
    auto constant = [&](unsigned index, unsigned width) {
        auto* value = LLVMBC::dyn_cast<LLVMBC::ConstantInt>(instruction.getArgOperand(index));
        if (!value || !value->getType()->isIntegerTy() || value->getType()->getIntegerBitWidth() != width) throw std::runtime_error("DXIL raw buffer load constant has an invalid integer type");
        return static_cast<std::uint32_t>(value->getUniqueInteger().getZExtValue());
    };
    if (constant(0, 32) != 139) throw std::runtime_error("DXIL raw buffer load opcode is invalid");
    auto mask = constant(4, 8);
    auto alignment = constant(5, 32);
    if (mask == 0 || mask > 15 || !std::has_single_bit(alignment)) throw std::runtime_error("DXIL raw buffer load mask or alignment is invalid");
    auto* result = instruction.getType();
    if (!result || result->getTypeID() != LLVMBC::Type::TypeID::StructTyID || result->getStructNumElements() != 5) throw std::runtime_error("DXIL raw buffer load result must contain five members");
    for (unsigned index = 0; index < 5; ++index) {
        auto* member = result->getStructElementType(index);
        if (!member->isIntegerTy() || member->getIntegerBitWidth() != (index == 4 ? 32 : bits)) throw std::runtime_error("DXIL raw buffer load result member type is invalid");
    }
    auto* input = instruction.getArgOperand(1);
    auto handle = handles.find(input);
    if (handle == handles.end()) throw std::runtime_error("DXIL raw buffer load requires a previously created resource handle");
    const auto& binding = *handle->second.binding;
    if (binding.resourceClass != ResourceClass::ShaderResource && binding.resourceClass != ResourceClass::UnorderedAccess) throw std::runtime_error("DXIL raw buffer load requires an SRV or UAV");
    if (binding.kind != BufferKind::Raw && binding.kind != BufferKind::Structured) throw std::runtime_error("DXIL raw buffer load requires a raw or structured buffer");
    auto* index = instruction.getArgOperand(2);
    auto* offset = instruction.getArgOperand(3);
    if (!index->getType()->isIntegerTy() || index->getType()->getIntegerBitWidth() != 32 || !offset->getType()->isIntegerTy() || offset->getType()->getIntegerBitWidth() != 32) throw std::runtime_error("DXIL buffer indices must be 32-bit integers");
    auto& selected = scalar.Get(*handle->second.index);
    auto view = bindings.ResolveBuffer(binding, selected, handle->second.nonUniform, builder);
    if (view.address.Type() != IrType::U64 || view.byteLength.Type() != IrType::U64) throw std::logic_error("DXIL buffer binding requires a 64-bit address and size");
    auto& element = builder.ConstructU64(scalar.Get(*index), builder.Constant(0));
    IrValue* baseOffset = &element;
    IrValue* structureOffset = nullptr;
    if (binding.kind == BufferKind::Structured) {
        structureOffset = &builder.ConstructU64(scalar.Get(*offset), builder.Constant(0));
        auto& stride = builder.ConstantU64(binding.stride);
        auto& start = builder.Emit(IrOpcode::IMul64, IrType::U64, {&element, &stride});
        baseOffset = &builder.Emit(IrOpcode::IAdd64, IrType::U64, {&start, structureOffset});
    } else if (!LLVMBC::isa<LLVMBC::UndefValue>(offset)) {
        throw std::runtime_error("DXIL raw byte address buffer load must not specify a structure offset");
    }
    std::array<IrValue*, 5> output{};
    for (unsigned component = 0; component < 4; ++component) {
        if ((mask & (1u << component)) == 0) continue;
        auto& displacement = builder.ConstantU64(component * (bits / 8));
        auto& location = builder.Emit(IrOpcode::IAdd64, IrType::U64, {baseOffset, &displacement});
        auto& size = builder.ConstantU64(bits / 8);
        auto& end = builder.Emit(IrOpcode::IAdd64, IrType::U64, {&location, &size});
        auto& outside = builder.Emit(IrOpcode::UGreaterThan64, IrType::Bool, {&end, &view.byteLength});
        IrValue* active = &builder.LogicalNot(outside);
        if (structureOffset) {
            auto& componentEnd = builder.Emit(IrOpcode::IAdd64, IrType::U64, {structureOffset, &builder.ConstantU64((component + 1) * (bits / 8))});
            auto& exceedsElement = builder.Emit(IrOpcode::UGreaterThan64, IrType::Bool, {&componentEnd, &builder.ConstantU64(binding.stride)});
            active = &builder.LogicalAnd(*active, builder.LogicalNot(exceedsElement));
        }
        auto& address = builder.Emit(IrOpcode::IAdd64, IrType::U64, {&view.address, &location});
        output[component] = &read(address, *active, bits, binding.globallyCoherent);
    }
    aggregates.emplace(&instruction, output);
}
