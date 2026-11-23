#include <d3d12_xs.hpp>
#include <anybox/Unimplemented.hpp>

std::int32_t D3D12ConfigureVirtualMemoryX(D3D12VirtualMemoryConfiguration*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t D3D12CreateRootSignatureDeserializer(const void*, std::size_t, const AnyBox::Guid&, void**) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t D3D12SerializeVersionedRootSignature(const D3D12VersionedRootSignatureDescription*, ID3DBlob**, ID3DBlob**) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t D3D12XboxCreateDevice(IGraphicsUnknown*, const D3D12XboxCreateDeviceParameters*, const AnyBox::Guid&, void**) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}
