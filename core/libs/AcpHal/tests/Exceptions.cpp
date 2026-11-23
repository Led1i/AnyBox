#include <AcpHal.hpp>
#include <anybox/ExceptionTest.hpp>

int main() {
    return AnyBox::Tests::Run([] {
        IAcpHal* acpHal = nullptr;
        AnyBox::Tests::ExpectUnimplemented("AcpHalAllocateShapeContexts", [] { AcpHalAllocateShapeContexts(nullptr); });
        AnyBox::Tests::ExpectUnimplemented("AcpHalCreate", [&] { AcpHalCreate(&acpHal); });
        AnyBox::Tests::ExpectUnimplemented("AcpHalReleaseShapeContexts", [] { AcpHalReleaseShapeContexts(); });
        if (acpHal != nullptr) throw std::runtime_error("An unimplemented export changed its output");
    });
}
