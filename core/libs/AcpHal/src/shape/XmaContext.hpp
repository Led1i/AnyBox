#pragma once

#include <anybox/codecs/XmaFrameAssembler.hpp>
#include <anybox/codecs/XmaFrameDecoder.hpp>
#include <cstdint>
#include <span>

namespace AnyBox::AcpHal {
class XmaContext {
public:
    explicit XmaContext(std::span<std::uint32_t, 16> words);
    void Process();
    void Consume(std::uint32_t blocks);

private:
    bool readFrame();
    void advancePacket(std::uint32_t nextPacket, std::uint32_t packetCount);
    void writeBlock(std::span<std::byte> output, std::uint32_t writeBlock);
    void validate() const;
    std::uint32_t get(std::size_t word, std::uint32_t shift, std::uint32_t mask) const;
    void set(std::size_t word, std::uint32_t shift, std::uint32_t mask, std::uint32_t value);
    std::uint32_t availableBlocks() const;
    std::span<std::uint32_t, 16> words;
    std::uint32_t channels;
    std::uint32_t rateIndex;
    std::uint32_t framePosition = 512;
    std::uint32_t skippedPackets = 0;
    Codecs::XmaFrameDecoder decoder;
    Codecs::XmaFrameAssembler assembler;
    Codecs::XmaFrame frame{};
};
}
