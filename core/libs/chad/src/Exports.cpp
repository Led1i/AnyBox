#include <chad.hpp>
#include <anybox/Unimplemented.hpp>

std::int32_t XDspActivate(XDspConnection*, const XDspActivationParameters*, void**, XDspStream**) noexcept(false) {
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

std::int32_t XDspSubmitCommand(XDspConnection*, const XDspCommand*) noexcept(false) {
    ANYBOX_NOT_IMPLEMENTED();
}
