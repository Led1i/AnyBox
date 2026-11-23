#include "../memory/Tests.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <new>

int main() {
    try {
        using namespace AnyBox::Tests;
        ApuCreateHeap(0x100000, 0x226E00);
        AcpHalShapeContexts descriptor{};
        Require(AcpHalAllocateShapeContexts(&descriptor) == 0, "Empty SHAPE descriptor was rejected");
        for (auto* context : descriptor.contexts) Require(context == nullptr, "Empty SHAPE array allocated storage");
        AcpHalReleaseShapeContexts();
        descriptor.counts[4] = 317;
        Require(AcpHalAllocateShapeContexts(&descriptor) == 0, "SHAPE context allocation failed");
        auto* first = descriptor.contexts[4];
        Require(first != nullptr && ApuIsVirtualAddressValid(first, 64), "SHAPE context storage is not APU-visible");
        auto* bytes = static_cast<std::byte*>(first);
        for (std::size_t i = 0; i < 317 * 64; ++i) Require(bytes[i] == std::byte{0}, "SHAPE context was not initialized");
        std::memset(first, 0x73, 317 * 64);
        descriptor.counts[4] = 19;
        Require(AcpHalAllocateShapeContexts(&descriptor) == 0, "SHAPE context replacement failed");
        Require(descriptor.contexts[4] != first, "SHAPE replacement reused live storage");
        ExpectException<std::invalid_argument>([&] { ApuFree(first); });
        auto saved = descriptor.contexts;
        descriptor.counts[4] = 0x100000 / 64 + 1;
        ExpectException<std::bad_alloc>([&] { AcpHalAllocateShapeContexts(&descriptor); });
        Require(descriptor.contexts == saved, "Failed SHAPE allocation changed its output");
        auto* retained = static_cast<std::byte*>(saved[4]);
        retained[0] = std::byte{0x24};
        Require(ApuMapApuAddress(ApuMapVirtualAddress(retained)) == retained, "Failed SHAPE replacement released live storage");
        descriptor.counts[4] = std::numeric_limits<std::uint32_t>::max();
        ExpectException<std::overflow_error>([&] { AcpHalAllocateShapeContexts(&descriptor); });
        descriptor.reserved = 1;
        ExpectException<std::invalid_argument>([&] { AcpHalAllocateShapeContexts(&descriptor); });
        ExpectException<std::invalid_argument>([] { AcpHalAllocateShapeContexts(nullptr); });
        AcpHalReleaseShapeContexts();
        AcpHalReleaseShapeContexts();
        void* entireHeap = nullptr;
        Require(ApuAlloc(&entireHeap, nullptr, 0x100000, 1, 0) == 0, "SHAPE release leaked APU storage");
        ApuFree(entireHeap);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
