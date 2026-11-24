#include "../../src/native/FirstLane.hpp"
#include "NativeFixtures.hpp"
#include <IntermediateRepresentation/IrBuilder.hpp>

namespace {
using namespace ShaderRecompiler;

struct Fixture {
    IrProgram program;
    IrValue* mask = nullptr;
    IrValue* read = nullptr;
    IrValue* payload = nullptr;

    Fixture(bool narrow, bool duplicate, bool changing, bool validEntry) {
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
        auto& condition = builder.INotEqual(builder.GetUserData(static_cast<ScalarReg>(0)), builder.Constant(0));
        auto& other = builder.INotEqual(builder.GetUserData(static_cast<ScalarReg>(1)), builder.Constant(0));
        builder.SetInsertionPoint(header);
        mask = &builder.Emit(IrOpcode::Phi, IrType::Bool, {});
        auto* copied = duplicate ? &builder.Emit(IrOpcode::Phi, IrType::Bool, {}) : mask;
        metadata[1].condition = mask;
        builder.SetInsertionPoint(body);
        auto& chosen = changing ? builder.LogicalNot(*mask) : condition;
        payload = &builder.Constant(32);
        auto& value = builder.Select(chosen, *payload, builder.Constant(3));
        read = &builder.Emit(IrOpcode::ReadFirstLane, IrType::U32, {&value, mask});
        auto& update = narrow ? builder.LogicalAnd(*copied, other) : builder.LogicalOr(*copied, other);
        auto* initial = validEntry ? &condition : &other;
        mask->AddPhiOperand(&entry, initial);
        mask->AddPhiOperand(&body, &update);
        if (duplicate) {
            copied->AddPhiOperand(&entry, initial);
            copied->AddPhiOperand(&body, &update);
        }
        ValidateProgram(program, true);
    }
};
}

void CheckFirstLaneGuards() {
    for (bool duplicate : {false, true}) {
        Fixture fixture(true, duplicate, false, true);
        Fixtures::Require(AnyBox::Shader::SimplifyFirstLaneGuards(fixture.program) == 1);
        Fixtures::Require(fixture.read->Argument(0) == fixture.payload && fixture.read->Argument(1) == fixture.mask);
        ValidateProgram(fixture.program, true);
    }
    Fixture widening(false, false, false, true);
    Fixtures::Require(AnyBox::Shader::SimplifyFirstLaneGuards(widening.program) == 0);
    Fixture changing(true, false, true, true);
    Fixtures::Require(AnyBox::Shader::SimplifyFirstLaneGuards(changing.program) == 0);
    Fixture unguarded(true, false, false, false);
    Fixtures::Require(AnyBox::Shader::SimplifyFirstLaneGuards(unguarded.program) == 0);
}
