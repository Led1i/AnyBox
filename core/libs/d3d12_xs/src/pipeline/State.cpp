#include "State.hpp"

AnyBox::Graphics::PipelineState::PipelineState(ID3D12PipelineState* host, Device* device, RootSignature* root) : vtable(&GetPipelineTable()), host(host), device(device), root(root) {
    RootSignature::AddRef(root);
    try { Device::AddRef(device); } catch (...) { RootSignature::Release(root); throw; }
}

AnyBox::Graphics::PipelineState::~PipelineState() {
    host->Release();
    RootSignature::Release(root);
    Device::Release(device);
}

HRESULT AnyBox::Graphics::PipelineState::GetCachedBlob(PipelineState*, ID3DBlob** blob) {
    if (!blob) throw std::invalid_argument("Pipeline cache blob output is null");
    *blob = nullptr;
    throw std::runtime_error("Xbox pipeline cache serialization is not implemented");
}

const AnyBox::Graphics::ObjectTable& AnyBox::Graphics::GetPipelineTable() {
    static const auto table = [] {
        auto value = MakeObjectTable<PipelineState>(std::make_index_sequence<std::tuple_size_v<ObjectTable>>{});
        value[8] = MethodAddress(&PipelineState::GetCachedBlob);
        return value;
    }();
    return table;
}
