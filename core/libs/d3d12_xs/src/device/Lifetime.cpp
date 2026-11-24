#include "Device.hpp"
#include <limits>
#include <stdexcept>

AnyBox::Graphics::Device::Device(ID3D12Device* host, const D3D12XboxCreateDeviceParameters& parameters) : vtable(&GetDeviceTable()), host(host), parameters(parameters) {}
AnyBox::Graphics::Device::~Device() { host->Release(); }

std::int32_t AnyBox::Graphics::Device::QueryInterface(Device* self, const Guid& interfaceId, void** object) {
    if (!object) throw std::invalid_argument("Graphics device interface output is null");
    *object = nullptr;
    if (interfaceId != XboxDeviceId && interfaceId != UnknownId) throw std::invalid_argument("Unsupported graphics device interface");
    AddRef(self);
    *object = self;
    return 0;
}

std::uint32_t AnyBox::Graphics::Device::AddRef(Device* self) {
    auto previous = self->references.load(std::memory_order_relaxed);
    do {
        if (previous == 0 || previous == std::numeric_limits<std::uint32_t>::max()) throw std::overflow_error("Graphics device reference count is invalid");
    } while (!self->references.compare_exchange_weak(previous, previous + 1, std::memory_order_relaxed));
    return previous + 1;
}

std::uint32_t AnyBox::Graphics::Device::Release(Device* self) {
    auto previous = self->references.fetch_sub(1, std::memory_order_acq_rel);
    if (previous == 0) throw std::logic_error("Graphics device reference count underflows");
    if (previous == 1) delete self;
    return previous - 1;
}
