#pragma once

#include "../root/Signature.hpp"

namespace AnyBox::Graphics {
inline constexpr Guid PipelineStateId{0x765A30F3, 0xF624, 0x4C6F, {0xA8, 0x28, 0xAC, 0xE9, 0x48, 0x62, 0x24, 0x45}};
const ObjectTable& GetPipelineTable();

class PipelineState : public ObjectLifetime<PipelineState> {
public:
    PipelineState(ID3D12PipelineState* host, Device* device, RootSignature* root);
    ~PipelineState();
    ID3D12PipelineState* GetHost() const noexcept { return host; }
    Device* GetParent() const noexcept { return device; }
    RootSignature* GetRootSignature() const noexcept { return root; }
    static bool Supports(const Guid& interfaceId) noexcept { return interfaceId == PipelineStateId || interfaceId == UnknownId || interfaceId == ObjectId || interfaceId == DeviceChildId || interfaceId == PageableId; }
    static HRESULT GetCachedBlob(PipelineState* self, ID3DBlob** blob);
    static constexpr std::size_t GetVtableOffset() { return offsetof(PipelineState, vtable); }

private:
    friend class ObjectLifetime<PipelineState>;
    std::atomic<std::uint32_t>& getReferences() noexcept { return references; }
    const ObjectTable* vtable;
    std::atomic<std::uint32_t> references{1};
    ID3D12PipelineState* host;
    Device* device;
    RootSignature* root;
};
static_assert(std::is_standard_layout_v<PipelineState>);
static_assert(PipelineState::GetVtableOffset() == 0);
}
