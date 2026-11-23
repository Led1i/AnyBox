#include "Tests.hpp"

#include <cstddef>
#include <cstring>
#include <iostream>

int main() {
    try {
        using AnyBox::Tests::Require;
        Require(ApuCreateHeap(0x100000, 0x226E00) == 0, "APU heap could not use default XMem storage");
        void* allocation = nullptr;
        Require(ApuAlloc(&allocation, nullptr, 65536, 2048, 2) == 0, "APU allocation failed with default XMem storage");
        Require(ApuIsVirtualAddressValid(allocation, 2048), "APU default heap allocation is invalid");
        std::memset(allocation, 0x6D, 65536);
        auto* bytes = static_cast<std::byte*>(allocation);
        auto address = ApuMapVirtualAddress(allocation);
        for (std::uint32_t i = 0; i < 65536; ++i) {
            Require(ApuMapApuAddress(address + i) == bytes + i, "APU default heap translation failed");
            Require(bytes[i] == std::byte{0x6D}, "APU default heap contents changed");
        }
        ApuFree(allocation);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
