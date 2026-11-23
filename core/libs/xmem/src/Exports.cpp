#include <xmem.hpp>
#include <anybox/Unimplemented.hpp>

std::int32_t XMemAllocatePhysicalPages(std::uint32_t, std::size_t*, std::uint64_t*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

void XMemFreePhysicalPages(std::size_t, const std::uint64_t*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

void* XMemMapPhysicalPages(void*, std::size_t, const std::uint64_t*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t XMemMapPhysicalPagesScatter(void* const*, std::size_t, const std::uint64_t*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t XMemTransferMemory(std::uint32_t, std::size_t) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}
