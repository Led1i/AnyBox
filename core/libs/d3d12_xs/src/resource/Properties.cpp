#include "Resource.hpp"
#include <anybox/Unimplemented.hpp>

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
    if (data) *data = self->buffer->Map().data();
    return S_OK;
}

void AnyBox::Graphics::Resource::Unmap(Resource* self, std::uint32_t subresource, const D3D12_RANGE* range) {
    if (subresource != 0 || (range && (range->Begin > range->End || range->End > self->description.Width))) throw std::invalid_argument("Placed buffer unmap range is invalid");
}

HRESULT AnyBox::Graphics::Resource::GetHeapProperties(Resource*, D3D12_HEAP_PROPERTIES* properties, D3D12_HEAP_FLAGS* flags) {
    if (!properties && !flags) throw std::invalid_argument("Placed buffer heap property outputs are null");
    ANYBOX_NOT_IMPLEMENTED();
}
