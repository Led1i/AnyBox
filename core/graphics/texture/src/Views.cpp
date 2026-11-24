#include <anybox/texture/Resource.hpp>
#include <array>

VkImageView AnyBox::Texture::Resource::GetView(const Descriptor::ImageWords& words) {
    std::scoped_lock lock(viewMutex);
    auto found = views.find(words);
    if (found != views.end()) return found->second->GetHandle();
    Descriptor::TextureViewDescription source;
    source.address = (static_cast<std::uint64_t>(words[0]) << 8) | (static_cast<std::uint64_t>(words[1] & 255) << 40);
    source.firstMip = (words[3] >> 12) & 15;
    auto lastMip = (words[3] >> 16) & 15;
    source.firstLayer = words[4] >> 16;
    auto lastLayer = words[4] & 65535;
    if (source.address != GetAddress() || lastMip < source.firstMip || lastLayer < source.firstLayer) throw std::invalid_argument("Texture descriptor address or subresource range is invalid");
    source.mipCount = lastMip - source.firstMip + 1;
    source.layerCount = lastLayer - source.firstLayer + 1;
    source.minimumLod = static_cast<float>((words[1] >> 8) & 4095) / 256;
    if (source.minimumLod != static_cast<float>(source.firstMip)) throw std::runtime_error("Texture view minimum LOD clamping is not implemented");
    constexpr std::array<std::uint32_t, 8> selectors{4, 5, UINT32_MAX, UINT32_MAX, 0, 1, 2, 3};
    constexpr std::array<VkComponentSwizzle, 6> swizzles{VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ONE};
    std::array<VkComponentSwizzle, 4> components{};
    source.componentMapping = 0x1000;
    for (unsigned component = 0; component < 4; ++component) {
        auto selector = selectors[(words[3] >> (component * 3)) & 7];
        if (selector == UINT32_MAX) throw std::invalid_argument("Texture descriptor component selector is invalid");
        source.componentMapping |= selector << (component * 3);
        components[component] = swizzles[selector];
    }
    if (Descriptor::EncodeTexture2D(description, source) != words) throw std::runtime_error("Texture descriptor fields do not match their registered resource");

    Vulkan::ImageViewDescription view;
    view.type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    view.firstMip = source.firstMip;
    view.mipCount = source.mipCount;
    view.firstLayer = source.firstLayer;
    view.layerCount = source.layerCount;
    view.components = {components[0], components[1], components[2], components[3]};
    auto created = std::make_unique<Vulkan::ImageView>(image, view);
    auto handle = created->GetHandle();
    views.emplace(words, std::move(created));
    return handle;
}
