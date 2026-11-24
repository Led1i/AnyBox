#include "Queue.hpp"
#include <limits>
#include <stdexcept>

AnyBox::Graphics::Queue::Queue(ID3D12CommandQueue* host, Device* device, const D3D12_COMMAND_QUEUE_DESC& description) : vtable(&GetQueueTable()), host(host), device(device), description(description) { Device::AddRef(device); }

AnyBox::Graphics::Queue::~Queue() {
    host->Release();
    if (xboxSelection) device->ReleaseQueueSelection(xboxSelection);
    Device::Release(device);
}

bool AnyBox::Graphics::Queue::Supports(const Guid& interfaceId) noexcept { return interfaceId == QueueId || interfaceId == UnknownId || interfaceId == ObjectId || interfaceId == DeviceChildId || interfaceId == PageableId; }

std::int32_t AnyBox::Graphics::Queue::QueryInterface(Queue* self, const Guid& interfaceId, void** object) {
    if (!object) throw std::invalid_argument("Command queue interface output is null");
    *object = nullptr;
    if (!Supports(interfaceId)) throw std::invalid_argument("Unsupported command queue interface");
    AddRef(self);
    *object = self;
    return 0;
}

std::uint32_t AnyBox::Graphics::Queue::AddRef(Queue* self) {
    auto previous = self->references.load(std::memory_order_relaxed);
    do {
        if (previous == 0 || previous == std::numeric_limits<std::uint32_t>::max()) throw std::overflow_error("Command queue reference count is invalid");
    } while (!self->references.compare_exchange_weak(previous, previous + 1, std::memory_order_relaxed));
    return previous + 1;
}

std::uint32_t AnyBox::Graphics::Queue::Release(Queue* self) {
    auto previous = self->references.fetch_sub(1, std::memory_order_acq_rel);
    if (previous == 0) throw std::logic_error("Command queue reference count underflows");
    if (previous == 1) delete self;
    return previous - 1;
}

std::int32_t AnyBox::Graphics::Queue::GetDevice(Queue* self, const Guid& interfaceId, void** device) { return Device::QueryInterface(self->device, interfaceId, device); }
