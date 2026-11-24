#include "Resource.hpp"
#include <anybox/platform/WindowsError.hpp>

D3D12_RESOURCE_DESC AnyBox::Graphics::Resource::GetDescription(Resource* self) {
    D3D12_RESOURCE_DESC value{};
    static_assert(offsetof(D3D12_RESOURCE_DESC, Flags) == offsetof(D3D12_RESOURCE_DESC1, Flags));
    std::memcpy(&value, &self->description, offsetof(D3D12_RESOURCE_DESC, Flags) + sizeof(value.Flags));
    return value;
}

D3D12_RESOURCE_DESC1 AnyBox::Graphics::Resource::GetDescription1(Resource* self) { return self->description; }
std::uint64_t AnyBox::Graphics::Resource::GetGPUVirtualAddress(Resource* self) noexcept { return self->address; }

HRESULT AnyBox::Graphics::Resource::Map(Resource* self, std::uint32_t subresource, const D3D12_RANGE* range, void** data) {
    if (data) *data = nullptr;
    if (subresource != 0 || (range && (range->Begin > range->End || range->End > self->description.Width))) throw std::invalid_argument("Placed buffer map range is invalid");
    void* mapped = nullptr;
    auto result = self->host->Map(subresource, range, data ? &mapped : nullptr);
    Windows::CheckHResult(result, "Map placed physical buffer");
    if (data) {
        if (!mapped) throw std::runtime_error("Placed buffer mapping returned no CPU address");
        *data = reinterpret_cast<void*>(self->address);
    }
    return result;
}

void AnyBox::Graphics::Resource::Unmap(Resource* self, std::uint32_t subresource, const D3D12_RANGE* range) {
    if (subresource != 0 || (range && (range->Begin > range->End || range->End > self->description.Width))) throw std::invalid_argument("Placed buffer unmap range is invalid");
    self->host->Unmap(subresource, range);
}

HRESULT AnyBox::Graphics::Resource::GetHeapProperties(Resource* self, D3D12_HEAP_PROPERTIES* properties, D3D12_HEAP_FLAGS* flags) {
    if (!properties && !flags) throw std::invalid_argument("Placed buffer heap property outputs are null");
    auto result = self->host->GetHeapProperties(properties, flags);
    Windows::CheckHResult(result, "Get placed buffer heap properties");
    return result;
}
