#include "Backing.hpp"
#include <anybox/platform/WindowsError.hpp>
#include <anybox/Unimplemented.hpp>
#include <limits>

AnyBox::Graphics::BufferBacking AnyBox::Graphics::CreateBufferBacking(Device* device, std::uintptr_t address, const D3D12_RESOURCE_DESC1& description) {
    static_assert(sizeof(D3D12_RESOURCE_DESC1) == 64);
    if (description.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER) ANYBOX_NOT_IMPLEMENTED();
    if (address == 0 || address % PhysicalPageSize != 0 || description.Width == 0 || description.Width > std::numeric_limits<std::size_t>::max() - (PhysicalPageSize - 1)) throw std::invalid_argument("Placed buffer address or width is invalid");
    if (description.Height != 1 || description.DepthOrArraySize != 1 || description.MipLevels != 1 || description.Format != DXGI_FORMAT_UNKNOWN || description.SampleDesc.Count != 1 || description.SampleDesc.Quality != 0 || description.Layout != D3D12_TEXTURE_LAYOUT_ROW_MAJOR) throw std::invalid_argument("Placed buffer description is invalid");
    if (description.Alignment != 0 && description.Alignment != PhysicalPageSize) ANYBOX_NOT_IMPLEMENTED();
    if (description.SamplerFeedbackMipRegion.Width != 0 || description.SamplerFeedbackMipRegion.Height != 0 || description.SamplerFeedbackMipRegion.Depth != 0) throw std::invalid_argument("Placed buffer cannot have a sampler feedback mip region");
    constexpr auto xboxFlags = 0x20000u;
    constexpr auto nativeFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE | D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER | D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
    if ((static_cast<std::uint32_t>(description.Flags) & ~(static_cast<std::uint32_t>(nativeFlags) | xboxFlags)) != 0) ANYBOX_NOT_IMPLEMENTED();
    auto count = static_cast<std::size_t>((description.Width + PhysicalPageSize - 1) / PhysicalPageSize);
    BufferBacking backing;
    backing.pages.reset(AnyBoxAcquireMappedPhysicalPages(reinterpret_cast<void*>(address), count));
    auto memory = backing.pages->GetContiguousMemory();
    if (memory.empty()) throw std::runtime_error("GPU buffers spanning noncontiguous physical backing are not implemented");
    MEMORY_BASIC_INFORMATION mapping{};
    if (!VirtualQuery(memory.data(), &mapping, sizeof(mapping))) Windows::ThrowLastError("VirtualQuery placed buffer backing");
    auto base = reinterpret_cast<std::uintptr_t>(mapping.AllocationBase);
    auto offset = reinterpret_cast<std::uintptr_t>(memory.data()) - base;
    constexpr GUID deviceId{0x517F8718, 0xAA66, 0x49F9, {0xB0, 0x2B, 0xA7, 0xAB, 0x89, 0xC0, 0x60, 0x31}};
    constexpr GUID heapId{0x6B3B2502, 0x6E51, 0x45B3, {0x90, 0xEE, 0x98, 0x84, 0x26, 0x5E, 0x8D, 0xF3}};
    constexpr GUID resourceId{0x696442BE, 0xA72E, 0x4059, {0xBC, 0x79, 0x5B, 0x5C, 0x98, 0x04, 0x0F, 0xAD}};
    void* output = nullptr;
    Windows::CheckHResult(device->GetHost()->QueryInterface(deviceId, &output), "Placed buffer device interface");
    if (!output) throw std::runtime_error("Placed buffer device interface is null");
    ComOwner<ID3D12Device10> native(static_cast<ID3D12Device10*>(output));
    Windows::CheckHResult(native->OpenExistingHeapFromAddress(mapping.AllocationBase, heapId, &output), "Open placed buffer physical heap");
    backing.heap.reset(static_cast<ID3D12Heap*>(output));
    if (!backing.heap) throw std::runtime_error("Placed buffer heap interface is null");
    auto heap = backing.heap->GetDesc();
    if (offset > heap.SizeInBytes || memory.size() > heap.SizeInBytes - offset || heap.Properties.CPUPageProperty != D3D12_CPU_PAGE_PROPERTY_WRITE_BACK || heap.Properties.MemoryPoolPreference != D3D12_MEMORY_POOL_L0) throw std::runtime_error("Placed buffer heap does not cover coherent physical memory");
    auto translated = description;
    translated.Flags = static_cast<D3D12_RESOURCE_FLAGS>(static_cast<std::uint32_t>(description.Flags) & ~xboxFlags);
    if ((heap.Flags & D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER) != 0) translated.Flags |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
    Windows::CheckHResult(native->CreatePlacedResource2(backing.heap.get(), offset, &translated, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, resourceId, &output), "Create placed physical buffer");
    backing.resource.reset(static_cast<ID3D12Resource*>(output));
    if (!backing.resource) throw std::runtime_error("Placed buffer resource interface is null");
    return backing;
}
