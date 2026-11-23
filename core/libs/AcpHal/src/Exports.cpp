#include <AcpHal.hpp>
#include <anybox/Unimplemented.hpp>

std::int32_t AcpHalAllocateShapeContexts(AcpHalShapeContexts*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t AcpHalCreate(IAcpHal**) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

void AcpHalReleaseShapeContexts() noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t ApuAlloc(void**, void*, std::uint32_t, std::uint32_t, std::uint32_t) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t ApuCreateHeap(std::uint32_t, std::uint32_t) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

void ApuFree(void*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

bool ApuIsVirtualAddressValid(const void*, std::uint32_t) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

void* ApuMapApuAddress(std::uint32_t) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::uint32_t ApuMapVirtualAddress(const void*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}
