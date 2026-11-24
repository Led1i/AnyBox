#pragma once

#include <anybox/platform/Windows.hpp>
#include <d3d12.h>

namespace AnyBox::Graphics {
D3D12_RESOURCE_ALLOCATION_INFO ComputeTextureAllocation(const D3D12_RESOURCE_DESC& description);
}
