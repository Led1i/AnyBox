#include "Device.hpp"
#include <algorithm>
#include <stdexcept>
#include <system_error>

GUID AnyBox::Graphics::NativeGuid(const Guid& value) {
    GUID result{value.data1, value.data2, value.data3, {}};
    std::copy(value.data4.begin(), value.data4.end(), result.Data4);
    return result;
}

void AnyBox::Graphics::CheckGraphicsResult(HRESULT result, const char* operation) {
    if (FAILED(result)) throw std::system_error(static_cast<int>(result), std::system_category(), operation);
}

HRESULT AnyBox::Graphics::Device::GetPrivateData(Device* self, const Guid& key, std::uint32_t* size, void* data) {
    if (!size) throw std::invalid_argument("Graphics private data size output is null");
    auto result = self->host->GetPrivateData(NativeGuid(key), size, data);
    CheckGraphicsResult(result, "ID3D12Device::GetPrivateData");
    return result;
}

HRESULT AnyBox::Graphics::Device::SetPrivateData(Device* self, const Guid& key, std::uint32_t size, const void* data) {
    if (size != 0 && !data) throw std::invalid_argument("Graphics private data is null");
    auto result = self->host->SetPrivateData(NativeGuid(key), size, data);
    CheckGraphicsResult(result, "ID3D12Device::SetPrivateData");
    return result;
}

HRESULT AnyBox::Graphics::Device::SetPrivateDataInterface(Device* self, const Guid& key, const IUnknown* object) {
    auto result = self->host->SetPrivateDataInterface(NativeGuid(key), object);
    CheckGraphicsResult(result, "ID3D12Device::SetPrivateDataInterface");
    return result;
}

HRESULT AnyBox::Graphics::Device::SetName(Device* self, const wchar_t* name) {
    if (!name) throw std::invalid_argument("Graphics device name is null");
    auto result = self->host->SetName(name);
    CheckGraphicsResult(result, "ID3D12Device::SetName");
    return result;
}

std::uint32_t AnyBox::Graphics::Device::GetNodeCount(Device* self) { return self->host->GetNodeCount(); }

D3D12_HEAP_PROPERTIES AnyBox::Graphics::Device::GetCustomHeapProperties(Device* self, std::uint32_t nodeMask, D3D12_HEAP_TYPE type) {
    if (type < D3D12_HEAP_TYPE_DEFAULT || type > D3D12_HEAP_TYPE_CUSTOM) throw std::invalid_argument("Invalid graphics heap type");
    auto count = self->host->GetNodeCount();
    if (count >= 32 || nodeMask == 0 || nodeMask >= (1u << count)) throw std::invalid_argument("Invalid graphics heap node mask");
    return self->host->GetCustomHeapProperties(nodeMask, type);
}
