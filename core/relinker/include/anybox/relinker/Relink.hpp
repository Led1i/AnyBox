#pragma once

#include "Image.hpp"

namespace AnyBox::Relinker {
struct RelinkResult {
    std::uint64_t instructionCount = 0;
    std::vector<std::uint32_t> writebackAddresses;
};
RelinkResult LowerCacheWriteback(Image& image);
}
