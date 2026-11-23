#ifndef ANYBOX_CHAD_HPP
#define ANYBOX_CHAD_HPP

#include <chadTypes.hpp>

#include <cstdint>

extern "C" {

std::int32_t XDspActivate(XDspConnection* connection, const XDspActivationParameters* parameters, void** sharedMemory, XDspStream** stream) noexcept(false);
std::int32_t XDspConnectWithMaximumStreamLimit(void* memory, std::uint32_t size, std::uint32_t flags, std::uint32_t maximumStreamCount, std::uint32_t mode, XDspConnection** connection) noexcept(false);
std::int32_t XDspDeactivate(XDspStream* stream) noexcept(false);
std::int32_t XDspDisconnect(XDspConnection* connection) noexcept(false);
std::int32_t XDspSubmitCommand(XDspConnection* connection, const XDspCommand* command) noexcept(false);

}

#endif
