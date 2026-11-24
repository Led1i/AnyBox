#include "Buffer.hpp"
#include <limits>

using namespace ShaderRecompiler;
using AnyBox::Shader::Dxil::BufferLowering;

IrValue& BufferLowering::read(IrValue& address, IrValue& active, unsigned bits, bool coherent) {
    auto& low = builder.CompositeExtract(address, 0);
    auto& high = builder.CompositeExtract(address, 1);
    auto& resource = builder.Emit(IrOpcode::GetAddressResource, IrType::AddressResource, {&low, &high});
    MemoryInfo memory;
    memory.kind = ResourceKind::Global;
    memory.addressIsFull = true;
    memory.dataBits = bits == 64 ? 32 : bits;
    memory.dataDwords = bits == 64 ? 2 : 1;
    memory.componentCount = memory.dataDwords;
    memory.coherent = coherent;
    auto& metadata = program.Resources().memoryInfo;
    if (metadata.size() >= std::numeric_limits<std::uint32_t>::max()) throw std::length_error("DXIL memory operation count exceeds the IR limit");
    MemoryFlags flags{static_cast<std::uint32_t>(metadata.size()), 0};
    metadata.push_back(memory);
    auto opcode = bits == 16 ? IrOpcode::LoadAddressU16 : bits == 64 ? IrOpcode::LoadAddressU32x2 : IrOpcode::LoadAddressU32;
    auto& loaded = builder.Emit(opcode, IrOpcodeType(opcode), {&resource, &low, &high, &active}, flags);
    if (bits == 16) return builder.Emit(IrOpcode::ConvertU32U16, IrType::U32, {&loaded});
    if (bits == 64) return builder.ConstructU64(builder.CompositeExtract(loaded, 0), builder.CompositeExtract(loaded, 1));
    return loaded;
}
