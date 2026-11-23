#include "Tests.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

void AnyBox::Tests::CheckDefaultHeap() {
    for (std::uint64_t kind = 0; kind <= 9; ++kind) {
        for (std::uint64_t page = 0; page < 3; ++page) {
            auto attributes = (kind << 29) | (page << 14) | (5ULL << 24) | (37ULL << 16) | 123;
            auto* allocation = XMemAlloc(4097, attributes);
            std::array<std::size_t, 3> pages{4096, 65536, 4194304};
            auto alignment = kind == 0 ? 32 : pages[page];
            Require(allocation != nullptr && reinterpret_cast<std::uintptr_t>(allocation) % alignment == 0, "XMem allocation alignment changed");
            auto* bytes = static_cast<std::byte*>(allocation);
            for (std::size_t i = 0; i < 4097; ++i) Require(bytes[i] == std::byte{0}, "XMem allocation was not initialized");
            std::memset(allocation, static_cast<int>(kind + 1), 4097);
            XMemFreeDefault(allocation, attributes, nullptr, nullptr);
        }
    }
    for (std::uint64_t exponent = 0; exponent <= 20; ++exponent) {
        auto attributes = exponent << 24;
        auto* allocation = XMemAllocDefault(333, attributes, nullptr, nullptr);
        auto alignment = exponent < 4 ? 16 : std::size_t{1} << exponent;
        Require(reinterpret_cast<std::uintptr_t>(allocation) % alignment == 0, "XMem explicit alignment was not honored");
        XMemFree(allocation, attributes);
    }
    auto* first = XMemAllocDefault(0, 0, nullptr, nullptr);
    auto* second = XMemAllocDefault(0, 0, nullptr, nullptr);
    Require(first != nullptr && second != nullptr && first != second, "Zero-sized XMem allocations do not have distinct identities");
    XMemFree(first, 0);
    XMemFree(second, 0);
    XMemFree(nullptr, 0);
    XMemFreeDefault(nullptr, 0, nullptr, nullptr);
}
