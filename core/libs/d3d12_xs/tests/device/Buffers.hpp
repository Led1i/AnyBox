#pragma once

#include "Tests.hpp"
#include <memory>

namespace AnyBox::Graphics::Tests {
struct ResourceDeleter {
    void operator()(ID3D12Resource* value) const noexcept { value->Release(); }
};
using NativeResource = std::unique_ptr<ID3D12Resource, ResourceDeleter>;
inline constexpr std::size_t TransferSize = 65536;

struct NativeBuffers {
    NativeResource upload;
    NativeResource device;
    NativeResource readback;
};

NativeBuffers CreateNativeBuffers(Device* device);
void VerifyReadback(ID3D12Resource* resource);
}
