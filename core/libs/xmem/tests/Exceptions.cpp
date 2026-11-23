#include <xmem.hpp>
#include <anybox/ExceptionTest.hpp>

int main() {
    return AnyBox::Tests::Run([] {
        std::size_t pageCount = 1;
        std::uint64_t pageFrame = 0x123456789ABCDEF0;
        void* address = nullptr;
        AnyBox::Tests::ExpectUnimplemented("XMemAlloc", [&] { XMemAlloc(4096, 0); });
        AnyBox::Tests::ExpectUnimplemented("XMemAllocDefault", [&] { XMemAllocDefault(4096, 0, nullptr, nullptr); });
        AnyBox::Tests::ExpectUnimplemented("XMemAllocatePhysicalPages", [&] { XMemAllocatePhysicalPages(0, &pageCount, &pageFrame); });
        AnyBox::Tests::ExpectUnimplemented("XMemFree", [&] { XMemFree(nullptr, 0); });
        AnyBox::Tests::ExpectUnimplemented("XMemFreeDefault", [&] { XMemFreeDefault(nullptr, 0, nullptr, nullptr); });
        AnyBox::Tests::ExpectUnimplemented("XMemFreePhysicalPages", [&] { XMemFreePhysicalPages(1, &pageFrame); });
        AnyBox::Tests::ExpectUnimplemented("XMemGetWorkingSetStatistics", [&] { XMemGetWorkingSetStatistics(0, nullptr); });
        AnyBox::Tests::ExpectUnimplemented("XMemIsToolsMemAvailable", [&] { XMemIsToolsMemAvailable(); });
        AnyBox::Tests::ExpectUnimplemented("XMemMapPhysicalPages", [&] { XMemMapPhysicalPages(nullptr, 1, &pageFrame); });
        AnyBox::Tests::ExpectUnimplemented("XMemMapPhysicalPagesScatter", [&] { XMemMapPhysicalPagesScatter(&address, 1, &pageFrame); });
        AnyBox::Tests::ExpectUnimplemented("XMemTransferMemory", [&] { XMemTransferMemory(1, 4096); });
        AnyBox::Tests::ExpectUnimplemented("XMemVirtualAlloc", [&] { XMemVirtualAlloc(nullptr, 4096, 0, 0, 0); });
        if (pageCount != 1 || pageFrame != 0x123456789ABCDEF0 || address != nullptr) throw std::runtime_error("An unimplemented export changed its output");
    });
}
