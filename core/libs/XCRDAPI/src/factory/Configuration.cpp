#include "Factory.hpp"
#include "../workers/Configuration.hpp"

#include <anybox/platform/WindowsError.hpp>

#include <stdexcept>

std::int32_t AnyBox::Storage::Factory::SetCpuAffinity(std::uint64_t affinity) {
    GetWorkerConfiguration().SetAffinity(affinity);
    return 0;
}

void AnyBox::Storage::Factory::SetDebugFlags(std::uint32_t flags) {
    if ((flags & ~3U) != 0) throw std::invalid_argument("Unsupported DirectStorage debug flags");
    debugFlags.store(flags, std::memory_order_release);
}

std::int32_t AnyBox::Storage::Factory::SetStagingBufferSize(std::uint32_t size) {
    stagingPool.SetSize(size);
    return 0;
}
