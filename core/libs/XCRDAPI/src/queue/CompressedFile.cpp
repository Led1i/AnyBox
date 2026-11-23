#include "Queue.hpp"
#include "../metrics/Requests.hpp"

#include <algorithm>
#include <stdexcept>
#include <system_error>

void AnyBox::Storage::Queue::decompressFile(ReadOperation& operation, std::span<std::byte> destination) {
    constexpr std::size_t chunkSize = 1024 * 1024;
    const auto& request = operation.request;
    WorkStatistics statistics(WorkType::Decompression);
    auto staging = GetStagingPool().Acquire(std::min<std::size_t>(request.sourceSize, chunkSize));
    if (!streamDecoder) streamDecoder.emplace();
    streamDecoder->Reset(destination);
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
        try {
            streamDecoder->Process(input, consumed == request.sourceSize);
        } catch (const std::runtime_error& error) {
            throw std::system_error(ERROR_INVALID_DATA, std::system_category(), error.what());
        } catch (const std::length_error& error) {
            throw std::system_error(ERROR_INVALID_DATA, std::system_category(), error.what());
        }
    }
}
