#include "Heap.hpp"

#include <bit>
#include <stdexcept>

bool AnyBox::AcpHal::Heap::IsValid(const void* address, std::uint32_t alignment) const {
    if (!std::has_single_bit(alignment) || alignment > 0x10000) throw std::invalid_argument("Unsupported APU alignment");
    auto* base = memory.load(std::memory_order_acquire);
    if (!base || !address) return false;
    auto value = reinterpret_cast<std::uintptr_t>(address);
    auto baseValue = reinterpret_cast<std::uintptr_t>(base);
    return value >= baseValue && value - baseValue < memorySize && value % alignment == 0;
}

std::uint32_t AnyBox::AcpHal::Heap::ToApu(const void* address) const {
    if (!address) return 0;
    auto* base = memory.load(std::memory_order_acquire);
    if (!base) throw std::logic_error("APU heap has not been created");
    auto value = reinterpret_cast<std::uintptr_t>(address);
    auto baseValue = reinterpret_cast<std::uintptr_t>(base);
    if (value < baseValue || value - baseValue >= memorySize) throw std::invalid_argument("CPU address is outside the APU heap");
    return busBase + static_cast<std::uint32_t>(value - baseValue);
}

void* AnyBox::AcpHal::Heap::ToVirtual(std::uint32_t address) const {
    if (address == 0) return nullptr;
    auto* base = memory.load(std::memory_order_acquire);
    if (!base) throw std::logic_error("APU heap has not been created");
    if (address < busBase || address - busBase >= memorySize) throw std::invalid_argument("APU address is outside the heap");
    return base + (address - busBase);
}
