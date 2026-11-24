#include "BitProof.hpp"
#include <optional>

namespace {
using namespace ShaderRecompiler;

bool Constant(const IrValue* value, std::uint32_t expected) {
    value = value->Resolve();
    return value->Type() == IrType::U32 && value->HasImmediate() && value->ImmediateU32() == expected;
}

struct Bit {
    const IrValue* word;
    const IrValue* selector;
};

std::optional<Bit> Predicate(const IrValue* value) {
    value = value->Resolve();
    if (value->Opcode() != IrOpcode::INotEqual32 || value->ArgumentCount() != 2) return std::nullopt;
    auto* bit = Constant(value->Argument(1), 0) ? value->Argument(0)->Resolve() : Constant(value->Argument(0), 0) ? value->Argument(1)->Resolve() : nullptr;
    if (!bit || bit->Opcode() != IrOpcode::BitwiseAnd32 || bit->ArgumentCount() != 2) return std::nullopt;
    auto* shifted = Constant(bit->Argument(1), 1) ? bit->Argument(0)->Resolve() : Constant(bit->Argument(0), 1) ? bit->Argument(1)->Resolve() : nullptr;
    if (!shifted || shifted->Opcode() != IrOpcode::ShiftRightLogical32 || shifted->ArgumentCount() != 2) return std::nullopt;
    return Bit{shifted->Argument(0)->Resolve(), shifted->Argument(1)->Resolve()};
}

std::optional<std::uint32_t> LaneMask(const IrValue* value) {
    value = value->Resolve();
    if (value->Opcode() == IrOpcode::LaneId) return UINT32_MAX;
    if (value->Opcode() != IrOpcode::BitwiseAnd32 || value->ArgumentCount() != 2) return std::nullopt;
    for (unsigned index = 0; index < 2; ++index) {
        auto* lane = value->Argument(index)->Resolve();
        auto* mask = value->Argument(1 - index)->Resolve();
        if (lane->Opcode() == IrOpcode::LaneId && mask->Type() == IrType::U32 && mask->HasImmediate()) return mask->ImmediateU32();
    }
    return std::nullopt;
}

bool Subset(const IrValue* value, const IrValue* limit, unsigned depth, unsigned& steps) {
    if (steps == 0) return false;
    --steps;
    value = value->Resolve();
    limit = limit->Resolve();
    if (value == limit || Constant(value, 0) || Constant(limit, UINT32_MAX)) return true;
    if (value->Type() == IrType::U32 && limit->Type() == IrType::U32 && value->HasImmediate() && limit->HasImmediate()) return (value->ImmediateU32() & ~limit->ImmediateU32()) == 0;
    if (depth == 0 || value->ArgumentCount() != 2) return false;
    if (value->Opcode() == IrOpcode::BitwiseAnd32) return Subset(value->Argument(0), limit, depth - 1, steps) || Subset(value->Argument(1), limit, depth - 1, steps);
    if (value->Opcode() == IrOpcode::BitwiseOr32) return Subset(value->Argument(0), limit, depth - 1, steps) && Subset(value->Argument(1), limit, depth - 1, steps);
    return false;
}

bool BallotImplies(const IrValue* word, const IrValue* condition, unsigned depth, unsigned& steps) {
    if (steps == 0 || depth == 0) return false;
    --steps;
    word = word->Resolve();
    if (word->Opcode() == IrOpcode::BitwiseAnd32 && word->ArgumentCount() == 2) return BallotImplies(word->Argument(0), condition, depth - 1, steps) || BallotImplies(word->Argument(1), condition, depth - 1, steps);
    if (word->Opcode() == IrOpcode::BitwiseOr32 && word->ArgumentCount() == 2) return BallotImplies(word->Argument(0), condition, depth - 1, steps) && BallotImplies(word->Argument(1), condition, depth - 1, steps);
    if (word->Opcode() != IrOpcode::CompositeExtractU32x4 || word->ArgumentCount() != 2 || !Constant(word->Argument(1), 0)) return false;
    auto* ballot = word->Argument(0)->Resolve();
    return ballot->Opcode() == IrOpcode::Ballot && ballot->ArgumentCount() == 1 && ballot->Argument(0)->Resolve() == condition->Resolve();
}
}

bool AnyBox::Shader::NativeBitPredicateImplies(const ShaderRecompiler::IrValue* mask, const ShaderRecompiler::IrValue* condition, unsigned& steps) {
    auto source = Predicate(mask);
    auto target = Predicate(condition);
    if (!source || !target) return false;
    if (source->selector != target->selector) {
        auto sourceMask = LaneMask(source->selector);
        auto targetMask = LaneMask(target->selector);
        if (!sourceMask || !targetMask || *sourceMask != *targetMask) return false;
    }
    return Subset(source->word, target->word, 64, steps);
}

bool AnyBox::Shader::NativeBallotPredicateImplies(const ShaderRecompiler::IrValue* mask, const ShaderRecompiler::IrValue* condition, std::uint32_t waveSize, unsigned& steps) {
    if (waveSize != 32) return false;
    auto source = Predicate(mask);
    if (!source) return false;
    auto selector = LaneMask(source->selector);
    if (!selector || (*selector != 31 && *selector != UINT32_MAX)) return false;
    return BallotImplies(source->word, condition, 64, steps);
}
