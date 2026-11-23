#include "Queue.hpp"
#include "../metrics/Requests.hpp"

#include <algorithm>
#include <stdexcept>
#include <system_error>

void AnyBox::Storage::Queue::decompressFile(ReadOperation& operation, MemoryView& destination) {
    constexpr std::size_t chunkSize = 1024 * 1024;
    const auto& request = operation.request;
    WorkStatistics statistics(WorkType::Decompression);
    auto staging = GetStagingPool().Acquire(std::min<std::size_t>(request.sourceSize, chunkSize));
    startStream(destination);
    std::size_t consumed = 0;
    while (consumed < request.sourceSize) {
        auto count = std::min<std::size_t>(request.sourceSize - consumed, staging.Get().size());
        auto input = staging.Get().first(count);
        try {
            readFile(operation, input, consumed);
        } catch (...) {
            statistics.Cancel();
            throw;
        }
        consumed += count;
        processStream(input, consumed == request.sourceSize);
    }
}
