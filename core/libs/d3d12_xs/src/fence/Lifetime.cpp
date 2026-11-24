#include "Fence.hpp"
#include <limits>
#include <stdexcept>

AnyBox::Graphics::Fence::Fence(ID3D12Fence* host, Device* device, D3D12_FENCE_FLAGS flags) : vtable(&GetFenceTable()), host(host), device(device), flags(flags) { Device::AddRef(device); }

AnyBox::Graphics::Fence::~Fence() {
    host->Release();
    Device::Release(device);
}

AnyBox::Graphics::Fence* AnyBox::Graphics::Fence::From(void* object) {
    if (!object) throw std::invalid_argument("Fence is null");
    const DeviceTable* table;
    std::memcpy(&table, object, sizeof(table));
    if (table != &GetFenceTable()) throw std::invalid_argument("Foreign fence interface is unsupported");
    return static_cast<Fence*>(object);
}

bool AnyBox::Graphics::Fence::Supports(const Guid& interfaceId) noexcept { return interfaceId == FenceId || interfaceId == Fence1Id || interfaceId == UnknownId || interfaceId == ObjectId || interfaceId == DeviceChildId || interfaceId == PageableId; }

HRESULT AnyBox::Graphics::Fence::QueryInterface(Fence* self, const Guid& interfaceId, void** object) {
    if (!object) throw std::invalid_argument("Fence interface output is null");
    *object = nullptr;
    if (!Supports(interfaceId)) throw std::invalid_argument("Unsupported fence interface");
    AddRef(self);
    *object = self;
    return 0;
}

std::uint32_t AnyBox::Graphics::Fence::AddRef(Fence* self) {
    auto previous = self->references.load(std::memory_order_relaxed);
    do {
        if (previous == 0 || previous == std::numeric_limits<std::uint32_t>::max()) throw std::overflow_error("Fence reference count is invalid");
    } while (!self->references.compare_exchange_weak(previous, previous + 1, std::memory_order_relaxed));
    return previous + 1;
}

std::uint32_t AnyBox::Graphics::Fence::Release(Fence* self) {
    auto previous = self->references.fetch_sub(1, std::memory_order_acq_rel);
    if (previous == 0) throw std::logic_error("Fence reference count underflows");
    if (previous == 1) delete self;
    return previous - 1;
}

HRESULT AnyBox::Graphics::Fence::GetDevice(Fence* self, const Guid& interfaceId, void** device) { return Device::QueryInterface(self->device, interfaceId, device); }
