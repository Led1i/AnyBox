#include <d3d12_xs.hpp>
#include <anybox/ExceptionTest.hpp>

int main() {
    return AnyBox::Tests::Run([] {
        AnyBox::Guid interfaceId{};
        void* device = nullptr;
        void* deserializer = nullptr;
        ID3DBlob* blob = nullptr;
        ID3DBlob* errorBlob = nullptr;
        AnyBox::Tests::ExpectUnimplemented("D3D12XboxCreateDevice", [&] { D3D12XboxCreateDevice(nullptr, nullptr, interfaceId, &device); });
        if (device != nullptr || deserializer != nullptr || blob != nullptr || errorBlob != nullptr) throw std::runtime_error("An unimplemented export changed its output");
    });
}
