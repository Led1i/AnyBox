#include "Descriptions.hpp"
#include "../device/Device.hpp"
#include <anybox/Unimplemented.hpp>
#include <algorithm>

void AnyBox::Graphics::Device::CreateShaderResourceViewX(Device*, const D3D12_RESOURCE_DESC* resource, const XboxShaderViewDescription* description, std::uint64_t destination) {
    if (destination == 0 || destination % 16 != 0) throw std::invalid_argument("Xbox extended shader view destination is null or misaligned");
    auto* output = reinterpret_cast<void*>(static_cast<std::uintptr_t>(destination));
    if (!resource) {
        std::memset(output, 0, 32);
        return;
    }
    if (!description || resource->Dimension != D3D12_RESOURCE_DIMENSION_BUFFER) ANYBOX_NOT_IMPLEMENTED();
    const auto& view = description->view;
    if (view.ViewDimension != D3D12_SRV_DIMENSION_BUFFER || view.Format != DXGI_FORMAT_R32_TYPELESS || view.Buffer.Flags != D3D12_BUFFER_SRV_FLAG_RAW) ANYBOX_NOT_IMPLEMENTED();
    if (view.Buffer.StructureByteStride != 0) throw std::invalid_argument("Xbox raw shader view cannot have a structure stride");
    if (description->address == UINT64_MAX || std::ranges::any_of(description->parameters, [](auto value) { return value != 0; })) ANYBOX_NOT_IMPLEMENTED();
    if (resource->Width > UINT32_MAX || resource->Width % 4 != 0 || view.Buffer.FirstElement > resource->Width / 4) throw std::out_of_range("Xbox raw shader view resource extent or first element is invalid");
    auto offset = view.Buffer.FirstElement * 4;
    auto count = view.Buffer.NumElements == UINT32_MAX ? resource->Width - offset : static_cast<std::uint64_t>(view.Buffer.NumElements) * 4;
    if (count > UINT32_MAX || count > resource->Width - offset) throw std::out_of_range("Xbox raw shader view exceeds its resource extent");
    if (description->address > 0xFFFFFFFFFFFFULL || offset > 0xFFFFFFFFFFFFULL - description->address || description->address % 4 != 0) throw std::invalid_argument("Xbox raw shader view address cannot be encoded");
    auto address = description->address + offset;

    constexpr std::array<std::uint32_t, 6> selectors{4, 5, 6, 7, 0, 1};
    std::uint32_t swizzle = 0;
    for (unsigned component = 0; component < 4; ++component) {
        auto selector = (view.Shader4ComponentMapping >> (component * 3)) & 7;
        if (selector >= selectors.size()) throw std::invalid_argument("Xbox shader view component selector is invalid");
        swizzle |= selectors[selector] << (component * 3);
    }
    const std::array<std::uint32_t, 8> words{static_cast<std::uint32_t>(address), static_cast<std::uint32_t>(address >> 32), static_cast<std::uint32_t>(count), 0x30014000 | swizzle, 0, 0, 0, 0};
    std::memcpy(output, words.data(), sizeof(words));
}
