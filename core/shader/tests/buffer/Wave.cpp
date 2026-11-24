#include "FloatFixture.hpp"
#include "Fixtures.hpp"
#include <Recompiler.hpp>
#include <bit>

namespace {
ShaderRecompiler::IrProgram Make(std::uint64_t mask, unsigned waveSize) {
    using namespace ShaderRecompiler;
    ResourceTests::Fixture source;
    auto& context = source.Context();
    auto* word = LLVMBC::Type::getInt32Ty(context);
    auto* boolean = LLVMBC::Type::getInt1Ty(context);
    source.Module().add_value_name(110, "dx.op.waveIsFirstLane");
    auto* signature = context.construct<LLVMBC::FunctionType>(context, boolean, LLVMBC::Vector<LLVMBC::Type*>{word});
    auto* function = context.construct<LLVMBC::Function>(signature, 110, source.Module());
    auto* call = context.construct<LLVMBC::CallInst>(signature, function, LLVMBC::Vector<LLVMBC::Value*>{LLVMBC::ConstantInt::get(word, 110)});
    IrProgram program;
    program.SetWaveSize(waveSize);
    auto& entry = program.CreateBlock();
    auto& body = program.CreateBlock();
    auto& exit = program.CreateBlock();
    program.SetEntryBlock(entry);
    program.BlockOrder() = {&entry, &body, &exit};
    entry.AddBranch(&body);
    entry.AddBranch(&exit);
    body.AddBranch(&exit);
    auto& metadata = program.Metadata().blockInfo;
    metadata.resize(3);
    for (unsigned index = 0; index < 3; ++index) metadata[index].id = index;
    metadata[0].terminator.kind = TerminatorKind::ConditionalBranch;
    metadata[0].terminator.condition = BranchCondition::ScalarInstruction;
    metadata[0].terminator.trueBlock = 1;
    metadata[0].terminator.falseBlock = metadata[0].terminator.mergeBlock = 2;
    metadata[1].terminator.kind = TerminatorKind::Branch;
    metadata[1].terminator.trueBlock = 2;
    IrBuilder builder(program);
    builder.SetInsertionPoint(entry);
    auto& lane = builder.Emit(IrOpcode::LaneId, IrType::U32, {});
    auto& low = builder.Constant(static_cast<std::uint32_t>(mask));
    auto& high = builder.Constant(static_cast<std::uint32_t>(mask >> 32));
    auto& selected = builder.Select(builder.ULessThan(lane, builder.Constant(32)), low, high);
    auto& shift = builder.BitwiseAnd(lane, builder.Constant(31));
    auto& active = builder.INotEqual(builder.BitwiseAnd(builder.ShiftRightLogical(selected, shift), builder.Constant(1)), builder.Constant(0));
    metadata[0].condition = &active;
    (void)builder.Emit(IrOpcode::Reference, IrType::Void, {&active});
    auto& global = builder.Emit(IrOpcode::GetBuiltin, IrType::U32, {&builder.Constant(static_cast<unsigned>(StageInputKind::GlobalInvocationId)), &builder.Constant(0)});
    builder.SetInsertionPoint(body);
    AnyBox::Shader::Dxil::ScalarLowering scalar(program, builder);
    auto& elected = scalar.Get(*call);
    auto& result = builder.Select(elected, builder.Constant(1), builder.Constant(0));
    auto& offset = builder.ShiftLeftLogical(global, builder.Constant(2));
    auto& address = builder.Emit(IrOpcode::GetAddressResource, IrType::AddressResource, {&offset, &builder.Constant(2)});
    MemoryInfo memory;
    memory.kind = ResourceKind::Global;
    memory.addressIsFull = true;
    program.Resources().memoryInfo.push_back(memory);
    (void)builder.Emit(IrOpcode::StoreAddressU32, IrType::Void, {&address, &offset, &builder.Constant(2), &result, &active}, MemoryFlags{0, 0});
    ValidateProgram(program, true);
    return program;
}

void CheckInvalid(AnyBox::Vulkan::Context& context) {
    BufferTests::FloatFixture fixture;
    auto* boolean = fixture.IntegerType(1);
    auto reject = [&](LLVMBC::Type* result, LLVMBC::Vector<LLVMBC::Value*> arguments) {
        fixture.Reject(*fixture.Call("dx.op.waveIsFirstLane", result, std::move(arguments)));
    };
    reject(boolean, {});
    reject(boolean, {fixture.Integer(32, 110), fixture.Integer(32, 0)});
    reject(fixture.IntegerType(32), {fixture.Integer(32, 110)});
    reject(boolean, {fixture.Integer(16, 110)});
    reject(boolean, {fixture.Integer(32, 111)});
    reject(boolean, {fixture.Argument(0)});
    auto* dynamic = fixture.Make<LLVMBC::CastInst>(fixture.IntegerType(32), fixture.Argument(0), LLVMBC::Instruction::BitCast);
    reject(boolean, {dynamic});
    auto* first = fixture.Call("dx.op.waveIsFirstLane", boolean, {fixture.Integer(32, 110)});
    fixture.Expect(*fixture.Make<LLVMBC::SelectInst>(fixture.Integer(32, 1), fixture.Integer(32, 0), first), 1);
    std::array<std::uint32_t, 16> input{};
    fixture.Run(context, input);
}
}

void BufferTests::CheckWaveFirstGpu(AnyBox::Vulkan::Context& context) {
    std::vector<std::uint64_t> masks{0, 1, UINT64_MAX, 0xA55A8420A55A8420ULL, 0xFFFF000000000000ULL, 0x8000000080000000ULL};
    for (unsigned lane = 0; lane < 32; ++lane) masks.push_back(std::uint64_t{1} << lane);
    for (unsigned waveSize : {32u, 64u}) {
        for (auto mask : masks) {
            if (waveSize == 64) {
                Fixtures::Reject<std::runtime_error>([&] { (void)Make(mask, waveSize); });
                continue;
            }
            auto program = Make(mask, waveSize);
            ShaderRecompiler::ShaderComputeInputInfo info;
            info.threadsNum[0] = waveSize;
            info.threadsNum[1] = info.threadsNum[2] = 1;
            info.waveSize = waveSize;
            info.hostSubgroupSize = 32;
            auto shader = AnyBox::Shader::CompileCompute(program, info, Target(), {0, 0, 0, 128});
            auto used = waveSize == 32 ? mask & UINT32_MAX : mask;
            auto first = static_cast<unsigned>(std::countr_zero(used));
            std::vector<std::uint32_t> expected(waveSize * 2, UINT32_MAX);
            for (unsigned index = 0; index < expected.size(); ++index) {
                auto lane = index % waveSize;
                if (((used >> lane) & 1) != 0) expected[index] = lane == first ? 1 : 0;
            }
            RunGpu(context, shader, expected, {2, 1, 1});
        }
    }
    CheckInvalid(context);
}
