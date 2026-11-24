#include "InsertFixture.hpp"
#include <anybox/platform/WindowsError.hpp>
#include <memory>
#include <cstring>

namespace {
struct Release {
    void operator()(void* value) const noexcept { if (!VirtualFree(value, 0, MEM_RELEASE)) std::terminate(); }
};
}

void AnyBox::Relinker::Tests::CheckInsertUnwind(const LoweredFunction& function) {
    std::unique_ptr<void, Release> memory(VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (!memory) Windows::ThrowLastError("Allocate INSERTQ unwind test image");
    auto* bytes = static_cast<std::byte*>(memory.get());
    std::memcpy(bytes, function.code.data(), function.code.size());
    const std::array<std::uint8_t, 12> unwind{1, 5, 1, 0, 5, 0x42, 0, 0, 1, 0, 0, 0};
    std::memcpy(bytes + 2048, unwind.data(), unwind.size());
    Require(function.ranges.size() == 2 && function.ranges[0].stackSize == 40 && function.ranges[0].prologSize == 5 && function.ranges[1].stackSize == 0, "INSERTQ unwind ranges do not describe its generated stack states");
    std::array<RUNTIME_FUNCTION, 2> ranges{};
    for (std::size_t index = 0; index < ranges.size(); ++index) {
        ranges[index].BeginAddress = function.ranges[index].begin;
        ranges[index].EndAddress = function.ranges[index].end;
        ranges[index].UnwindData = index == 0 ? 2048 : 2056;
    }
    alignas(16) std::array<std::uint64_t, 64> stack{};
    stack[17] = 0x12345678;
    auto entryStack = reinterpret_cast<DWORD64>(&stack[17]);
    auto base = reinterpret_cast<DWORD64>(memory.get());
    Decoder decoder;
    for (std::size_t position = 0; position < function.code.size();) {
        auto instruction = decoder.Decode(std::span(function.code).subspan(position));
        bool returnedStack = position >= function.ranges[1].begin;
        CONTEXT context{};
        context.Rip = base + position;
        context.Rsp = entryStack - (position >= 5 && !returnedStack ? 40 : 0);
        PVOID handlerData = nullptr;
        DWORD64 frame = 0;
        RtlVirtualUnwind(UNW_FLAG_NHANDLER, base, context.Rip, &ranges[returnedStack ? 1 : 0], &context, &handlerData, &frame, nullptr);
        Require(context.Rip == stack[17] && context.Rsp == entryStack + 8, "INSERTQ helper did not unwind correctly at an instruction boundary");
        position += instruction.native.length;
    }
}
