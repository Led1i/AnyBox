#include "Tests.hpp"

#include <array>
#include <bit>
#include <cstddef>

namespace {

class TemporaryFile {
public:
    TemporaryFile() {
        std::array<wchar_t, MAX_PATH> directory;
        AnyBox::Tests::Require(GetTempPathW(static_cast<DWORD>(directory.size()), directory.data()) != 0, "Temporary directory query failed");
        AnyBox::Tests::Require(GetTempFileNameW(directory.data(), L"abq", 0, path.data()) != 0, "Temporary file creation failed");
        AnyBox::Storage::Handle handle(CreateFileW(path.data(), GENERIC_WRITE, 0, nullptr, TRUNCATE_EXISTING, 0, nullptr));
        if (handle.Get() == INVALID_HANDLE_VALUE) AnyBox::Windows::ThrowLastError("CreateFileW queue test");
        std::array<std::byte, 4096> data;
        for (std::size_t i = 0; i < data.size(); ++i) data[i] = std::byte(i % 251);
        DWORD written = 0;
        AnyBox::Tests::Require(WriteFile(handle.Get(), data.data(), static_cast<DWORD>(data.size()), &written, nullptr) != 0 && written == data.size(), "Queue test file write failed");
    }
    ~TemporaryFile() { DeleteFileW(path.data()); }
    const wchar_t* Path() const { return path.data(); }

private:
    std::array<wchar_t, MAX_PATH> path;
};

}

void AnyBox::Tests::CheckFileQueue() {
    TemporaryFile source;
    std::array<std::byte, 257> buffer{};
    QueueFixture test(0);
    void* output = nullptr;
    test.factory->OpenFile(source.Path(), Storage::FileId, &output);
    Object<IStorageFile> file(static_cast<IStorageFile*>(output));
    StorageRequest request{};
    request.source = file.get();
    request.sourceOffset = 197;
    request.sourceSize = static_cast<std::uint32_t>(buffer.size());
    request.destination = buffer.data();
    request.destinationSize = request.sourceSize;
    test.queue->EnqueueRequest(&request);
    test.Wait();
    Require(test.status->GetHResult(0) == 0, "Queued file read failed");
    for (std::size_t i = 0; i < buffer.size(); ++i) Require(buffer[i] == std::byte((197 + i) % 251), "Queued file read returned incorrect bytes");
    request.sourceOffset = 4096;
    test.queue->EnqueueRequest(&request);
    test.Wait();
    Require(test.status->GetHResult(0) == std::bit_cast<std::int32_t>(0x80070000U | ERROR_HANDLE_EOF), "Queue did not propagate the short read error");
    Storage::Handle error(test.queue->GetErrorEvent());
    Require(WaitForSingleObject(error.Get(), 0) == WAIT_OBJECT_0, "Queue error event was not set");
    request.sourceOffset = 0;
    test.queue->EnqueueRequest(&request);
    test.Wait();
    Require(test.status->GetHResult(0) == 0, "Queue reused an error from a previous status group");
    test.queue->Close();
}
