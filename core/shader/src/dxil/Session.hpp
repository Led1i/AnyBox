#pragma once

#include <dxil_spirv_c.h>
#include <stdexcept>

namespace AnyBox::Shader {
class DxilSession {
public:
    DxilSession() {
        if (active) throw std::logic_error("DXIL compiler session cannot be reentered on the same thread");
        unsigned major, minor, patch;
        dxil_spv_get_version(&major, &minor, &patch);
        if (major != DXIL_SPV_API_VERSION_MAJOR || minor != DXIL_SPV_API_VERSION_MINOR || patch != DXIL_SPV_API_VERSION_PATCH) throw std::runtime_error("DXIL compiler library version does not match its headers");
        dxil_spv_begin_thread_allocator_context();
        active = true;
    }
    ~DxilSession() {
        dxil_spv_end_thread_allocator_context();
        active = false;
    }
    DxilSession(const DxilSession&) = delete;
    DxilSession& operator=(const DxilSession&) = delete;

private:
    inline static thread_local bool active = false;
};
}
