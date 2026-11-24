#pragma once

#include "../device/Device.hpp"
#include <dxgi1_4.h>
#include <memory>

namespace AnyBox::Graphics {
struct AdapterDeleter {
    void operator()(IDXGIAdapter1* value) const noexcept { value->Release(); }
};
using NativeAdapter = std::unique_ptr<IDXGIAdapter1, AdapterDeleter>;
NativeAdapter GetHostAdapter(Device* device);
}
