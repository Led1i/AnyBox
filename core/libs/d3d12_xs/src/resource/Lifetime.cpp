#include "Resource.hpp"

AnyBox::Graphics::Resource::Resource(BufferBacking backing, Device* device, std::uintptr_t address, const D3D12_RESOURCE_DESC1& description) : vtable(&GetResourceTable()), host(backing.resource.get()), device(device), heap(backing.heap.get()), pages(backing.pages.get()), address(address), description(description) {
    Device::AddRef(device);
    backing.resource.release();
    backing.heap.release();
    backing.pages.release();
}

AnyBox::Graphics::Resource::~Resource() {
    host->Release();
    heap->Release();
    pages->Release();
    Device::Release(device);
}

bool AnyBox::Graphics::Resource::Supports(const Guid& id) noexcept { return id == ResourceId || id == Resource1Id || id == Resource2Id || id == UnknownId || id == ObjectId || id == DeviceChildId || id == PageableId; }
