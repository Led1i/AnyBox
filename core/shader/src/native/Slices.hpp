#pragma once

#include <Translation/InstructionTranslator.hpp>
#include <Translation/RoundingMode.hpp>
#include <vector>

namespace AnyBox::Shader {
struct NativeSlices {
    ShaderRecompiler::ControlFlowGraph graph;
    std::vector<std::vector<std::uint32_t>> groups;
};

bool IsNativeExtension(const ShaderRecompiler::RdnaInstruction& instruction);
NativeSlices SliceNativeExtensions(const ShaderRecompiler::RdnaProgram& program, const ShaderRecompiler::ControlFlowGraph& graph);
void JoinNativeSlices(ShaderRecompiler::IrProgram& program, const NativeSlices& slices, const ShaderRecompiler::ControlFlowGraph& original);
ShaderRecompiler::IrProgram TranslateNativeProgram(const ShaderRecompiler::RdnaProgram& program, const ShaderRecompiler::ControlFlowGraph& graph, const ShaderRecompiler::TranslateOptions& options);
void EmitNativeLegacyFma(ShaderRecompiler::IrProgram& program, ShaderRecompiler::IrBlock& block, const ShaderRecompiler::RdnaInstruction& instruction, const ShaderRecompiler::TranslateOptions& options, const ShaderRecompiler::RoundingState& rounding);
}
