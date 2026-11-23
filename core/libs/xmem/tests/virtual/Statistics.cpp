#include "Tests.hpp"

#include <algorithm>
#include <cstdint>

void AnyBox::Tests::CheckStatistics() {
    struct Guarded {
        std::uint64_t before = 0x1A2B3C4D;
        XMemWorkingSetStatistics statistics{};
        std::uint64_t after = 0x4D3C2B1A;
    } output;
    auto& statistics = output.statistics;
    Require(XMemGetWorkingSetStatistics(0x20000001, &statistics) != 0, "XMem statistics failed");
    MEMORYSTATUSEX native{};
    native.dwLength = sizeof(native);
    Require(GlobalMemoryStatusEx(&native) != 0, "Statistics test memory query failed");
    Require(statistics.gameLimit == std::min(native.ullTotalPhys, native.ullTotalVirtual), "XMem memory budget is fabricated");
    Require(statistics.gameUsed > 0 && statistics.gameUsedPeak >= statistics.gameUsed, "XMem private commit statistics are invalid");
    Require(statistics.toolsLimit == 0 && statistics.toolsUsed == 0, "XMem reported unavailable tools memory");
    auto before = statistics.gameUsed;
    {
        auto region = AllocateRegion(0x1000000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        XMemGetWorkingSetStatistics(0x2000000A, &statistics);
        Require(statistics.gameUsed >= before + 0x1000000, "XMem did not observe committed host memory");
        Require(statistics.gameUsedPeak >= statistics.gameUsed, "XMem peak commit is below current commit");
    }
    auto during = statistics.gameUsed;
    XMemGetWorkingSetStatistics(0x20000001, &statistics);
    Require(statistics.gameUsed < during, "XMem statistics did not observe released host memory");
    ExpectException<std::invalid_argument>([] { XMemGetWorkingSetStatistics(0, nullptr); });
    Require(output.before == 0x1A2B3C4D && output.after == 0x4D3C2B1A, "XMem statistics overran its output");
}
