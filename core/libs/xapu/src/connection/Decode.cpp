#include "Connection.hpp"

#include <anybox/Unimplemented.hpp>

#include <cmath>
#include <stdexcept>

void AnyBox::XApu::Connection::decode(Stream& stream, const XApuDecodeCommand& command, XApuResult& result) {
    if (!std::isfinite(command.sampleRate) || !std::isfinite(command.ramp)) throw std::invalid_argument("XApu processing parameter is not finite");
    if (command.sampleRate <= 0.0F) throw std::invalid_argument("XApu sample rate must be positive");
    if (command.sampleRate != 48000.0F || command.ramp != 0.0F) ANYBOX_NOT_IMPLEMENTED();

    auto* input = command.input.Get();
    auto* output = command.output.Get();
    if (!contains(input, command.inputSize) || !contains(output, command.outputSize)) throw std::invalid_argument("XApu command buffer is outside connection storage");
    if (reinterpret_cast<std::uintptr_t>(output) % alignof(float) != 0) throw std::invalid_argument("XApu PCM output is misaligned");

    auto packet = std::span(static_cast<const std::byte*>(input), command.inputSize);
    auto frames = stream.decoder->GetFrameCount(packet);
    if (command.firstFrame > frames) throw std::out_of_range("XApu first output frame exceeds the packet duration");
    auto outputFrames = frames - command.firstFrame;
    if (command.frameCount != 0 && command.frameCount < outputFrames) throw std::length_error("XApu frame limit is too small for the packet");
    auto channels = stream.decoder->GetChannelCount();
    auto outputSize = outputFrames * channels * static_cast<std::uint32_t>(sizeof(float));
    if (outputSize > command.outputSize) throw std::length_error("XApu PCM output buffer is too small");

    auto decoded = stream.decoder->Decode(packet, stream.samples);
    if (decoded != frames) throw std::runtime_error("XApu decoded packet duration changed");

    auto* samples = static_cast<float*>(output);
    for (std::uint32_t channel = 0; channel < channels; ++channel) {
        for (std::uint32_t frame = 0; frame < outputFrames; ++frame) {
            samples[channel * outputFrames + frame] = stream.samples[(command.firstFrame + frame) * channels + channel];
        }
    }

    result.state = 3;
    result.frameCount = outputFrames;
    result.outputSize = outputSize;
    result.output.Set(output);
    result.sampleRate = command.sampleRate;
    result.inputSize = command.inputSize;
    result.input.Set(input);
}
