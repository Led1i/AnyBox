#include "Tests.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

void AnyBox::Tests::CheckAllocation() {
    std::array<void*, 3> allocations{};
    for (std::uint32_t i = 0; i < allocations.size(); ++i) {
        Require(ApuAlloc(&allocations[i], nullptr, 4096, 2048, i % 2 + 1) == 0, "APU allocation failed");
        Require(reinterpret_cast<std::uintptr_t>(allocations[i]) % 2048 == 0, "APU allocation is misaligned");
        std::memset(allocations[i], static_cast<int>(i + 1), 4096);
    }
    for (std::size_t i = 0; i < allocations.size(); ++i) {
        for (std::size_t j = i + 1; j < allocations.size(); ++j) Require(allocations[i] != allocations[j], "APU allocations overlap");
        auto* bytes = static_cast<const std::byte*>(allocations[i]);
        for (std::size_t j = 0; j < 4096; ++j) Require(bytes[j] == std::byte(i + 1), "APU allocation contents changed");
    }
    ApuFree(allocations[1]);
    void* replacement = nullptr;
    std::uint32_t apuAddress = 0;
    Require(ApuAlloc(&replacement, &apuAddress, 4096, 2048, 1) == 0, "APU allocation with bus output failed");
    Require(apuAddress != 0 && ApuMapApuAddress(apuAddress) == replacement, "APU allocation bus output does not map to its CPU output");
    ApuFree(allocations[0]);
    ApuFree(allocations[2]);
    ApuFree(replacement);
    void* entireHeap = nullptr;
    Require(ApuAlloc(&entireHeap, nullptr, 0xFFF00, 256, 1) == 0, "APU free blocks were not coalesced");
    std::memset(entireHeap, 0x5A, 0xFFF00);
    ApuFree(entireHeap);
    ApuFree(nullptr);
}
