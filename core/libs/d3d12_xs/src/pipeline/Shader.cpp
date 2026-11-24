#include "Shader.hpp"
#include <anybox/graphics/container/Dxil.hpp>
#include <string>

void AnyBox::Graphics::ValidateComputeShader(Device& device, std::span<const std::byte> code) {
    Container::DxbcView container(code);
    auto part = container.FindPart(0x4C495844);
    auto modern = container.FindPart(0x58454853);
    auto legacy = container.FindPart(0x52444853);
    auto programs = static_cast<unsigned>(part.has_value()) + static_cast<unsigned>(modern.has_value()) + static_cast<unsigned>(legacy.has_value());
    if (programs != 1) throw std::invalid_argument("Compute shader container must contain one executable shader part");
    if (!part) return;
    Container::DxilView program(part->data);
    if ((program.GetProgramVersion() >> 16) != 5) throw std::invalid_argument("Compute pipeline shader has a different shader stage");
    if (!program.HasEmbeddedData()) return;
    auto options = program.FindEmbeddedPart(10);
    if (!options) throw std::runtime_error("Xbox compute shader wave size is unspecified");
    Container::CompilerOptionsView values(options->data);
    bool waveSpecified = false;
    bool sourceSpecified = false;

    for (std::uint32_t index = 0; index < values.GetCount(); ++index) {
        auto option = values.GetOption(index);
        if (option.key != 627 && option.key != 677) throw std::runtime_error("Xbox shader compiler option is not implemented: " + std::to_string(option.key));
        if (option.value != "0" && option.value != "1") throw std::invalid_argument("Xbox shader boolean compiler option is invalid");
        if (option.key == 677) {
            if (sourceSpecified) throw std::invalid_argument("Xbox shader inline source option is duplicated");
            sourceSpecified = true;
            continue;
        }
        if (waveSpecified) throw std::invalid_argument("Xbox shader wave size option is duplicated");
        waveSpecified = true;
        auto required = option.value == "1" ? 32U : 64U;
        auto limits = device.GetWaveLimits();
        if (limits[0] != required || limits[1] != required) throw std::runtime_error("Xbox shader wave size requires explicit subgroup control on this adapter");
    }
    if (!waveSpecified) throw std::runtime_error("Xbox compute shader wave size is unspecified");
}
