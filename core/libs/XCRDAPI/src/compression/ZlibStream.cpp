#include "ZlibStream.hpp"
#include "../metrics/Memory.hpp"

#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>

namespace {

void* Allocate(void*, uInt count, uInt size) noexcept {
    if (size != 0 && count > std::numeric_limits<std::size_t>::max() / size) return nullptr;
    auto bytes = static_cast<std::size_t>(count) * size;
    auto* memory = AnyBox::Storage::AllocateCodecMemory(bytes);
    if (memory) std::memset(memory, 0, bytes);
    return memory;
}

void Free(void*, void* memory) noexcept { AnyBox::Storage::FreeCodecMemory(memory); }

}

AnyBox::Storage::ZlibStream::ZlibStream() {
    stream.zalloc = Allocate;
    stream.zfree = Free;
    auto result = inflateInit(&stream);
    if (result == Z_MEM_ERROR) throw std::bad_alloc();
    if (result != Z_OK) throw std::runtime_error("Zlib stream initialization failed");
}

AnyBox::Storage::ZlibStream::~ZlibStream() {
    if (inflateEnd(&stream) != Z_OK) std::terminate();
}

void AnyBox::Storage::ZlibStream::Reset(std::span<std::byte> destination) {
    if (destination.empty() || destination.size() > std::numeric_limits<uInt>::max()) throw std::length_error("Invalid zlib stream destination size");
    if (inflateReset(&stream) != Z_OK) throw std::runtime_error("Zlib stream reset failed");
    stream.next_out = reinterpret_cast<Bytef*>(destination.data());
    stream.avail_out = static_cast<uInt>(destination.size());
    expectedSize = destination.size();
    overflowActive = false;
    finished = false;
}

void AnyBox::Storage::ZlibStream::Process(std::span<const std::byte> input, bool finalChunk) {
    if (finished || expectedSize == 0) throw std::logic_error("Zlib stream is not accepting input");
    if (input.size() > std::numeric_limits<uInt>::max()) throw std::length_error("Zlib stream chunk is too large");
    stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input.data()));
    stream.avail_in = static_cast<uInt>(input.size());

    for (;;) {
        if (stream.avail_out == 0) {
            stream.next_out = &overflow;
            stream.avail_out = 1;
            overflowActive = true;
        }
        auto inputBefore = stream.avail_in;
        auto outputBefore = stream.avail_out;
        auto result = inflate(&stream, Z_NO_FLUSH);
        if (overflowActive && stream.avail_out == 0) throw std::length_error("Zlib output exceeds its destination size");
        if (result == Z_MEM_ERROR) throw std::bad_alloc();
        if (result == Z_STREAM_END) {
            if (stream.avail_in != 0 || !finalChunk) throw std::runtime_error("Zlib source contains data after the compressed stream");
            if (stream.total_out != expectedSize) throw std::length_error("Zlib output size does not match its request");
            finished = true;
            return;
        }
        if (result != Z_OK && result != Z_BUF_ERROR) throw std::runtime_error("Invalid RFC 1950 zlib stream");
        if (inputBefore == stream.avail_in && outputBefore == stream.avail_out) break;
        if (stream.avail_in == 0 && stream.avail_out != 0) break;
    }
    if (stream.avail_in != 0) throw std::runtime_error("Zlib stream did not consume its input chunk");
    if (finalChunk) throw std::runtime_error("Truncated RFC 1950 zlib stream");
}
