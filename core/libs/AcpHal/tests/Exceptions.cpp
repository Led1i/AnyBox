#include <AcpHal.hpp>
#include <anybox/ExceptionTest.hpp>

int main() {
    return AnyBox::Tests::Run([] {
        IAcpHal* acpHal = nullptr;
        void* allocation = nullptr;
        AnyBox::Tests::ExpectUnimplemented("AcpHalAllocateShapeContexts", [] { AcpHalAllocateShapeContexts(nullptr); });
        AnyBox::Tests::ExpectUnimplemented("AcpHalCreate", [&] { AcpHalCreate(&acpHal); });
        AnyBox::Tests::ExpectUnimplemented("AcpHalReleaseShapeContexts", [] { AcpHalReleaseShapeContexts(); });
        AnyBox::Tests::ExpectUnimplemented("ApuAlloc", [&] { ApuAlloc(&allocation, nullptr, 4096, 2048, 2); });
        AnyBox::Tests::ExpectUnimplemented("ApuCreateHeap", [] { ApuCreateHeap(4096, 0); });
        AnyBox::Tests::ExpectUnimplemented("ApuFree", [] { ApuFree(nullptr); });
        AnyBox::Tests::ExpectUnimplemented("ApuIsVirtualAddressValid", [] { ApuIsVirtualAddressValid(nullptr, 64); });
        AnyBox::Tests::ExpectUnimplemented("ApuMapApuAddress", [] { ApuMapApuAddress(0); });
        AnyBox::Tests::ExpectUnimplemented("ApuMapVirtualAddress", [] { ApuMapVirtualAddress(nullptr); });
        if (acpHal != nullptr || allocation != nullptr) throw std::runtime_error("An unimplemented export changed its output");
    });
}
