#include "Buffers.hpp"

namespace {
AnyBox::Graphics::Tests::NativeResource CreateResource(ID3D12Device* device, D3D12_HEAP_TYPE type, D3D12_RESOURCE_STATES state) {
    using namespace AnyBox::Graphics::Tests;
    constexpr GUID interfaceId{0x696442BE, 0xA72E, 0x4059, {0xBC, 0x79, 0x5B, 0x5C, 0x98, 0x04, 0x0F, 0xAD}};
    D3D12_HEAP_PROPERTIES properties{type, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1};
    D3D12_RESOURCE_DESC description{};
    description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    description.Width = TransferSize;
    description.Height = 1;
    description.DepthOrArraySize = 1;
    description.MipLevels = 1;
    description.SampleDesc.Count = 1;
    description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    void* output = nullptr;
    Require(device->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE, &description, state, nullptr, interfaceId, &output) == 0 && output, "Native GPU synchronization test resource creation failed");
    return NativeResource(static_cast<ID3D12Resource*>(output));
}

std::byte Pattern(std::size_t index) { return static_cast<std::byte>((index * 37) ^ (index >> 8)); }
}

AnyBox::Graphics::Tests::NativeBuffers AnyBox::Graphics::Tests::CreateNativeBuffers(Device* device) {
    NativeBuffers buffers{
        CreateResource(device->GetHost(), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ),
        CreateResource(device->GetHost(), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON),
        CreateResource(device->GetHost(), D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST)
    };
    void* data = nullptr;
    D3D12_RANGE range{0, 0};
    Require(buffers.upload->Map(0, &range, &data) == 0 && data, "GPU synchronization upload mapping failed");
    auto* bytes = static_cast<std::byte*>(data);
    for (std::size_t index = 0; index < TransferSize; ++index) bytes[index] = Pattern(index);
    range.End = TransferSize;
    buffers.upload->Unmap(0, &range);
    return buffers;
}

void AnyBox::Graphics::Tests::VerifyReadback(ID3D12Resource* resource) {
    void* data = nullptr;
    D3D12_RANGE range{0, TransferSize};
    Require(resource->Map(0, &range, &data) == 0 && data, "GPU synchronization readback mapping failed");
    auto* bytes = static_cast<const std::byte*>(data);
    bool valid = true;
    for (std::size_t index = 0; index < TransferSize; ++index) valid = valid && bytes[index] == Pattern(index);
    range.End = 0;
    resource->Unmap(0, &range);
    Require(valid, "GPU synchronization did not preserve transferred bytes");
}
