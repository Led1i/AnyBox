#ifndef ANYBOX_D3D12_XS_TYPES_HPP
#define ANYBOX_D3D12_XS_TYPES_HPP

#include <array>
#include <cstddef>
#include <cstdint>

struct IGraphicsUnknown;
struct ID3D10Blob;
using ID3DBlob = ID3D10Blob;
struct _GUID;
struct D3D12VirtualMemoryConfiguration {
    std::uint32_t flags;
    std::uint32_t pageTablePageCount;
    std::uint32_t framePreserveBufferSize;
    std::uint32_t reserved;
    std::array<std::uintptr_t, 2> backendData;
};

static_assert(sizeof(D3D12VirtualMemoryConfiguration) == 32);
static_assert(offsetof(D3D12VirtualMemoryConfiguration, backendData) == 16);
struct D3D12_VERSIONED_ROOT_SIGNATURE_DESC;
using D3D12VersionedRootSignatureDescription = D3D12_VERSIONED_ROOT_SIGNATURE_DESC;
struct D3D12XboxCreateDeviceParameters;

#endif
