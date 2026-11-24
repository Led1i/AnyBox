#include "Slices.hpp"
#include <algorithm>
#include <stdexcept>

ShaderRecompiler::IrProgram AnyBox::Shader::TranslateNativeProgram(const ShaderRecompiler::RdnaProgram& decoded, const ShaderRecompiler::ControlFlowGraph& graph, const ShaderRecompiler::TranslateOptions& options) {
    using namespace ShaderRecompiler;
    if (std::ranges::none_of(decoded.instructions, IsNativeExtension)) return InstructionTranslator{}.Translate(decoded, graph, options);
    auto slices = SliceNativeExtensions(decoded, graph);
    auto skeleton = decoded;
    for (auto& instruction : skeleton.instructions) {
        if (!IsNativeExtension(instruction)) continue;
        instruction.op = RdnaOpcode::SNop;
        instruction.family = RdnaInstructionFamily::SOPP;
    }
    auto program = InstructionTranslator{}.Translate(skeleton, slices.graph, options);
    if (program.Metadata().vectorWindowSize != 0) throw std::runtime_error("Native instruction extensions with indexed vector registers are not implemented");
    auto rounding = ComputeEntryRounding(slices.graph, decoded, options.floatMode.has_value() ? options.floatMode->floatMode : ModeUndeclaredSeed);
    for (std::size_t index = 0; index < slices.graph.blocks.size(); ++index) {
        const auto& block = slices.graph.blocks[index];
        if (block.instructionEnd != block.instructionBegin + 1) continue;
        const auto& instruction = decoded.instructions.at(block.instructionBegin);
        if (!IsNativeExtension(instruction)) continue;
        EmitNativeLegacyFma(program, *program.BlockOrder().at(index + 1), instruction, options, rounding.at(block.id));
    }
    JoinNativeSlices(program, slices, graph);
    return program;
}
