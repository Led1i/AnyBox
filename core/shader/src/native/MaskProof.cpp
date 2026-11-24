#include "FirstLane.hpp"
#include "BitProof.hpp"
#include <algorithm>
#include <unordered_set>

namespace {
using namespace ShaderRecompiler;

bool Dominates(const IrProgram& program, const IrBlock* first, const IrBlock* second) {
    std::vector<const IrBlock*> pending{second};
    std::unordered_set<const IrBlock*> visited;
    for (std::size_t next = 0; next < pending.size(); ++next) {
        auto* block = pending[next];
        if (block == first || !visited.insert(block).second) continue;
        if (block == program.BlockOrder().front() || block->Predecessors().empty()) return false;
        pending.insert(pending.end(), block->Predecessors().begin(), block->Predecessors().end());
    }
    return true;
}

bool SamePhi(const IrValue* first, const IrValue* second) {
    if (first == second) return true;
    if (!first->IsPhi() || !second->IsPhi() || first->Parent() != second->Parent() || first->ArgumentCount() != second->ArgumentCount() || second->PhiBlockCount() != second->ArgumentCount()) return false;
    for (std::size_t index = 0; index < first->ArgumentCount(); ++index) {
        bool matched = false;
        for (std::size_t other = 0; other < second->ArgumentCount(); ++other) {
            matched |= first->PhiBlock(index) == second->PhiBlock(other) && first->Argument(index)->Resolve() == second->Argument(other)->Resolve();
        }
        if (!matched) return false;
    }
    return true;
}

bool Narrows(const IrValue* value, const IrValue* phi, unsigned depth, unsigned& steps) {
    if (steps == 0) return false;
    --steps;
    value = value->Resolve();
    if (SamePhi(value, phi)) return true;
    if (depth == 0 || value->Opcode() != IrOpcode::LogicalAnd || value->ArgumentCount() != 2) return false;
    return Narrows(value->Argument(0), phi, depth - 1, steps) || Narrows(value->Argument(1), phi, depth - 1, steps);
}

bool Implies(const IrProgram& program, const IrValue* mask, const IrValue* condition, unsigned depth, unsigned& steps) {
    if (steps == 0) return false;
    --steps;
    mask = mask->Resolve();
    condition = condition->Resolve();
    if (mask == condition || (mask->HasImmediate() && !mask->ImmediateBool()) || (condition->HasImmediate() && condition->ImmediateBool())) return true;
    if (depth == 0) return false;
    if (AnyBox::Shader::NativeBitPredicateImplies(mask, condition, steps)) return true;
    if (mask->Opcode() == IrOpcode::LogicalAnd && mask->ArgumentCount() == 2) return Implies(program, mask->Argument(0), condition, depth - 1, steps) || Implies(program, mask->Argument(1), condition, depth - 1, steps);
    if (mask->Opcode() == IrOpcode::LogicalOr && mask->ArgumentCount() == 2) return Implies(program, mask->Argument(0), condition, depth - 1, steps) && Implies(program, mask->Argument(1), condition, depth - 1, steps);
    if (!mask->IsPhi() || mask->ArgumentCount() == 0 || mask->ArgumentCount() != mask->PhiBlockCount()) return false;
    auto* header = mask->Parent();
    auto position = std::ranges::find(program.BlockOrder(), header);
    if (position == program.BlockOrder().end()) return false;
    const auto& metadata = program.Metadata().blockInfo.at(static_cast<std::size_t>(position - program.BlockOrder().begin()));
    if (!metadata.terminator.loopHeader || condition->Parent() == nullptr || condition->Parent() == header || !Dominates(program, condition->Parent(), header)) return false;
    bool entered = false;
    bool repeated = false;
    for (std::size_t index = 0; index < mask->ArgumentCount(); ++index) {
        auto* incoming = mask->Argument(index)->Resolve();
        if (Dominates(program, header, mask->PhiBlock(index))) {
            if (!Narrows(incoming, mask, depth - 1, steps)) return false;
            repeated = true;
        } else {
            if (!Implies(program, incoming, condition, depth - 1, steps)) return false;
            entered = true;
        }
    }
    return entered && repeated;
}
}

bool AnyBox::Shader::NativeMaskImplies(const ShaderRecompiler::IrProgram& program, const ShaderRecompiler::IrValue* mask, const ShaderRecompiler::IrValue* condition) {
    if (!mask || !condition || mask->Type() != ShaderRecompiler::IrType::Bool || condition->Type() != ShaderRecompiler::IrType::Bool) return false;
    unsigned steps = 4096;
    return Implies(program, mask, condition, 64, steps);
}
