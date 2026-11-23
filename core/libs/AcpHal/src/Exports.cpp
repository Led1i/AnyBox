#include <AcpHal.hpp>
#include <anybox/Unimplemented.hpp>

std::int32_t AcpHalAllocateShapeContexts(AcpHalShapeContexts*) noexcept(false) {
    AnyBox::NotImplemented("AcpHal.dll", __func__);
}

std::int32_t AcpHalCreate(IAcpHal**) noexcept(false) {
    AnyBox::NotImplemented("AcpHal.dll", __func__);
}

void AcpHalReleaseShapeContexts() noexcept(false) {
    AnyBox::NotImplemented("AcpHal.dll", __func__);
}

std::int32_t ApuAlloc(void**, void*, std::uint32_t, std::uint32_t, std::uint32_t) noexcept(false) {
    AnyBox::NotImplemented("AcpHal.dll", __func__);
}

std::int32_t ApuCreateHeap(std::uint32_t, std::uint32_t) noexcept(false) {
    AnyBox::NotImplemented("AcpHal.dll", __func__);
}

void ApuFree(void*) noexcept(false) {
    AnyBox::NotImplemented("AcpHal.dll", __func__);
}

bool ApuIsVirtualAddressValid(const void*, std::uint32_t) noexcept(false) {
    AnyBox::NotImplemented("AcpHal.dll", __func__);
}

void* ApuMapApuAddress(std::uint32_t) noexcept(false) {
    AnyBox::NotImplemented("AcpHal.dll", __func__);
}

std::uint32_t ApuMapVirtualAddress(const void*) noexcept(false) {
    AnyBox::NotImplemented("AcpHal.dll", __func__);
}
