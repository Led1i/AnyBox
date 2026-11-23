#include "Tests.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <new>

namespace {

std::uint32_t backingAllocations = 0;
std::uint32_t backingFrees = 0;

void* Allocate(std::size_t size, std::uint64_t attributes) {
    AnyBox::Tests::Require(size == 0x100000 && attributes == 0x12340000, "APU backing allocation arguments changed");
    ++backingAllocations;
    return static_cast<std::byte*>(::operator new(size + 16, std::align_val_t{0x10000})) + 16;
}

void Free(void* memory, std::uint64_t attributes) {
    AnyBox::Tests::Require(attributes == 0x12340000, "APU backing free attributes changed");
    ++backingFrees;
    ::operator delete(static_cast<std::byte*>(memory) - 16, std::align_val_t{0x10000});
}

}

struct TitleHooks {
    decltype(&Allocate) Allocation;
    decltype(&Free) Release;
};

extern "C" {
TitleHooks __XMemAllocationHooks__{Allocate, Free};
}

int main() {
    try {
        using namespace AnyBox::Tests;
        Require(!ApuIsVirtualAddressValid(nullptr, 64), "Null APU address is valid");
        Require(ApuMapVirtualAddress(nullptr) == 0 && ApuMapApuAddress(0) == nullptr, "Null APU translations changed");
        void* output = nullptr;
        ExpectException<std::logic_error>([&] { ApuAlloc(&output, nullptr, 64, 64, 1); });
        ExpectException<std::invalid_argument>([] { ApuCreateHeap(0, 0); });
        Require(backingAllocations == 0, "Rejected APU heap allocated storage");
        Require(ApuCreateHeap(0x100000, 0x12340000) == 0, "APU heap creation failed");
        ExpectException<std::logic_error>([] { ApuCreateHeap(0x100000, 0x12340000); });
        CheckAllocation();
        CheckMapping();
        CheckErrors();
        CheckConcurrency();
        Require(backingAllocations == 1 && backingFrees == 0, "APU heap backing lifetime changed");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
