#include "Tests.hpp"

#include <array>
#include <cstddef>
#include <limits>
#include <new>

void AnyBox::Tests::CheckErrors() {
    std::array<std::byte, 64> foreign{};
    Require(!ApuIsVirtualAddressValid(foreign.data(), 1), "Foreign storage was accepted as APU memory");
    ExpectException<std::invalid_argument>([&] { ApuMapVirtualAddress(foreign.data()); });
    ExpectException<std::invalid_argument>([] { ApuMapApuAddress(1); });
    ExpectException<std::invalid_argument>([] { ApuMapApuAddress(std::numeric_limits<std::uint32_t>::max()); });
    ExpectException<std::invalid_argument>([] { ApuAlloc(nullptr, nullptr, 64, 64, 0); });
    void* output = foreign.data();
    ExpectException<std::invalid_argument>([&] { ApuAlloc(&output, nullptr, 0, 64, 0); });
    ExpectException<std::invalid_argument>([&] { ApuAlloc(&output, nullptr, 64, 3, 0); });
    ExpectException<std::invalid_argument>([&] { ApuAlloc(&output, nullptr, 64, 0x20000, 0); });
    ExpectException<std::invalid_argument>([&] { ApuAlloc(&output, nullptr, 64, 64, 3); });
    ExpectException<std::bad_alloc>([&] { ApuAlloc(&output, nullptr, 0x100001, 1, 0); });
    Require(output == foreign.data(), "Failed APU allocation modified its output");
    void* allocation = nullptr;
    ApuAlloc(&allocation, nullptr, 64, 64, 0);
    ExpectException<std::invalid_argument>([&] { ApuFree(static_cast<std::byte*>(allocation) + 1); });
    ApuFree(allocation);
    ExpectException<std::invalid_argument>([&] { ApuFree(allocation); });
}
