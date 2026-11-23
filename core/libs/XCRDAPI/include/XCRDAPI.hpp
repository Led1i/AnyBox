#ifndef ANYBOX_XCRDAPI_HPP
#define ANYBOX_XCRDAPI_HPP

#include <anybox/Types.hpp>

#include <cstdint>

extern "C" {

std::int32_t DStorageGetFactory(const AnyBox::Guid& interfaceId, void** factory) noexcept(false);

}

#endif
