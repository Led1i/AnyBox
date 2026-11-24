#include "Output.hpp"
#include <algorithm>
#include <array>
#include <numeric>
#include <vector>

namespace {
std::vector<DXGI_MODE_DESC1> Enumerate(IDXGIOutput1* output, DXGI_FORMAT format, UINT flags) {
    UINT count = 0;
    auto result = output->GetDisplayModeList1(format, flags, &count, nullptr);
    if (result == DXGI_ERROR_NOT_FOUND) return {};
    AnyBox::Windows::CheckHResult(result, "DXGI display mode count");
    std::vector<DXGI_MODE_DESC1> modes(count);
    if (count != 0) {
        AnyBox::Windows::CheckHResult(output->GetDisplayModeList1(format, flags, &count, modes.data()), "DXGI display mode enumeration");
        modes.resize(count);
    }
    for (auto& mode : modes) {
        auto divisor = std::gcd(mode.RefreshRate.Numerator, mode.RefreshRate.Denominator);
        if (mode.RefreshRate.Denominator == 0) throw std::runtime_error("DXGI display mode refresh denominator is zero");
        mode.RefreshRate.Numerator /= divisor;
        mode.RefreshRate.Denominator /= divisor;
    }
    return modes;
}
}

HRESULT AnyBox::Graphics::DxgiOutput::GetDisplayModeList1(DXGI_FORMAT format, UINT flags, UINT* count, DXGI_MODE_DESC1* modes) {
    static_assert(sizeof(DXGI_MODE_DESC1) == 32);
    if (!count) throw std::invalid_argument("DXGI display mode count output is null");
    constexpr UINT supported = DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING | DXGI_ENUM_MODES_STEREO | DXGI_ENUM_MODES_DISABLED_STEREO;
    if ((flags & ~supported) != 0) throw std::invalid_argument("Unsupported DXGI display enumeration flags");
    std::vector<DXGI_MODE_DESC1> available;
    if (format == DXGI_FORMAT_UNKNOWN) {
        constexpr std::array formats{DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8X8_UNORM, DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT};
        for (auto candidate : formats) {
            auto values = Enumerate(host.get(), candidate, flags);
            available.insert(available.end(), values.begin(), values.end());
        }
    } else {
        available = Enumerate(host.get(), format, flags);
    }

    auto capacity = modes ? *count : 0;
    if (available.size() > std::numeric_limits<UINT>::max()) throw std::overflow_error("DXGI display mode count overflows");
    *count = static_cast<UINT>(available.size());
    if (!modes) return S_OK;
    if (capacity < *count) throw std::length_error("DXGI display mode buffer is too small");
    std::ranges::copy(available, modes);
    return S_OK;
}
