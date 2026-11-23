#include "Queue.hpp"

#include <cstring>
#include <stdexcept>
#include <system_error>

void AnyBox::Storage::Queue::read(ReadOperation& operation) {
    if (operation.canceled) return;
    const auto& request = operation.request;
    if (request.sourceSize == 0) return;
    auto destination = std::span(static_cast<std::byte*>(request.destination), request.destinationSize);
    bool compressed = (request.options & 1) != 0;

    if (description.description.sourceType == 0) {
        if (!compressed) {
            readFile(operation, destination);
            return;
        }
        auto staging = GetStagingPool().Acquire(request.sourceSize);
        readFile(operation, staging.Get());
        decompress(staging.Get(), destination);
        return;
    }
    if (!compressed) {
        std::memmove(destination.data(), request.source, request.sourceSize);
        return;
    }

    auto source = std::span(static_cast<const std::byte*>(request.source), request.sourceSize);
    auto sourceAddress = reinterpret_cast<std::uintptr_t>(source.data());
    auto destinationAddress = reinterpret_cast<std::uintptr_t>(destination.data());
    bool overlaps = sourceAddress < destinationAddress + destination.size() && destinationAddress < sourceAddress + source.size();
    if (overlaps) {
        if (source.size() > overlapCapacity) {
            auto replacement = std::make_unique_for_overwrite<std::byte[]>(source.size());
            overlapBuffer = std::move(replacement);
            overlapCapacity = source.size();
        }
        std::memcpy(overlapBuffer.get(), source.data(), source.size());
        source = {overlapBuffer.get(), source.size()};
    }
    decompress(source, destination);
}

void AnyBox::Storage::Queue::decompress(std::span<const std::byte> source, std::span<std::byte> destination) {
    if (!decoder) decoder.emplace();
    try {
        decoder->Decode(source, destination);
    } catch (const std::runtime_error& error) {
        throw std::system_error(ERROR_INVALID_DATA, std::system_category(), error.what());
    }
}
