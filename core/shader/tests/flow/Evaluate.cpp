#include "Fixture.hpp"
#include <algorithm>

std::vector<std::uint32_t> FlowTests::Evaluate(const ShaderRecompiler::IrProgram& program) {
    using namespace ShaderRecompiler;
    std::unordered_map<const IrValue*, std::uint32_t> values;
    auto read = [&](const IrValue* value) {
        value = value->Resolve();
        if (value->HasImmediate()) return value->Type() == IrType::Bool ? static_cast<std::uint32_t>(value->ImmediateBool()) : value->ImmediateU32();
        return values.at(value);
    };
    auto* current = &program.EntryBlock();
    const IrBlock* predecessor = nullptr;
    for (unsigned step = 0; step < 1024; ++step) {
        std::vector<std::pair<const IrValue*, std::uint32_t>> incoming;
        for (auto* instruction : current->Instructions()) {
            if (!instruction->IsPhi()) continue;
            bool found = false;
            for (std::size_t index = 0; index < instruction->PhiBlockCount(); ++index) {
                if (instruction->PhiBlock(index) != predecessor) continue;
                incoming.emplace_back(instruction, read(instruction->Argument(index)));
                found = true;
            }
            if (!found) throw std::runtime_error("Test execution found no incoming PHI value");
        }
        for (auto [value, data] : incoming) values[value] = data;
        for (auto* instruction : current->Instructions()) {
            auto argument = [&](unsigned index) { return read(instruction->Argument(index)); };
            switch (instruction->Opcode()) {
                case IrOpcode::Phi: break;
                case IrOpcode::IAdd32: values[instruction] = argument(0) + argument(1); break;
                case IrOpcode::BitwiseAnd32: values[instruction] = argument(0) & argument(1); break;
                case IrOpcode::IEqual32: values[instruction] = argument(0) == argument(1); break;
                case IrOpcode::INotEqual32: values[instruction] = argument(0) != argument(1); break;
                case IrOpcode::ULessThan32: values[instruction] = argument(0) < argument(1); break;
                case IrOpcode::LogicalNot: values[instruction] = !argument(0); break;
                case IrOpcode::LogicalXor: values[instruction] = argument(0) != argument(1); break;
                case IrOpcode::Branch:
                case IrOpcode::BranchConditional:
                case IrOpcode::Return: break;
                default: throw std::runtime_error("Test execution encountered an unexpected IR opcode");
            }
        }
        const auto& info = program.Metadata().blockInfo.at(current->Id());
        if (info.terminator.kind == TerminatorKind::Return) {
            std::vector<std::uint32_t> output;
            for (const auto& block : program.Blocks()) {
                for (auto* instruction : block->Instructions()) {
                    if (instruction->IsPhi() && values.contains(instruction)) output.push_back(values.at(instruction));
                }
            }
            return output;
        }
        auto target = info.terminator.kind == TerminatorKind::Branch || read(info.condition) ? info.terminator.trueBlock : info.terminator.falseBlock;
        predecessor = current;
        current = program.Blocks().at(target).get();
    }
    throw std::runtime_error("Test execution exceeded its step limit");
}
