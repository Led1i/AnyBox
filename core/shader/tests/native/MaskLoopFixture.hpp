#pragma once

#include <IntermediateRepresentation/IrBuilder.hpp>

namespace NativeMaskTests {
enum class Mode { Narrow, Widen, UnguardedWords, UnguardedBoolean, SwappedHalves, WrongPartition, Changing };

struct Fixture {
    ShaderRecompiler::IrProgram program;
    ShaderRecompiler::IrValue* mask = nullptr;
    ShaderRecompiler::IrValue* read = nullptr;
    ShaderRecompiler::IrValue* payload = nullptr;
    ShaderRecompiler::IrValue* value = nullptr;
    ShaderRecompiler::IrValue* chosen = nullptr;

    Fixture(std::uint32_t waveSize, Mode mode) {
        using namespace ShaderRecompiler;
        program.SetWaveSize(waveSize);
        auto& entry = program.CreateBlock();
        auto& header = program.CreateBlock();
        auto& body = program.CreateBlock();
        auto& exit = program.CreateBlock();
        program.SetEntryBlock(entry);
        program.BlockOrder() = {&entry, &header, &body, &exit};
        entry.AddBranch(&header);
        header.AddBranch(&body);
        header.AddBranch(&exit);
        body.AddBranch(&header);
        auto& metadata = program.Metadata().blockInfo;
        metadata.resize(4);
        for (unsigned index = 0; index < 4; ++index) metadata[index].id = index;
        metadata[0].terminator.kind = metadata[2].terminator.kind = TerminatorKind::Branch;
        metadata[0].terminator.trueBlock = metadata[2].terminator.trueBlock = 1;
        metadata[1].terminator.kind = TerminatorKind::ConditionalBranch;
        metadata[1].terminator.loopHeader = true;
        metadata[1].terminator.trueBlock = 2;
        metadata[1].terminator.falseBlock = metadata[1].terminator.mergeBlock = 3;
        metadata[1].terminator.continueBlock = 2;
        IrBuilder builder(program);
        builder.SetInsertionPoint(entry);
        auto& lane = builder.Emit(IrOpcode::LaneId, IrType::U32, {});
        auto& condition = builder.UGreaterThan(lane, builder.Constant(8));
        auto& initial = builder.Emit(IrOpcode::Ballot, IrType::Vec4U32, {mode == Mode::UnguardedWords ? &builder.ConstantBool(true) : &condition});
        auto& lowInitial = builder.CompositeExtract(initial, 0);
        auto& highInitial = builder.CompositeExtract(initial, 1);
        builder.SetInsertionPoint(header);
        auto& mask = builder.Emit(IrOpcode::Phi, IrType::Bool, {});
        auto& low = builder.Emit(IrOpcode::Phi, IrType::U32, {});
        auto& high = builder.Emit(IrOpcode::Phi, IrType::U32, {});
        metadata[1].condition = &mask;
        builder.SetInsertionPoint(body);
        auto& guard = mode == Mode::Changing ? builder.LogicalNot(mask) : condition;
        auto& payload = builder.ShiftLeftLogical(builder.IAdd(lane, builder.Constant(1)), builder.Constant(5));
        auto& value = builder.Select(guard, payload, builder.Constant(3));
        auto& read = builder.Emit(IrOpcode::ReadFirstLane, IrType::U32, {&value, &mask});
        auto& chosen = builder.LogicalAnd(mask, builder.IEqual(read, payload));
        auto& ballot = builder.Emit(IrOpcode::Ballot, IrType::Vec4U32, {&chosen});
        auto& lowRemoved = builder.BitwiseNot(builder.CompositeExtract(ballot, 0));
        auto& highRemoved = builder.BitwiseNot(builder.CompositeExtract(ballot, 1));
        auto& lowNext = mode == Mode::Widen ? builder.BitwiseOr(low, lowRemoved) : builder.BitwiseAnd(low, lowRemoved);
        auto& highNext = mode == Mode::Widen ? builder.BitwiseOr(high, highRemoved) : builder.BitwiseAnd(high, highRemoved);
        auto& lowHalf = builder.ULessThan(lane, builder.Constant(mode == Mode::WrongPartition ? 16 : 32));
        auto& selected = mode == Mode::SwappedHalves ? builder.Select(lowHalf, highNext, lowNext) : builder.Select(lowHalf, lowNext, highNext);
        auto& word = waveSize == 64 ? selected : lowNext;
        auto& selector = builder.BitwiseAnd(lane, builder.Constant(31));
        auto& next = builder.INotEqual(builder.BitwiseAnd(builder.ShiftRightLogical(word, selector), builder.Constant(1)), builder.Constant(0));
        mask.AddPhiOperand(&entry, mode == Mode::UnguardedBoolean ? &builder.ConstantBool(true) : &condition);
        mask.AddPhiOperand(&body, &next);
        low.AddPhiOperand(&entry, &lowInitial);
        low.AddPhiOperand(&body, &lowNext);
        high.AddPhiOperand(&entry, &highInitial);
        high.AddPhiOperand(&body, &highNext);
        this->mask = &mask;
        this->read = &read;
        this->payload = &payload;
        this->value = &value;
        this->chosen = &chosen;
        ValidateProgram(program, true);
    }
};
}
