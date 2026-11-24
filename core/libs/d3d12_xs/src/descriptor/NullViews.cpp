#include "../device/Device.hpp"
#include <anybox/Unimplemented.hpp>

void AnyBox::Graphics::Device::CreateShaderResourceView(Device*, void* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* description, std::uint64_t destination) {
    if (destination == 0 || destination % 16 != 0) throw std::invalid_argument("Xbox shader view destination is null or misaligned");
    if (resource) ANYBOX_NOT_IMPLEMENTED();
    auto* output = reinterpret_cast<void*>(static_cast<std::uintptr_t>(destination));
    if (description && description->ViewDimension == D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE) {
        std::array<std::uint64_t, 2> words{description->RaytracingAccelerationStructure.Location & 0xFFFFFFFFFFFFULL, 0x1400000000000ULL};
        std::memcpy(output, words.data(), sizeof(words));
    } else std::memset(output, 0, 32);
}
