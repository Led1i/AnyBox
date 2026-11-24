#include "Device.hpp"
#include "../host/Library.hpp"
#include <d3d12_xs.hpp>
#include <anybox/Unimplemented.hpp>
#include <memory>
#include <stdexcept>

namespace {
constexpr GUID HostDeviceId{0x189819F1, 0x1DB6, 0x4B57, {0xBE, 0x54, 0x18, 0x21, 0x33, 0x9B, 0x85, 0xF7}};

struct DeviceDeleter {
    void operator()(ID3D12Device* value) const noexcept { value->Release(); }
};
}

std::int32_t D3D12XboxCreateDevice(IGraphicsUnknown* adapter, const D3D12XboxCreateDeviceParameters* parameters, const AnyBox::Guid& interfaceId, void** device) noexcept(false) {
    if (!device) throw std::invalid_argument("Graphics device output is null");
    *device = nullptr;
    if (!parameters) throw std::invalid_argument("Xbox graphics device parameters are null");
    if (parameters->version != 0x20004 || parameters->processDebugFlags != 0) ANYBOX_NOT_IMPLEMENTED();
    if (interfaceId != AnyBox::Graphics::XboxDeviceId && interfaceId != AnyBox::Graphics::UnknownId) throw std::invalid_argument("Unsupported Xbox graphics device interface");
    if (adapter) ANYBOX_NOT_IMPLEMENTED();
    static auto create = AnyBox::Graphics::GetHostLibrary().Get<PFN_D3D12_CREATE_DEVICE>("D3D12CreateDevice");
    void* host = nullptr;
    auto result = create(nullptr, D3D_FEATURE_LEVEL_12_0, HostDeviceId, &host);
    AnyBox::Graphics::CheckGraphicsResult(result, "D3D12CreateDevice");
    if (!host) throw std::runtime_error("D3D12CreateDevice returned no interface");
    std::unique_ptr<ID3D12Device, DeviceDeleter> ownedHost(static_cast<ID3D12Device*>(host));
    auto wrapper = std::make_unique<AnyBox::Graphics::Device>(ownedHost.get(), *parameters);
    ownedHost.release();
    *device = wrapper.release();
    return static_cast<std::int32_t>(result);
}
