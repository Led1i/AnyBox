#include "Function.hpp"
#include <unordered_map>

using AnyBox::Shader::Dxil::FunctionLowering;

void FunctionLowering::connectPhis() {
    for (const auto& phi : phis) {
        std::unordered_map<const LLVMBC::BasicBlock*, IrValue*> incoming;
        for (unsigned index = 0; index < phi.source->getNumIncomingValues(); ++index) {
            auto* predecessor = phi.source->getIncomingBlock(index);
            auto* input = phi.source->getIncomingValue(index);
            if (input->getType() != phi.source->getType()) throw std::runtime_error("DXIL PHI incoming type does not match its result");
            auto& value = scalar.Get(*input);
            if (auto previous = incoming.find(predecessor); previous != incoming.end()) {
                auto* first = previous->second;
                bool sameConstant = first->HasImmediate() && value.HasImmediate() && (first->Type() == ShaderRecompiler::IrType::Bool ? first->ImmediateBool() == value.ImmediateBool() : first->ImmediateU32() == value.ImmediateU32());
                if (first != &value && !sameConstant) throw std::runtime_error("DXIL PHI repeats a predecessor with different incoming values");
                continue;
            }
            incoming.emplace(predecessor, &value);
            bool found = false;
            for (const auto& edge : edges) {
                if (edge.source != predecessor || edge.target != phi.block) continue;
                phi.value->AddPhiOperand(edge.block, &value);
                found = true;
            }
            if (!found) throw std::runtime_error("DXIL PHI incoming block is not a predecessor");
        }
        if (phi.value->PhiBlockCount() != blocks.at(phi.block)->Predecessors().size()) throw std::runtime_error("DXIL PHI does not cover every incoming edge");
    }
}
