#ifndef ANYBOX_ACPHAL_TYPES_HPP
#define ANYBOX_ACPHAL_TYPES_HPP

#include <array>
#include <cstddef>
#include <cstdint>

struct IAcpHal;

struct AcpHalShapeContexts {
    std::array<std::uint32_t, 5> counts;
    std::uint32_t reserved;
    std::array<void*, 5> contexts;
};

static_assert(offsetof(AcpHalShapeContexts, contexts) == 24);
static_assert(sizeof(AcpHalShapeContexts) == 64);

#endif
