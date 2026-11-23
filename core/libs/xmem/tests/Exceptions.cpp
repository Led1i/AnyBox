#include <xmem.hpp>
#include <anybox/ExceptionTest.hpp>

int main() {
    return AnyBox::Tests::Run([] {
        std::size_t pageCount = 1;
        std::uint64_t pageFrame = 0x123456789ABCDEF0;
        void* address = nullptr;
        AnyBox::Tests::ExpectUnimplemented("XMemAllocatePhysicalPages", [&] { XMemAllocatePhysicalPages(0, &pageCount, &pageFrame); });
        AnyBox::Tests::ExpectUnimplemented("XMemFreePhysicalPages", [&] { XMemFreePhysicalPages(1, &pageFrame); });
        if (XMemIsToolsMemAvailable() != 0) throw std::runtime_error("Development-kit tools memory is not available");
        AnyBox::Tests::ExpectUnimplemented("XMemMapPhysicalPages", [&] { XMemMapPhysicalPages(nullptr, 1, &pageFrame); });
        AnyBox::Tests::ExpectUnimplemented("XMemMapPhysicalPagesScatter", [&] { XMemMapPhysicalPagesScatter(&address, 1, &pageFrame); });
        AnyBox::Tests::ExpectUnimplemented("XMemTransferMemory", [&] { XMemTransferMemory(1, 4096); });
        if (pageCount != 1 || pageFrame != 0x123456789ABCDEF0 || address != nullptr) throw std::runtime_error("An unimplemented export changed its output");
    });
}
