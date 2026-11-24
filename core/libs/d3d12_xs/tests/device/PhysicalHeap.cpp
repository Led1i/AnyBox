#include "Buffers.hpp"
#include "../../src/host/Com.hpp"
#include <anybox/PhysicalMemory.hpp>
#include <anybox/platform/WindowsError.hpp>
#include <xmem.hpp>
#include <vector>

namespace {
class PhysicalStorage {
public:
    PhysicalStorage() {
        auto count = frames.size();
        if (XMemAllocatePhysicalPages(0x280, &count, frames.data()) != 1 || count != frames.size()) throw std::runtime_error("Physical GPU test allocation failed");
        try { lease.reset(AnyBoxAcquirePhysicalPages(2, frames.data() + 1)); }
        catch (...) { XMemFreePhysicalPages(frames.size(), frames.data()); throw; }
    }
    ~PhysicalStorage() {
        lease.reset();
        XMemFreePhysicalPages(frames.size(), frames.data());
    }
    std::span<std::byte> GetMemory() const { return lease->GetContiguousMemory(); }

private:
    std::array<std::uint64_t, 3> frames{};
    AnyBox::Graphics::ComOwner<AnyBox::IPhysicalPageLease> lease;
};
}

void AnyBox::Graphics::Tests::CheckPhysicalHeap(Device* device) {
    constexpr GUID deviceId{0x81DADC15, 0x2BAD, 0x4392, {0x93, 0xC5, 0x10, 0x13, 0x45, 0xC4, 0xAA, 0x98}};
    constexpr GUID heapId{0x6B3B2502, 0x6E51, 0x45B3, {0x90, 0xEE, 0x98, 0x84, 0x26, 0x5E, 0x8D, 0xF3}};
    constexpr GUID resourceId{0x696442BE, 0xA72E, 0x4059, {0xBC, 0x79, 0x5B, 0x5C, 0x98, 0x04, 0x0F, 0xAD}};
    constexpr GUID allocatorId{0x6102DEE4, 0xAF59, 0x4B09, {0xB9, 0x99, 0xB4, 0x4D, 0x73, 0xF0, 0x9B, 0x24}};
    constexpr GUID listId{0x5B160D0F, 0xAC1B, 0x4185, {0x8B, 0xA8, 0xB3, 0xAE, 0x42, 0xA5, 0xA4, 0x55}};
    constexpr GUID queueId{0x0EC870A6, 0x5D7E, 0x4C22, {0x8C, 0xFC, 0x5B, 0xAA, 0xE0, 0x76, 0x16, 0xED}};
    constexpr GUID fenceId{0x0A753DCF, 0xC4D8, 0x4B91, {0xAD, 0xF6, 0xBE, 0x5A, 0x60, 0xD9, 0x5A, 0x76}};
    PhysicalStorage storage;
    auto memory = storage.GetMemory();
    Require(memory.size() == 2 * PhysicalPageSize, "Physical GPU memory is not contiguous");
    std::vector<std::byte> expected(TransferSize);
    for (std::size_t index = 0; index < expected.size(); ++index) expected[index] = static_cast<std::byte>((index * 71) ^ (index >> 9));
    std::memcpy(memory.data(), expected.data(), expected.size());
    void* output = nullptr;
    Windows::CheckHResult(device->GetHost()->QueryInterface(deviceId, &output), "Physical GPU memory device interface");
    ComOwner<ID3D12Device3> native(static_cast<ID3D12Device3*>(output));
    MEMORY_BASIC_INFORMATION mapping{};
    if (!VirtualQuery(memory.data(), &mapping, sizeof(mapping))) Windows::ThrowLastError("VirtualQuery physical GPU heap");
    Windows::CheckHResult(native->OpenExistingHeapFromAddress(mapping.AllocationBase, heapId, &output), "Open physical GPU memory heap");
    ComOwner<ID3D12Heap> heap(static_cast<ID3D12Heap*>(output));
    auto offset = reinterpret_cast<std::uintptr_t>(memory.data()) - reinterpret_cast<std::uintptr_t>(mapping.AllocationBase);
    Require(offset == PhysicalPageSize && heap->GetDesc().SizeInBytes >= offset + memory.size(), "Physical GPU heap did not retain the underlying section and offset");
    D3D12_RESOURCE_DESC description{};
    description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    description.Width = memory.size();
    description.Height = 1;
    description.DepthOrArraySize = 1;
    description.MipLevels = 1;
    description.SampleDesc.Count = 1;
    description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    description.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
    Windows::CheckHResult(native->CreatePlacedResource(heap.get(), offset, &description, D3D12_RESOURCE_STATE_COMMON, nullptr, resourceId, &output), "Create physical GPU buffer");
    ComOwner<ID3D12Resource> resource(static_cast<ID3D12Resource*>(output));
    auto buffers = CreateNativeBuffers(device);
    Windows::CheckHResult(native->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, allocatorId, &output), "Physical GPU memory allocator");
    ComOwner<ID3D12CommandAllocator> allocator(static_cast<ID3D12CommandAllocator*>(output));
    Windows::CheckHResult(native->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.get(), nullptr, listId, &output), "Physical GPU memory command list");
    ComOwner<ID3D12GraphicsCommandList> list(static_cast<ID3D12GraphicsCommandList*>(output));
    list->CopyBufferRegion(buffers.readback.get(), 0, resource.get(), 0, TransferSize);
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition = {resource.get(), D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST};
    list->ResourceBarrier(1, &barrier);
    list->CopyBufferRegion(resource.get(), 0, buffers.upload.get(), 0, TransferSize);
    Windows::CheckHResult(list->Close(), "Close physical GPU memory command list");
    D3D12_COMMAND_QUEUE_DESC queueDescription{D3D12_COMMAND_LIST_TYPE_DIRECT, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0};
    Windows::CheckHResult(native->CreateCommandQueue(&queueDescription, queueId, &output), "Physical GPU memory queue");
    ComOwner<ID3D12CommandQueue> queue(static_cast<ID3D12CommandQueue*>(output));
    Windows::CheckHResult(native->CreateFence(0, D3D12_FENCE_FLAG_NONE, fenceId, &output), "Physical GPU memory fence");
    ComOwner<ID3D12Fence> fence(static_cast<ID3D12Fence*>(output));
    ID3D12CommandList* commands[]{list.get()};
    queue->ExecuteCommandLists(1, commands);
    Windows::CheckHResult(queue->Signal(fence.get(), 1), "Signal physical GPU memory completion");
    Windows::CheckHResult(fence->SetEventOnCompletion(1, nullptr), "Wait physical GPU memory completion");
    Windows::CheckHResult(native->GetDeviceRemovedReason(), "Physical GPU memory device status");
    void* mapped = nullptr;
    D3D12_RANGE range{0, TransferSize};
    Windows::CheckHResult(buffers.readback->Map(0, &range, &mapped), "Read physical GPU memory transfer");
    Require(std::memcmp(mapped, expected.data(), expected.size()) == 0, "GPU did not read the actual XMem physical pages");
    range = {0, 0};
    buffers.readback->Unmap(0, &range);
    VerifyReadback(resource.get());
    Require(memory[18] == static_cast<std::byte>(18 * 37), "GPU writes did not reach the canonical XMem physical pages");
}
