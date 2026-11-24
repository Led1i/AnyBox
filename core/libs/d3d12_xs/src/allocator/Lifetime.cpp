#include "Allocator.hpp"
#include <limits>
#include <stdexcept>

AnyBox::Graphics::Allocator::Allocator(ID3D12CommandAllocator* host, Device* device, D3D12_COMMAND_LIST_TYPE type) : vtable(&GetAllocatorTable()), host(host), device(device), type(type) { Device::AddRef(device); }

AnyBox::Graphics::Allocator::~Allocator() {
    host->Release();
    Device::Release(device);
}

AnyBox::Graphics::Allocator* AnyBox::Graphics::Allocator::From(void* object) {
    if (!object) throw std::invalid_argument("Allocator is null");
    const DeviceTable* table;
    std::memcpy(&table, object, sizeof(table));
    if (table != &GetAllocatorTable()) throw std::invalid_argument("Foreign fence interface is unsupported");
    return static_cast<Allocator*>(object);
}

bool AnyBox::Graphics::Allocator::Supports(const Guid& interfaceId) noexcept { return interfaceId == AllocatorId || interfaceId == UnknownId || interfaceId == ObjectId || interfaceId == DeviceChildId || interfaceId == PageableId; }

HRESULT AnyBox::Graphics::Allocator::QueryInterface(Allocator* self, const Guid& interfaceId, void** object) {
    if (!object) throw std::invalid_argument("Allocator interface output is null");
    *object = nullptr;
    if (!Supports(interfaceId)) throw std::invalid_argument("Unsupported fence interface");
    AddRef(self);
    *object = self;
    return 0;
}

std::uint32_t AnyBox::Graphics::Allocator::AddRef(Allocator* self) {
    auto previous = self->references.load(std::memory_order_relaxed);
    do {
        if (previous == 0 || previous == std::numeric_limits<std::uint32_t>::max()) throw std::overflow_error("Allocator reference count is invalid");
    } while (!self->references.compare_exchange_weak(previous, previous + 1, std::memory_order_relaxed));
    return previous + 1;
}

std::uint32_t AnyBox::Graphics::Allocator::Release(Allocator* self) {
    auto previous = self->references.fetch_sub(1, std::memory_order_acq_rel);
    if (previous == 0) throw std::logic_error("Allocator reference count underflows");
    if (previous == 1) delete self;
    return previous - 1;
}

HRESULT AnyBox::Graphics::Allocator::GetDevice(Allocator* self, const Guid& interfaceId, void** device) { return Device::QueryInterface(self->device, interfaceId, device); }

HRESULT AnyBox::Graphics::Allocator::Reset(Allocator* self) {
    auto result = self->host->Reset();
    CheckGraphicsResult(result, "ID3D12CommandAllocator::Reset");
    return result;
}
