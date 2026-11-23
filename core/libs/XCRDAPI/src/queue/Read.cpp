#include "Queue.hpp"

#include <cstring>
#include <limits>
#include <stdexcept>
#include <system_error>

void AnyBox::Storage::Queue::read(ReadOperation& operation) {
    if (operation.canceled) return;
    const auto& request = operation.request;
    if (request.sourceSize == 0) return;
    if (description.description.sourceType == 1) {
        std::memmove(request.destination, request.source, request.sourceSize);
        return;
    }
    if (request.sourceOffset > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) - request.sourceSize) {
        throw std::overflow_error("DirectStorage file range overflows");
    }
    OVERLAPPED overlapped{};
    overlapped.Offset = static_cast<DWORD>(request.sourceOffset);
    overlapped.OffsetHigh = static_cast<DWORD>(request.sourceOffset >> 32);
    overlapped.hEvent = readEvent.Get();
    if (!ResetEvent(readEvent.Get())) Windows::ThrowLastError("ResetEvent DirectStorage read");
    DWORD transferred = 0;
    auto result = ReadFile(operation.file.Get(), request.destination, request.sourceSize, &transferred, &overlapped);
    if (!result && GetLastError() != ERROR_IO_PENDING) Windows::ThrowLastError("ReadFile DirectStorage");
    if (!GetOverlappedResult(operation.file.Get(), &overlapped, &transferred, TRUE)) Windows::ThrowLastError("GetOverlappedResult DirectStorage");
    if (transferred != request.destinationSize) throw std::system_error(ERROR_HANDLE_EOF, std::system_category(), "Short DirectStorage read");
}
