#pragma once

#include "../device/Device.hpp"
#include <anybox/vulkan/Image.hpp>

namespace AnyBox::Graphics {
struct TextureBacking {
    Vulkan::Image image;
    D3D12_HEAP_PROPERTIES properties;
    D3D12_HEAP_FLAGS flags;
    D3D12_BARRIER_LAYOUT initialLayout;
};

std::unique_ptr<TextureBacking> CreateTextureBacking(Device* device, const D3D12_HEAP_PROPERTIES& properties, D3D12_HEAP_FLAGS flags, const D3D12_RESOURCE_DESC1& description, D3D12_BARRIER_LAYOUT layout, const D3D12_CLEAR_VALUE* clearValue);
VkFormat TextureFormat(DXGI_FORMAT format);
}
