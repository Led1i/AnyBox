#pragma once

#include "../ResourceLayout.hpp"
#include <IntermediateRepresentation/IrBuilder.hpp>
#include <array>
#include <unordered_map>

namespace AnyBox::Shader::Dxil {
class ScalarLowering;

struct BufferAddress {
    ShaderRecompiler::IrValue& address;
    ShaderRecompiler::IrValue& byteLength;
};

class IResourceBindings {
public:
    virtual ~IResourceBindings() = default;
    virtual BufferAddress ResolveBuffer(const BufferBinding& binding, ShaderRecompiler::IrValue& index, bool nonUniform, ShaderRecompiler::IrBuilder& builder) = 0;
};

class BufferLowering {
public:
    BufferLowering(ShaderRecompiler::IrProgram& program, ShaderRecompiler::IrBuilder& builder, const ResourceLayout& layout, IResourceBindings& bindings);
    bool Emit(const LLVMBC::CallInst& instruction, ScalarLowering& scalar);
    ShaderRecompiler::IrValue* Extract(const LLVMBC::ExtractValueInst& instruction, ScalarLowering& scalar);

private:
    using IrValue = ShaderRecompiler::IrValue;
    void load(const LLVMBC::CallInst& instruction, unsigned bits, ScalarLowering& scalar);
    IrValue& read(IrValue& address, IrValue& active, unsigned bits, bool coherent);
    ShaderRecompiler::IrProgram& program;
    ShaderRecompiler::IrBuilder& builder;
    const ResourceLayout& layout;
    IResourceBindings& bindings;
    std::unordered_map<const LLVMBC::Value*, ResourceHandle> handles;
    std::unordered_map<const LLVMBC::Value*, std::array<IrValue*, 5>> aggregates;
};
}
