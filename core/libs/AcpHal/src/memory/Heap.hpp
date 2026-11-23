#ifndef ANYBOX_ACPHAL_MEMORY_HEAP_HPP
#define ANYBOX_ACPHAL_MEMORY_HEAP_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <list>
#include <mutex>

namespace AnyBox::AcpHal {

class Heap {
public:
    void Create(std::uint32_t size, std::uint32_t attributes);
    void* Allocate(std::uint32_t size, std::uint32_t alignment, std::uint32_t flags);
    void Free(void* allocation);
    bool IsValid(const void* address, std::uint32_t alignment) const;
    std::uint32_t ToApu(const void* address) const;
    void* ToVirtual(std::uint32_t address) const;

private:
    struct Block {
        std::size_t offset;
        std::size_t size;
        bool allocated;
    };

    mutable std::mutex mutex;
    std::atomic<std::byte*> memory{nullptr};
    std::uint32_t memorySize = 0;
    std::uint32_t busBase = 0;
    std::list<Block> blocks;
};

Heap& GetHeap();

}

#endif
