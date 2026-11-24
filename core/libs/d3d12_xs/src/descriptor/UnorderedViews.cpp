#include "../device/Device.hpp"
#include <anybox/Unimplemented.hpp>
#include <cstddef>

static_assert(sizeof(D3D12_UNORDERED_ACCESS_VIEW_DESC) == 40);
static_assert(offsetof(D3D12_UNORDERED_ACCESS_VIEW_DESC, Buffer) == 8);
static_assert(offsetof(D3D12_UNORDERED_ACCESS_VIEW_DESC, Buffer) + offsetof(D3D12_BUFFER_UAV, StructureByteStride) == 20);
static_assert(offsetof(D3D12_UNORDERED_ACCESS_VIEW_DESC, Buffer) + offsetof(D3D12_BUFFER_UAV, Flags) == 32);

void AnyBox::Graphics::Device::CreateUnorderedAccessView(Device*, void* resource, void*, const D3D12_UNORDERED_ACCESS_VIEW_DESC* description, std::uint64_t destination) {
    if (destination == 0 || destination % 16 != 0) throw std::invalid_argument("Xbox unordered view destination is null or misaligned");
    if (resource) ANYBOX_NOT_IMPLEMENTED();
    std::array<std::uint64_t, 4> words{};
    if (description && description->ViewDimension == D3D12_UAV_DIMENSION_BUFFER && static_cast<std::uint32_t>(description->Buffer.Flags) == 2 && description->Buffer.StructureByteStride != 0 && description->Format != DXGI_FORMAT_UNKNOWN) {
        auto offset = static_cast<std::uint32_t>(description->Buffer.FirstElement);
        auto stride = static_cast<std::uint64_t>(description->Buffer.StructureByteStride & 0x3FFF) << 48;
        words[0] = offset | stride;
    }
    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(destination)), words.data(), sizeof(words));
}
