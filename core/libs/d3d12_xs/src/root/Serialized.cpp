#include "Serialized.hpp"
#include <anybox/graphics/container/Dxbc.hpp>
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace {
std::uint32_t Read(std::span<const std::byte> data, std::size_t offset) {
    std::uint32_t value = 0;
    for (std::size_t index = 0; index < 4; ++index) value |= std::to_integer<std::uint32_t>(data[offset + index]) << (index * 8);
    return value;
}
}

std::size_t AnyBox::Graphics::LocateSerializedRootFlags(std::span<const std::byte> data) {
    if (data.size() < 32 || std::memcmp(data.data(), "DXBC", 4) != 0) throw std::invalid_argument("Root signature container header is invalid");
    if (Read(data, 20) != 1 || Read(data, 24) != data.size()) throw std::invalid_argument("Root signature container version or length is invalid");
    auto count = Read(data, 28);
    if (count == 0 || count > (data.size() - 32) / 4) throw std::invalid_argument("Root signature container part table is invalid");
    auto checksum = Container::ComputeDxbcChecksum(data.subspan(20));
    if (!std::ranges::equal(checksum, data.subspan(4, 16))) throw std::invalid_argument("Root signature container checksum is invalid");
    std::size_t result = 0;
    auto tableEnd = 32 + static_cast<std::size_t>(count) * 4;
    for (std::uint32_t index = 0; index < count; ++index) {
        auto offset = static_cast<std::size_t>(Read(data, 32 + static_cast<std::size_t>(index) * 4));
        if (offset < tableEnd || offset % 4 != 0 || offset > data.size() - 8) throw std::invalid_argument("Root signature container part offset is invalid");
        auto size = Read(data, offset + 4);
        if (size > data.size() - offset - 8) throw std::invalid_argument("Root signature container part length is invalid");
        for (std::uint32_t previous = 0; previous < index; ++previous) {
            auto other = static_cast<std::size_t>(Read(data, 32 + static_cast<std::size_t>(previous) * 4));
            auto otherEnd = other + 8 + Read(data, other + 4);
            if (offset < otherEnd && other < offset + 8 + size) throw std::invalid_argument("Root signature container parts overlap");
        }
        if (Read(data, offset) != 0x30535452) continue;
        if (result != 0 || size < 24) throw std::invalid_argument("Root signature container has an invalid root signature part");
        auto version = Read(data, offset + 8);
        if (version != 1 && version != 2) throw std::invalid_argument("Unsupported Xbox root signature part version");
        result = offset + 28;
    }
    if (result == 0) throw std::invalid_argument("Root signature container has no root signature part");
    return result;
}

std::uint32_t AnyBox::Graphics::ReadSerializedRootFlags(std::span<const std::byte> data) {
    return Read(data, LocateSerializedRootFlags(data));
}
