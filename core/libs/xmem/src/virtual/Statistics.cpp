#include <xmem.hpp>
#include <anybox/platform/WindowsError.hpp>

#include <psapi.h>

#include <algorithm>
#include <stdexcept>

std::int32_t XMemGetWorkingSetStatistics(std::uint32_t, XMemWorkingSetStatistics* statistics) noexcept(false) {
    if (!statistics) throw std::invalid_argument("XMem working set output is null");
    PROCESS_MEMORY_COUNTERS_EX counters{};
    counters.cb = sizeof(counters);
    if (!GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters), sizeof(counters))) {
        AnyBox::Windows::ThrowLastError("GetProcessMemoryInfo");
    }
    MEMORYSTATUSEX memory{};
    memory.dwLength = sizeof(memory);
    if (!GlobalMemoryStatusEx(&memory)) AnyBox::Windows::ThrowLastError("GlobalMemoryStatusEx");
    XMemWorkingSetStatistics result{};
    result.gameLimit = std::min(memory.ullTotalPhys, memory.ullTotalVirtual);
    result.gameUsed = counters.PrivateUsage;
    result.gameUsedPeak = counters.PeakPagefileUsage;
    *statistics = result;
    return 1;
}
