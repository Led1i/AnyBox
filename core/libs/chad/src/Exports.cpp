#include <chad.hpp>
#include <anybox/Unimplemented.hpp>

std::int32_t XDspActivate(XDspConnection*, const XDspActivationParameters*, XDspStatus**, XDspStream**) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t XDspConnectWithMaximumStreamLimit(void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, XDspConnection**) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t XDspDeactivate(XDspStream*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t XDspDisconnect(XDspConnection*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t XDspSubmitCommand(XDspStream*, const XDspCommand*, std::uint32_t*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}
