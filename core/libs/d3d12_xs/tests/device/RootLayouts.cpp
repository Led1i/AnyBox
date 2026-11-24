#include "Tests.hpp"
#include "../../src/root/Signature.hpp"
#include <d3d12_xs.hpp>
#include <array>
#include <limits>
#include <memory>

namespace {
struct Release {
    void operator()(ID3DBlob* value) const noexcept { value->Release(); }
};

void CheckLayout(AnyBox::Graphics::Device* device, D3D_ROOT_SIGNATURE_VERSION version, std::uint32_t flags) {
    using namespace AnyBox::Graphics;
    using namespace AnyBox::Graphics::Tests;
    std::array<D3D12_ROOT_PARAMETER, 4> parameters{};
    parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    parameters[2].Constants = {0, 0, 16};
    D3D12_DESCRIPTOR_RANGE range{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 2, 1, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND};
    parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    parameters[3].DescriptorTable = {1, &range};
    std::array<D3D12_ROOT_PARAMETER1, 4> parameters1{};
    for (std::size_t index = 0; index < parameters.size(); ++index) parameters1[index].ParameterType = parameters[index].ParameterType;
    parameters1[0].Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC;
    parameters1[1].Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE;
    parameters1[2].Constants = parameters[2].Constants;
    D3D12_DESCRIPTOR_RANGE1 range1{range.RangeType, range.NumDescriptors, range.BaseShaderRegister, range.RegisterSpace, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, range.OffsetInDescriptorsFromTableStart};
    parameters1[3].DescriptorTable = {1, &range1};
    D3D12_STATIC_SAMPLER_DESC sampler{};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MaxLOD = std::numeric_limits<float>::max();
    sampler.ShaderRegister = 2;
    sampler.RegisterSpace = 4;
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC description{};
    description.Version = version;
    auto nativeFlags = static_cast<D3D12_ROOT_SIGNATURE_FLAGS>(flags);
    if (version == D3D_ROOT_SIGNATURE_VERSION_1_0) description.Desc_1_0 = {4, parameters.data(), 1, &sampler, nativeFlags};
    else description.Desc_1_1 = {4, parameters1.data(), 1, &sampler, nativeFlags};

    ID3DBlob* serialized = nullptr;
    D3D12SerializeVersionedRootSignature(&description, &serialized, nullptr);
    std::unique_ptr<ID3DBlob, Release> blob(serialized);
    void* output = nullptr;
    auto create = Method<decltype(&Device::CreateRootSignature)>(device, 16);
    Require(create(device, 1, blob->GetBufferPointer(), blob->GetBufferSize(), RootSignatureId, &output) == 0 && output, "Xbox root signature creation failed");
    auto* signature = static_cast<RootSignature*>(output);
    auto release = Method<decltype(&RootSignature::Release)>(signature, 2);
    blob.reset();
    const auto& retained = signature->GetDescription();
    Require(retained.Version == version && signature->GetFlags() == flags, "Xbox root signature version or flags were lost");
    if (version == D3D_ROOT_SIGNATURE_VERSION_1_0) {
        const auto& value = retained.Desc_1_0;
        Require(value.Flags == nativeFlags && value.NumParameters == 4 && value.NumStaticSamplers == 1, "Root signature description changed");
        Require(value.pParameters[2].Constants.Num32BitValues == 16, "Root signature constants were lost");
        Require(value.pParameters[3].DescriptorTable.pDescriptorRanges[0].RegisterSpace == 1, "Root descriptor range was lost");
        Require(value.pStaticSamplers[0].RegisterSpace == 4, "Root static sampler was lost");
    } else {
        const auto& value = retained.Desc_1_1;
        Require(value.Flags == nativeFlags && value.NumParameters == 4 && value.NumStaticSamplers == 1, "Versioned root signature description changed");
        Require(value.pParameters[0].Descriptor.Flags == parameters1[0].Descriptor.Flags, "Root descriptor flags were lost");
        Require(value.pParameters[1].Descriptor.Flags == parameters1[1].Descriptor.Flags, "Volatile root descriptor flags were lost");
        Require(value.pParameters[3].DescriptorTable.pDescriptorRanges[0].Flags == range1.Flags, "Root descriptor range flags were lost");
    }
    if (flags == 0) Require(signature->GetHostForExecution() == signature->GetHost(), "Standard root signature execution interface changed");
    else Reject<std::runtime_error>([&] { signature->GetHostForExecution(); });
    Require(release(signature) == 0, "Xbox root signature retained an unexpected reference");
}
}

void AnyBox::Graphics::Tests::CheckRootLayouts(Device* device) {
    for (auto version : {D3D_ROOT_SIGNATURE_VERSION_1_0, D3D_ROOT_SIGNATURE_VERSION_1_1}) {
        for (auto flags : {0U, 0x01000000U, 0x02000000U, 0x03000000U, 0x80000000U, 0x83000000U}) CheckLayout(device, version, flags);
    }
}
