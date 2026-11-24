#include "Device.hpp"
#include <anybox/vulkan/Context.hpp>
#include <memory>

AnyBox::Graphics::Device::Device(ID3D12Device* host, const D3D12XboxCreateDeviceParameters& parameters) : vtable(&GetDeviceTable()), host(host), parameters(parameters) {
    auto nativeLuid = host->GetAdapterLuid();
    Vulkan::AdapterLuid luid{};
    static_assert(sizeof(nativeLuid) == sizeof(luid));
    std::memcpy(luid.data(), &nativeLuid, luid.size());
    auto created = std::make_unique<Vulkan::Context>(luid, 5);
    host->AddRef();
    gpuContext = created.release();
}
