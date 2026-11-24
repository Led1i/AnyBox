#pragma once

#include "Decoder.hpp"
#include <vector>

namespace AnyBox::Relinker {
struct FunctionRange {
    std::uint32_t begin;
    std::uint32_t end;
    std::uint8_t prologSize;
    std::uint8_t stackSize;
};

struct LoweredFunction {
    std::vector<std::byte> code;
    std::vector<FunctionRange> ranges;
};

LoweredFunction LowerImmediateInsert(const DecodedInstruction& instruction);
}
