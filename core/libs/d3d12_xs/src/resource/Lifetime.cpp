#include "Resource.hpp"
#include "../object/PrivateData.hpp"

AnyBox::Graphics::Resource::Resource(BufferBacking backing, Device* device, std::uintptr_t address, const D3D12_RESOURCE_DESC1& description) : vtable(&GetResourceTable()), buffer(backing.resource.get()), device(device), pages(backing.pages.get()), address(address), description(description) {
    Device::AddRef(device);
    backing.resource.release();
    backing.pages.release();
}

AnyBox::Graphics::Resource::~Resource() {
    delete privateData.load(std::memory_order_relaxed);
    delete buffer;
    pages->Release();
    Device::Release(device);
}

bool AnyBox::Graphics::Resource::Supports(const Guid& id) noexcept { return id == ResourceId || id == Resource1Id || id == Resource2Id || id == UnknownId || id == ObjectId || id == DeviceChildId || id == PageableId; }
