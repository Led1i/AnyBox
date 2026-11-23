#include "Heap.hpp"

#include <cstring>
#include <new>
#include <stdexcept>

AnyBox::XMem::Heap::~Heap() {
    for (const auto& [address, allocation] : allocations) {
        ::operator delete(address, std::align_val_t{allocation.alignment});
    }
}

void* AnyBox::XMem::Heap::Allocate(std::size_t size, std::uint64_t attributes, std::size_t alignment) {
    auto* allocation = ::operator new(size, std::align_val_t{alignment});
    try {
        std::lock_guard lock(mutex);
        allocations.emplace(allocation, Allocation{alignment, attributes});
    } catch (...) {
        ::operator delete(allocation, std::align_val_t{alignment});
        throw;
    }
    std::memset(allocation, 0, size);
    return allocation;
}

void AnyBox::XMem::Heap::Free(void* allocation, std::uint64_t attributes) {
    if (!allocation) return;
    std::size_t alignment;
    {
        std::lock_guard lock(mutex);
        auto it = allocations.find(allocation);
        if (it == allocations.end()) throw std::invalid_argument("Address is not a live XMem heap allocation");
        if (it->second.attributes != attributes) throw std::invalid_argument("XMem free attributes differ from allocation attributes");
        alignment = it->second.alignment;
        allocations.erase(it);
    }
    ::operator delete(allocation, std::align_val_t{alignment});
}

AnyBox::XMem::Heap& AnyBox::XMem::GetHeap() {
    static Heap heap;
    return heap;
}
