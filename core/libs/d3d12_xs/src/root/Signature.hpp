#pragma once

#include "../object/Child.hpp"

namespace AnyBox::Graphics {
inline constexpr Guid RootSignatureId{0xC54A6B66, 0x72DF, 0x4EE8, {0x8B, 0xE5, 0xA9, 0x46, 0xA1, 0x42, 0x92, 0x14}};
using RootSignature = DeviceChild<ID3D12RootSignature, RootSignatureId>;
static_assert(std::is_standard_layout_v<RootSignature>);
static_assert(RootSignature::GetVtableOffset() == 0);
}
