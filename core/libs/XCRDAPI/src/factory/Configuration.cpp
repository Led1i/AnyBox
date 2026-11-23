#include "Factory.hpp"

#include <anybox/platform/WindowsError.hpp>

#include <stdexcept>

std::int32_t AnyBox::Storage::Factory::SetCpuAffinity(std::uint64_t affinity) {
    if (affinity == 0) throw std::invalid_argument("DirectStorage CPU affinity mask is empty");
    DWORD_PTR processMask;
    DWORD_PTR systemMask;
    if (!GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask)) Windows::ThrowLastError("GetProcessAffinityMask DirectStorage");
    if ((affinity & ~processMask) != 0) throw std::invalid_argument("DirectStorage CPU affinity includes unavailable processors");
    cpuAffinity.store(affinity, std::memory_order_release);
    return 0;
}

void AnyBox::Storage::Factory::SetDebugFlags(std::uint32_t flags) {
    if ((flags & ~3U) != 0) throw std::invalid_argument("Unsupported DirectStorage debug flags");
    debugFlags.store(flags, std::memory_order_release);
}

std::int32_t AnyBox::Storage::Factory::SetStagingBufferSize(std::uint32_t size) {
    std::vector<std::byte> replacement(size);
    std::lock_guard lock(mutex);
    stagingBuffer.swap(replacement);
    return 0;
}
