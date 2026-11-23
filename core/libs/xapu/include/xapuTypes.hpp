#ifndef ANYBOX_XAPU_TYPES_HPP
#define ANYBOX_XAPU_TYPES_HPP

#include <cstdint>

struct XApuConnectionParameters {
    std::uint32_t processor;
    std::uint32_t streamCount;
    std::uint32_t commandsPerStream;
    std::uint32_t bufferSize;
    std::uint32_t frameCount;
    std::uint32_t flags;
};

static_assert(sizeof(XApuConnectionParameters) == 24);

struct XApuBuffer;
struct XApuConnection;
struct XApuCommand;
struct XApuResult;

#endif
