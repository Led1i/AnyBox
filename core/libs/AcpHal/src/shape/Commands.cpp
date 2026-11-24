#include "Runtime.hpp"
#include <cstring>
#include <stdexcept>

void AnyBox::AcpHal::SubmitShapeCommand(std::uint32_t type, const void* data) {
    auto& runtime = GetShapeRuntime();
    std::lock_guard lock(runtime.mutex);
    if (!runtime.allocated) throw std::logic_error("SHAPE contexts do not exist");
    std::uint32_t index;
    std::memcpy(&index, data, sizeof(index));
    if (index >= runtime.decoders.size()) throw std::out_of_range("XMA context index exceeds the allocated context count");
    auto& decoder = runtime.decoders[index];
    if (type == 7) {
        decoder.reset();
        return;
    }
    if (type == 5) {
        if (!decoder) {
            auto* words = static_cast<std::uint32_t*>(runtime.descriptor.contexts[4]) + index * 16;
            auto replacement = std::make_unique<XmaContext>(std::span<std::uint32_t, 16>(words, 16));
            replacement->Process();
            decoder = std::move(replacement);
        } else decoder->Process();
        return;
    }
    if (type != 18) throw std::invalid_argument("Invalid XMA command type");
    if (!decoder) throw std::logic_error("XMA context is not active");
    std::uint32_t consumed;
    std::memcpy(&consumed, static_cast<const std::byte*>(data) + 4, sizeof(consumed));
    decoder->Consume(consumed);
}
