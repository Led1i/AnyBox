#ifndef ANYBOX_STORAGE_ZLIB_STREAM_HPP
#define ANYBOX_STORAGE_ZLIB_STREAM_HPP

#include <zlib.h>

#include <cstddef>
#include <span>

namespace AnyBox::Storage {

class ZlibStream {
public:
    ZlibStream();
    ZlibStream(const ZlibStream&) = delete;
    ZlibStream& operator=(const ZlibStream&) = delete;
    ~ZlibStream();
    void Reset(std::span<std::byte> destination);
    void Process(std::span<const std::byte> input, bool finalChunk);

private:
    z_stream stream{};
    std::size_t expectedSize = 0;
    unsigned char overflow = 0;
    bool overflowActive = false;
    bool finished = false;
};

}

#endif
