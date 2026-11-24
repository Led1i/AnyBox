#include "Device.hpp"
#include "../root/Signature.hpp"
#include <memory>
#include <stdexcept>

namespace {
struct RootDeleter {
    void operator()(ID3D12RootSignature* value) const noexcept { value->Release(); }
};
}

HRESULT AnyBox::Graphics::Device::CreateRootSignature(Device* self, std::uint32_t nodeMask, const void* data, std::size_t size, const Guid& interfaceId, void** signature) {
    if (!signature) throw std::invalid_argument("Root signature output is null");
    *signature = nullptr;
    if (!data || size == 0) throw std::invalid_argument("Root signature bytecode is empty");
    if (!RootSignature::Supports(interfaceId)) throw std::invalid_argument("Unsupported root signature creation interface");
    void* host = nullptr;
    auto result = self->host->CreateRootSignature(nodeMask, data, size, NativeGuid(RootSignatureId), &host);
    CheckGraphicsResult(result, "ID3D12Device::CreateRootSignature");
    if (!host) throw std::runtime_error("D3D12 root signature creation returned no interface");
    std::unique_ptr<ID3D12RootSignature, RootDeleter> ownedHost(static_cast<ID3D12RootSignature*>(host));
    auto wrapper = std::make_unique<RootSignature>(ownedHost.get(), self);
    ownedHost.release();
    *signature = wrapper.release();
    return result;
}
