#pragma once

#include "../device/Device.hpp"
#include "../host/Com.hpp"
#include <anybox/PhysicalMemory.hpp>

namespace AnyBox::Graphics {
struct BufferBacking {
    ComOwner<IPhysicalPageLease> pages;
    ComOwner<ID3D12Heap> heap;
    ComOwner<ID3D12Resource> resource;
};
BufferBacking CreateBufferBacking(Device* device, std::uintptr_t address, const D3D12_RESOURCE_DESC1& description);
}
