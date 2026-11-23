#ifndef ANYBOX_XMEM_TITLE_HOOKS_HPP
#define ANYBOX_XMEM_TITLE_HOOKS_HPP

#include <cstddef>
#include <cstdint>

namespace AnyBox::XMem {

struct TitleHooks {
    void* (*Allocate)(std::size_t size, std::uint64_t attributes);
    void (*Free)(void* allocation, std::uint64_t attributes);
};

const TitleHooks& GetTitleHooks();

}

#endif
