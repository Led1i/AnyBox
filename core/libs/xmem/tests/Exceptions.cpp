#include <xmem.hpp>
#include <anybox/ExceptionTest.hpp>

int main() {
    return AnyBox::Tests::Run([] {
        if (XMemIsToolsMemAvailable() != 0) throw std::runtime_error("Development-kit tools memory is not available");
        AnyBox::Tests::ExpectUnimplemented("XMemTransferMemory", [&] { XMemTransferMemory(1, 4096); });
    });
}
