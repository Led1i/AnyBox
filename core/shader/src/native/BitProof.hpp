#pragma once

#include <IntermediateRepresentation/IrValue.hpp>

namespace AnyBox::Shader {
bool NativeBitPredicateImplies(const ShaderRecompiler::IrValue* mask, const ShaderRecompiler::IrValue* condition, unsigned& steps);
}
