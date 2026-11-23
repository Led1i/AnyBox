#include "Heap.hpp"

#include <xmem.hpp>

#include <limits>
#include <new>
#include <stdexcept>

void AnyBox::AcpHal::Heap::Create(std::uint32_t size, std::uint32_t attributes) {
    if (size == 0) throw std::invalid_argument("APU heap size must be nonzero");
    if (size > 0xFFFF0000) throw std::length_error("APU heap exceeds its 32-bit address space");
    std::lock_guard lock(mutex);
    if (memory.load(std::memory_order_relaxed)) throw std::logic_error("APU heap already exists");
    std::list<Block> initialBlocks{{0, size, false}};
    auto* allocation = static_cast<std::byte*>(XMemAlloc(size, attributes));
    if (!allocation) throw std::bad_alloc();
    auto address = reinterpret_cast<std::uintptr_t>(allocation);
    if (size > std::numeric_limits<std::uintptr_t>::max() - address) {
        XMemFree(allocation, attributes);
        throw std::overflow_error("APU heap CPU address range overflows");
    }
    auto busAddress = 0x10000 + static_cast<std::uint32_t>(address & 0xFFFF);
    if (static_cast<std::uint64_t>(busAddress) + size > 0x100000000) {
        XMemFree(allocation, attributes);
        throw std::length_error("APU heap bus address range overflows");
    }
    busBase = busAddress;
    memorySize = size;
    blocks.splice(blocks.end(), initialBlocks);
    memory.store(allocation, std::memory_order_release);
}

AnyBox::AcpHal::Heap& AnyBox::AcpHal::GetHeap() {
    static Heap heap;
    return heap;
}
