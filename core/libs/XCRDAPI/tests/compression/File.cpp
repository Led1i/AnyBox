#include "Tests.hpp"

#include <algorithm>
#include <array>

namespace {

class CompressedFile {
public:
    explicit CompressedFile(std::span<const std::byte> data) {
        std::array<wchar_t, MAX_PATH> directory;
        AnyBox::Tests::Require(GetTempPathW(static_cast<DWORD>(directory.size()), directory.data()) != 0, "Compressed file temporary path query failed");
        AnyBox::Tests::Require(GetTempFileNameW(directory.data(), L"abz", 0, path.data()) != 0, "Compressed file creation failed");
        AnyBox::Storage::Handle handle(CreateFileW(path.data(), GENERIC_WRITE, 0, nullptr, TRUNCATE_EXISTING, 0, nullptr));
        if (handle.Get() == INVALID_HANDLE_VALUE) AnyBox::Windows::ThrowLastError("CreateFileW compressed test file");
        std::array<std::byte, 17> prefix{};
        DWORD written = 0;
        AnyBox::Tests::Require(WriteFile(handle.Get(), prefix.data(), static_cast<DWORD>(prefix.size()), &written, nullptr) != 0 && written == prefix.size(), "Compressed file prefix write failed");
        AnyBox::Tests::Require(WriteFile(handle.Get(), data.data(), static_cast<DWORD>(data.size()), &written, nullptr) != 0 && written == data.size(), "Compressed file payload write failed");
    }
    ~CompressedFile() { DeleteFileW(path.data()); }
    const wchar_t* Path() const { return path.data(); }

private:
    std::array<wchar_t, MAX_PATH> path;
};

}

void AnyBox::Tests::CheckCompressedFile() {
    CompressedData data;
    CompressedFile source(data.packed);
    std::vector<std::byte> output(data.plain.size() + 2, std::byte{0x53});
    QueueFixture test(0);
    void* object = nullptr;
    test.factory->OpenFile(source.Path(), Storage::FileId, &object);
    Object<IStorageFile> file(static_cast<IStorageFile*>(object));
    StorageRequest request{};
    request.options = 1;
    request.source = file.get();
    request.sourceOffset = 17;
    request.sourceSize = static_cast<std::uint32_t>(data.packed.size());
    request.destination = output.data() + 1;
    request.destinationSize = static_cast<std::uint32_t>(data.plain.size());
    test.queue->EnqueueRequest(&request);
    test.Wait();
    Require(test.status->GetHResult(0) == 0, "File-sourced zlib decompression failed");
    Require(std::equal(data.plain.begin(), data.plain.end(), output.begin() + 1), "File-sourced zlib data changed");
    Require(output.front() == std::byte{0x53} && output.back() == std::byte{0x53}, "File-sourced zlib output overran its destination");
    test.queue->Close();
}
