#include "TitleHooks.hpp"

#include <xmem.hpp>

#include <cstring>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#error Title allocation hook discovery requires Windows
#endif

namespace {

AnyBox::XMem::TitleHooks LoadTitleHooks() {
    auto module = GetModuleHandleW(nullptr);
    if (!module) throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "GetModuleHandleW");
    auto address = GetProcAddress(module, "__XMemAllocationHooks__");
    if (!address) {
        auto error = GetLastError();
        if (error != ERROR_PROC_NOT_FOUND) throw std::system_error(static_cast<int>(error), std::system_category(), "GetProcAddress");
        return {};
    }
    AnyBox::XMem::TitleHooks hooks{};
    std::memcpy(&hooks, reinterpret_cast<const void*>(address), sizeof(hooks));
    if (!hooks.Allocate || !hooks.Free) throw std::invalid_argument("Title allocation hooks require both allocation and free routines");
    if (hooks.Allocate == XMemAlloc || hooks.Free == XMemFree) throw std::invalid_argument("Title allocation hooks recurse into xmem");
    return hooks;
}

}

const AnyBox::XMem::TitleHooks& AnyBox::XMem::GetTitleHooks() {
    static const TitleHooks hooks = LoadTitleHooks();
    return hooks;
}
