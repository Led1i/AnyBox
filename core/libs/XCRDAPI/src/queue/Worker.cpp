#include "Queue.hpp"

#include <bit>
#include <system_error>

void AnyBox::Storage::Queue::run() {
    std::int32_t groupResult = 0;
    std::unique_lock lock(mutex);
    for (;;) {
        wake.wait(lock, [this] { return submitted != 0 || closing; });
        if (submitted == 0 && closing) {
            closed = true;
            drained.notify_all();
            return;
        }
        auto operation = std::move(operations[head]);
        operations[head].emplace<std::monostate>();
        head = (head + 1) % operations.size();
        --count;
        --submitted;
        spaceAvailable.notify_one();
        lock.unlock();
        try {
            if (auto* request = std::get_if<ReadOperation>(&operation)) read(*request);
            else if (auto* status = std::get_if<StatusOperation>(&operation)) {
                status->Complete(groupResult);
                groupResult = 0;
            } else if (auto* event = std::get_if<EventOperation>(&operation)) {
                if (!SetEvent(event->event.Get())) Windows::ThrowLastError("SetEvent DirectStorage queue");
            }
        } catch (const std::system_error& error) {
            if (groupResult == 0) groupResult = std::bit_cast<std::int32_t>(0x80070000U | (static_cast<std::uint32_t>(error.code().value()) & 0xFFFF));
            if (!SetEvent(errorEvent.Get())) std::terminate();
        } catch (...) {
            if (groupResult == 0) groupResult = std::bit_cast<std::int32_t>(std::uint32_t{0x80004005});
            if (!SetEvent(errorEvent.Get())) std::terminate();
        }
        operation.emplace<std::monostate>();
        lock.lock();
    }
}
