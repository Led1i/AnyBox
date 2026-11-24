#include <d3d12_xs.hpp>
#include "../host/Library.hpp"
#include <d3d12.h>
#include <stdexcept>

long D3D12CreateRootSignatureDeserializer(const void* sourceData, std::size_t sourceDataSize, const _GUID& interfaceId, void** deserializer) noexcept(false) {
    if (!deserializer) throw std::invalid_argument("Root signature deserializer output is null");
    *deserializer = nullptr;
    if (!sourceData || sourceDataSize == 0) throw std::invalid_argument("Root signature source data is empty");
    static auto deserialize = AnyBox::Graphics::GetHostLibrary().Get<PFN_D3D12_CREATE_ROOT_SIGNATURE_DESERIALIZER>("D3D12CreateRootSignatureDeserializer");
    auto result = deserialize(sourceData, sourceDataSize, interfaceId, deserializer);
    if (FAILED(result)) throw std::system_error(static_cast<int>(result), std::system_category(), "D3D12CreateRootSignatureDeserializer");
    if (!*deserializer) throw std::runtime_error("Root signature deserializer returned no interface");
    return result;
}
