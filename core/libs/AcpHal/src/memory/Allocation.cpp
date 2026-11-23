#include "Heap.hpp"

#include <bit>
#include <iterator>
#include <new>
#include <algorithm>
#include <limits>
#include <stdexcept>

void* AnyBox::AcpHal::Heap::Allocate(std::uint32_t size, std::uint32_t alignment, std::uint32_t flags) {
    if (size == 0) throw std::invalid_argument("APU allocation size must be nonzero");
    if (!std::has_single_bit(alignment) || alignment < 4 || alignment > 0x10000) throw std::invalid_argument("Unsupported APU alignment");
    if (flags < 1 || flags > 2) throw std::invalid_argument("Unsupported APU allocation flags");
    alignment = std::max(alignment, 256U);
    if (size > std::numeric_limits<std::uint32_t>::max() - (alignment - 1)) throw std::overflow_error("APU allocation size overflows");
    size = (size + alignment - 1) & ~(alignment - 1);
    std::lock_guard lock(mutex);
    auto* base = memory.load(std::memory_order_relaxed);
    if (!base) throw std::logic_error("APU heap has not been created");
    auto baseValue = reinterpret_cast<std::uintptr_t>(base);
    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        if (it->allocated) continue;
        auto padding = (alignment - (baseValue + it->offset) % alignment) % alignment;
        if (padding > it->size || size > it->size - padding) continue;
        auto start = it->offset + padding;
        auto remaining = it->size - padding - size;
        std::list<Block> prefix;
        std::list<Block> suffix;
        if (padding) prefix.push_back({it->offset, padding, false});
        if (remaining) suffix.push_back({start + size, remaining, false});
        blocks.splice(it, prefix);
        blocks.splice(std::next(it), suffix);
        *it = {start, size, true};
        return base + start;
    }
    throw std::bad_alloc();
}

void AnyBox::AcpHal::Heap::Free(void* allocation) {
    if (!allocation) return;
    std::lock_guard lock(mutex);
    auto* base = memory.load(std::memory_order_relaxed);
    if (!base) throw std::logic_error("APU heap has not been created");
    auto address = reinterpret_cast<std::uintptr_t>(allocation);
    auto baseValue = reinterpret_cast<std::uintptr_t>(base);
    if (address < baseValue || address - baseValue >= memorySize) throw std::invalid_argument("APU allocation is outside the heap");
    auto offset = address - baseValue;
    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        if (it->offset != offset || !it->allocated) continue;
        it->allocated = false;
        if (it != blocks.begin()) {
            auto previous = std::prev(it);
            if (!previous->allocated) {
                previous->size += it->size;
                blocks.erase(it);
                it = previous;
            }
        }
        auto next = std::next(it);
        if (next != blocks.end() && !next->allocated) {
            it->size += next->size;
            blocks.erase(next);
        }
        return;
    }
    throw std::invalid_argument("APU address is not the start of a live allocation");
}
