#ifndef ANYBOX_XG_XS_HPP
#define ANYBOX_XG_XS_HPP

#include <xg_xsTypes.hpp>

#include <cstdint>

extern "C" {

std::int32_t XGCreateTextureComputer(const XGTextureDescription* description, IXGTextureComputer** computer) noexcept(false);
std::int32_t XGSuggestSwizzleMode(const XGTextureDescription* description, std::uint32_t flags, std::uint32_t* swizzleMode) noexcept(false);

}

#endif
