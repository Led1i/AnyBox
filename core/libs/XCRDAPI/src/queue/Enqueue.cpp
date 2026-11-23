#include "Queue.hpp"
#include "../file/File.hpp"

#include <limits>
#include <stdexcept>

namespace {

AnyBox::Storage::ReadLease RetainFile(const StorageRequest& request, std::uint64_t sourceType) {
    if (sourceType == 1) return {};
    if (!request.source) throw std::invalid_argument("DirectStorage file source is null");
    auto* file = dynamic_cast<AnyBox::Storage::File*>(static_cast<IStorageFile*>(request.source));
    if (!file) throw std::invalid_argument("DirectStorage file does not belong to this runtime");
    return file->AcquireRead();
}

}

void AnyBox::Storage::Queue::requireSpace() const {
    if (closing) throw std::logic_error("DirectStorage queue is closing");
    if (count + (busy ? 1U : 0U) == operations.size()) throw std::overflow_error("DirectStorage queue is full");
}

void AnyBox::Storage::Queue::publish() {
    ++count;
    if ((description.options & 1) == 0 && count - submitted > operations.size() / 2) submitted = count;
    if (submitted != 0) wake.notify_one();
}

void AnyBox::Storage::Queue::EnqueueRequest(const StorageRequest* request) {
    if (!request) throw std::invalid_argument("DirectStorage request is null");
    if ((request->options & ~((1ULL << 22) | 1)) != 0) throw std::invalid_argument("DirectStorage request transformation is not implemented");
    if (((request->options >> 22) & 1) != description.description.sourceType) throw std::invalid_argument("DirectStorage request source does not match its queue");
    bool compressed = (request->options & 1) != 0;
    if (!compressed && request->sourceSize != request->destinationSize) throw std::invalid_argument("DirectStorage uncompressed request sizes differ");
    if (compressed && (request->sourceSize == 0 || request->destinationSize < request->sourceSize)) throw std::invalid_argument("Invalid DirectStorage compressed request sizes");
    if (request->sourceSize != 0 && (!request->source || !request->destination)) throw std::invalid_argument("DirectStorage request buffer is null");
    if (request->destinationPageOffset != 0 || request->reserved1 != 0) throw std::invalid_argument("Invalid DirectStorage memory destination fields");
    for (auto reserved : request->reserved0) if (reserved != 0) throw std::invalid_argument("DirectStorage request reserved fields must be zero");
    auto maximum = std::numeric_limits<std::uintptr_t>::max();
    if (request->destinationSize > maximum - reinterpret_cast<std::uintptr_t>(request->destination)) throw std::overflow_error("DirectStorage destination range overflows");
    if (description.description.sourceType == 1 && request->sourceSize > maximum - reinterpret_cast<std::uintptr_t>(request->source)) throw std::overflow_error("DirectStorage source range overflows");

    ReadOperation operation{*request, RetainFile(*request, description.description.sourceType)};
    std::lock_guard lock(mutex);
    requireSpace();
    operations[(head + count) % operations.size()].emplace<ReadOperation>(std::move(operation));
    publish();
}

void AnyBox::Storage::Queue::EnqueueStatus(IStorageStatusArray* array, std::uint32_t index) {
    auto* status = dynamic_cast<StatusArray*>(array);
    if (!status) throw std::invalid_argument("DirectStorage status array does not belong to this runtime");
    std::lock_guard lock(mutex);
    requireSpace();
    operations[(head + count) % operations.size()].emplace<StatusOperation>(status, index);
    publish();
}

void AnyBox::Storage::Queue::EnqueueSetEvent(void* event) {
    Handle borrowed(event);
    Handle owned;
    try {
        owned = borrowed.Duplicate();
    } catch (...) {
        borrowed.Release();
        throw;
    }
    borrowed.Release();
    std::lock_guard lock(mutex);
    requireSpace();
    operations[(head + count) % operations.size()].emplace<EventOperation>(EventOperation{std::move(owned)});
    publish();
}

void AnyBox::Storage::Queue::CancelRequestsWithTag(std::uint64_t mask, std::uint64_t value) {
    std::lock_guard lock(mutex);
    if (closing) throw std::logic_error("DirectStorage queue is closing");
    for (std::size_t i = 0; i < count; ++i) {
        auto* read = std::get_if<ReadOperation>(&operations[(head + i) % operations.size()]);
        if (read && (read->request.cancellationTag & mask) == value) read->canceled = true;
    }
}
