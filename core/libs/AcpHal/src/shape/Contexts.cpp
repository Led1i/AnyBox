#include <AcpHal.hpp>
#include <anybox/Unimplemented.hpp>

#include <array>
#include <cstring>
#include <limits>
#include <mutex>
#include <stdexcept>

namespace {

std::mutex contextMutex;
std::array<void*, 5> ownedContexts{};

}

std::int32_t AcpHalAllocateShapeContexts(AcpHalShapeContexts* shapeContexts) noexcept(false) {
    if (!shapeContexts) throw std::invalid_argument("SHAPE context descriptor is null");
    if (shapeContexts->reserved != 0) throw std::invalid_argument("SHAPE reserved field must be zero");
    for (std::size_t i = 0; i < 4; ++i) {
        if (shapeContexts->counts[i] != 0) ANYBOX_NOT_IMPLEMENTED();
    }
    auto count = shapeContexts->counts[4];
    if (count > std::numeric_limits<std::uint32_t>::max() / 64) throw std::overflow_error("SHAPE context array size overflows");
    std::lock_guard lock(contextMutex);
    std::array<void*, 5> replacement{};
    if (count != 0) {
        ApuAlloc(&replacement[4], nullptr, count * 64, 64, 2);
        std::memset(replacement[4], 0, static_cast<std::size_t>(count) * 64);
    }
    try {
        for (auto*& allocation : ownedContexts) {
            ApuFree(allocation);
            allocation = nullptr;
        }
    } catch (...) {
        ApuFree(replacement[4]);
        throw;
    }
    ownedContexts = replacement;
    shapeContexts->contexts = replacement;
    return 0;
}

void AcpHalReleaseShapeContexts() noexcept(false) {
    std::lock_guard lock(contextMutex);
    for (auto*& allocation : ownedContexts) {
        ApuFree(allocation);
        allocation = nullptr;
    }
}
