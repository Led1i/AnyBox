#include <AcpHal.hpp>
#include <anybox/ExceptionTest.hpp>

int main() {
    return AnyBox::Tests::Run([] {
        IAcpHal* acpHal = nullptr;
        AnyBox::Tests::ExpectUnimplemented("AcpHalCreate", [&] { AcpHalCreate(&acpHal); });
        if (acpHal != nullptr) throw std::runtime_error("An unimplemented export changed its output");
    });
}
