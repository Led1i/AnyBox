#include <xmem.hpp>
#include <anybox/platform/Windows.hpp>

#include <stdexcept>
#include <system_error>

std::int32_t XMemTransferMemory(std::uint32_t memoryPool, std::size_t size) noexcept(false) {
    if (memoryPool > 1) throw std::invalid_argument("Unsupported XMem tools memory transfer direction");
    if (size == 0) return 1;
    throw std::system_error(ERROR_NO_SYSTEM_RESOURCES, std::system_category(), "XMem tools memory is not available on this host");
}
