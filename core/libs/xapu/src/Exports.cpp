#include <xapu.hpp>
#include <anybox/Unimplemented.hpp>

std::int32_t XApuConnect(const XApuConnectionParameters*, XApuBuffer*, XApuConnection**) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t XApuDequeueResult(XApuConnection*, XApuResult*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t XApuDisconnect(XApuConnection*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t XApuEnqueueCommand(XApuConnection*, const XApuCommand*, std::uint32_t*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::uint32_t XApuGetResultCount(XApuConnection*, std::uint32_t*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}
