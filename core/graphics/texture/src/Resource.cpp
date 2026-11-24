#include <anybox/texture/Resource.hpp>
#include <anybox/platform/WindowsError.hpp>
#include <exception>
#include <limits>

AnyBox::Texture::Resource::Resource(AddressSpace& addresses, Vulkan::Context& context, const Vulkan::ImageDescription& value, const Descriptor::Texture2DDescription& description, std::uint64_t size) : addresses(addresses), image(context, value), description(description) {
    Descriptor::EncodeTexture2D(description, {});
    if (value.type != VK_IMAGE_TYPE_2D || value.extent.width != description.width || value.extent.height != description.height || value.arrayLayers != description.layers || value.mipLevels != description.mipLevels || value.samples != VK_SAMPLE_COUNT_1_BIT) throw std::invalid_argument("Texture GPU image does not match its guest description");
    if (size == 0 || size > std::numeric_limits<std::size_t>::max()) throw std::invalid_argument("Texture GPU address reservation size is invalid");
    reservation = VirtualAlloc(nullptr, static_cast<std::size_t>(size), MEM_RESERVE, PAGE_NOACCESS);
    if (!reservation) Windows::ThrowLastError("Reserve texture GPU address range");
    try {
        auto address = GetAddress();
        if (address > 0xFFFFFFFFFFFFULL || size - 1 > 0xFFFFFFFFFFFFULL - address || address % 65536 != 0) throw std::runtime_error("Texture GPU address range cannot be represented");
        addresses.insert(address, *this);
    } catch (...) {
        if (!VirtualFree(reservation, 0, MEM_RELEASE)) std::terminate();
        throw;
    }
}

AnyBox::Texture::Resource::~Resource() {
    addresses.remove(GetAddress());
    if (!VirtualFree(reservation, 0, MEM_RELEASE)) std::terminate();
}
