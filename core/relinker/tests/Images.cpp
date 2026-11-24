#include "Tests.hpp"
#include <anybox/relinker/Relink.hpp>
#include <algorithm>
#include <cstring>

namespace {
template<class TValue>
void Put(std::vector<std::byte>& bytes, std::size_t offset, TValue value) { std::memcpy(bytes.data() + offset, &value, sizeof(value)); }

std::vector<std::byte> Fixture(std::span<const std::uint8_t> code) {
    std::vector<std::byte> bytes(1536);
    Put<std::uint16_t>(bytes, 0, 0x5A4D);
    Put<std::uint32_t>(bytes, 0x3C, 0x80);
    Put<std::uint32_t>(bytes, 0x80, 0x4550);
    Put<std::uint16_t>(bytes, 0x84, 0x8664);
    Put<std::uint16_t>(bytes, 0x86, 2);
    Put<std::uint16_t>(bytes, 0x94, 240);
    Put<std::uint16_t>(bytes, 0x98, 0x20B);
    Put<std::uint32_t>(bytes, 0xA8, 0x1000);
    Put<std::uint64_t>(bytes, 0xB0, 0x140000000ULL);
    Put<std::uint32_t>(bytes, 0xD4, 512);
    Put<std::uint32_t>(bytes, 0x104, 16);
    Put<std::uint32_t>(bytes, 0x120, 0x2000);
    Put<std::uint32_t>(bytes, 0x124, 12);
    Put<std::uint32_t>(bytes, 0x190, 64);
    Put<std::uint32_t>(bytes, 0x194, 0x1000);
    Put<std::uint32_t>(bytes, 0x198, 512);
    Put<std::uint32_t>(bytes, 0x19C, 512);
    Put<std::uint32_t>(bytes, 0x1AC, 0x60000020);
    Put<std::uint32_t>(bytes, 0x1B8, 64);
    Put<std::uint32_t>(bytes, 0x1BC, 0x2000);
    Put<std::uint32_t>(bytes, 0x1C0, 512);
    Put<std::uint32_t>(bytes, 0x1C4, 1024);
    Put<std::uint32_t>(bytes, 0x1D4, 0x40000040);
    Put<std::uint32_t>(bytes, 1024, 0x1000);
    Put<std::uint32_t>(bytes, 1028, 0x1000 + static_cast<std::uint32_t>(code.size()));
    Put<std::uint32_t>(bytes, 1032, 0x2030);
    std::memcpy(bytes.data() + 512, code.data(), code.size());
    return bytes;
}

template<class TCallable>
void Reject(TCallable&& callable) {
    try { callable(); }
    catch (const std::runtime_error&) { return; }
    throw std::runtime_error("Malformed or ambiguous PE input was accepted");
}
}

void AnyBox::Relinker::Tests::CheckImages() {
    const std::array<std::uint8_t, 9> code{0x49, 0x89, 0xC9, 0x66, 0x41, 0x0F, 0xAE, 0x31, 0xC3};
    auto original = Fixture(code);
    Image image(original);
    auto result = LowerCacheWriteback(image);
    Require(result.writebackAddresses == std::vector<std::uint32_t>{0x1003}, "PE pass did not find the cache instruction at its actual boundary");
    Require(image.GetBytes().size() == original.size(), "PE lowering moved file data");
    Require(std::ranges::equal(image.GetBytes().subspan(1024), std::span(original).subspan(1024)), "PE lowering changed its unwind data");
    for (std::size_t index = 0; index < original.size(); ++index) {
        if (index != 515 && index != 519) Require(image.GetBytes()[index] == original[index], "PE lowering changed bytes outside the cache instruction");
    }
    Require(image.GetBytes()[515] == std::byte{0x40} && image.GetBytes()[519] == std::byte{0x39}, "PE cache replacement encoding changed");
    Require(LowerCacheWriteback(image).writebackAddresses.empty(), "PE cache lowering is not idempotent");

    auto signedBytes = original;
    signedBytes.resize(1568, std::byte{0xA7});
    Put<std::uint32_t>(signedBytes, 0xD8, 0x12345678);
    Put<std::uint32_t>(signedBytes, 0x128, 1536);
    Put<std::uint32_t>(signedBytes, 0x12C, 32);
    Image signedImage(signedBytes);
    LowerCacheWriteback(signedImage);
    Require(Read<std::uint32_t>(signedImage.GetBytes(), 0xD8) == 0 && Read<std::uint64_t>(signedImage.GetBytes(), 0x128) == 0, "PE lowering retained an invalid checksum or certificate directory");
    Require(std::ranges::equal(signedImage.GetBytes().subspan(1024), std::span(signedBytes).subspan(1024)), "PE header finalization changed section data or its overlay");

    const std::array<std::uint8_t, 11> immediate{0x48, 0xB8, 0x66, 0x41, 0x0F, 0xAE, 0x31, 0, 0, 0, 0xC3};
    auto data = Fixture(immediate);
    Image literal(data);
    Require(LowerCacheWriteback(literal).writebackAddresses.empty() && std::ranges::equal(literal.GetBytes(), data), "PE pass mistook immediate data for a cache instruction");

    auto corrupt = original;
    Put<std::uint32_t>(corrupt, 0x19C, 0xFFFFFFF0);
    Reject([&] { Image invalid(corrupt); });
    corrupt = original;
    Put<std::uint32_t>(corrupt, 0x124, 13);
    Reject([&] { Image invalid(corrupt); LowerCacheWriteback(invalid); });
    corrupt = original;
    corrupt[512] = std::byte{0xC3};
    Reject([&] { Image ambiguous(corrupt); LowerCacheWriteback(ambiguous); });
}
