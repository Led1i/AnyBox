#include "Tests.hpp"
#include "../../src/host/Com.hpp"
#include <anybox/platform/WindowsError.hpp>
#include <anybox/vulkan/Timeline.hpp>

namespace {
struct HandleDeleter {
    void operator()(void* value) const noexcept { CloseHandle(value); }
};
using Handle = std::unique_ptr<void, HandleDeleter>;
}

void AnyBox::Graphics::Tests::CheckVulkanTimeline(Device* device) {
    Vulkan::Context context(0);
    auto luid = device->GetHost()->GetAdapterLuid();
    Require(std::memcmp(context.GetAdapterLuid().data(), &luid, sizeof(luid)) == 0, "Vulkan and D3D12 adapters differ");
    constexpr GUID fenceId{0x0A753DCF, 0xC4D8, 0x4B91, {0xAD, 0xF6, 0xBE, 0x5A, 0x60, 0xD9, 0x5A, 0x76}};
    constexpr GUID queueId{0x0EC870A6, 0x5D7E, 0x4C22, {0x8C, 0xFC, 0x5B, 0xAA, 0xE0, 0x76, 0x16, 0xED}};
    void* output = nullptr;
    Windows::CheckHResult(device->GetHost()->CreateFence(11, D3D12_FENCE_FLAG_SHARED, fenceId, &output), "Create shared interop fence");
    ComOwner<ID3D12Fence> fence(static_cast<ID3D12Fence*>(output));
    HANDLE shared = nullptr;
    Windows::CheckHResult(device->GetHost()->CreateSharedHandle(fence.get(), nullptr, GENERIC_ALL, nullptr, &shared), "Export D3D12 interop fence");
    Handle sharedHandle(shared);
    auto timeline = Vulkan::Timeline::ImportD3D12Fence(context, sharedHandle.get());
    sharedHandle.reset();
    Require(timeline.GetValue() == 11 && timeline.Wait(11, 0) && !timeline.Wait(12, 0), "Vulkan imported fence initial value changed");
    Handle event(CreateEventW(nullptr, FALSE, FALSE, nullptr));
    if (!event) Windows::ThrowLastError("Create Vulkan interop fence event");

    Windows::CheckHResult(fence->SetEventOnCompletion(13, event.get()), "Register interop fence event");
    timeline.SignalOnQueue(0, 13);
    Require(WaitForSingleObject(event.get(), 5000) == WAIT_OBJECT_0 && fence->GetCompletedValue() == 13 && timeline.Wait(13, 0), "Vulkan GPU signal did not reach the D3D12 fence event");
    timeline.WaitOnQueue(0, 19);
    timeline.SignalOnQueue(0, 23);
    bool premature = timeline.Wait(23, 1000000);
    Windows::CheckHResult(fence->Signal(19), "Signal interop fence from the CPU");
    Require(timeline.Wait(23, 5000000000ull) && !premature, "D3D12 CPU signal did not release the Vulkan queue wait");

    D3D12_COMMAND_QUEUE_DESC description{};
    Windows::CheckHResult(device->GetHost()->CreateCommandQueue(&description, queueId, &output), "Create D3D12 interop queue");
    ComOwner<ID3D12CommandQueue> queue(static_cast<ID3D12CommandQueue*>(output));
    Windows::CheckHResult(queue->Wait(fence.get(), 29), "Wait for Vulkan signal on D3D12 queue");
    Windows::CheckHResult(queue->Signal(fence.get(), 31), "Signal D3D12 queue after Vulkan wait");
    timeline.SignalOnQueue(0, 29);
    Require(timeline.Wait(31, 5000000000ull), "Vulkan signal did not release the D3D12 queue wait");

    timeline.WaitOnQueue(0, 37);
    timeline.SignalOnQueue(0, 41);
    Windows::CheckHResult(queue->Signal(fence.get(), 37), "Signal Vulkan wait from D3D12 queue");
    Require(timeline.Wait(41, 5000000000ull) && fence->GetCompletedValue() == 41, "D3D12 queue signal did not release the Vulkan queue wait");
    std::cout << "Verified D3D12/Vulkan GPU timeline synchronization and Windows fence events\n";
}
