#pragma once

#include <anybox/platform/Windows.hpp>
#include <d3d12.h>
#include <cstddef>
#include <cstdint>
#include <span>

namespace AnyBox::Graphics {
class RootDescription {
public:
    explicit RootDescription(std::span<const std::byte> data);
    ~RootDescription() { owner->Release(); }
    RootDescription(const RootDescription&) = delete;
    RootDescription& operator=(const RootDescription&) = delete;
    const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& GetDescription() const noexcept { return description; }
    std::uint32_t GetFlags() const noexcept { return flags; }

private:
    ID3D12VersionedRootSignatureDeserializer* owner;
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC description;
    std::uint32_t flags;
};
}
