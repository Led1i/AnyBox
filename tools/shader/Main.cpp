#include <anybox/shader/Inspection.hpp>
#include <anybox/shader/Resources.hpp>
#include <anybox/shader/Compute.hpp>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string_view>
#include <vector>

int main(int argc, char** argv) {
    try {
        if (argc != 3 || (std::string_view(argv[1]) != "--inspect" && std::string_view(argv[1]) != "--bindings" && std::string_view(argv[1]) != "--compute")) throw std::invalid_argument("Usage: anybox_shader <--inspect|--bindings|--compute> <shader.dxbc>");
        std::ifstream input(std::filesystem::path(argv[2]), std::ios::binary | std::ios::ate);
        if (!input) throw std::runtime_error("Cannot open shader input");
        auto size = input.tellg();
        if (size <= 0 || static_cast<std::uint64_t>(size) > std::numeric_limits<std::uint32_t>::max()) throw std::length_error("Shader input size is invalid");
        std::vector<std::byte> data(static_cast<std::size_t>(size));
        input.seekg(0);
        if (!input.read(reinterpret_cast<char*>(data.data()), size)) throw std::runtime_error("Cannot read shader input");
        if (std::string_view(argv[1]) == "--inspect") {
            std::cout << AnyBox::Shader::InspectDxil(data);
        } else if (std::string_view(argv[1]) == "--compute") {
            auto info = AnyBox::Shader::ReadDxilComputeInfo(data);
            std::cout << "entry=" << info.entryPoint << " model=cs_6_" << info.shaderModelMinor << " threads=" << info.threads[0] << ',' << info.threads[1] << ',' << info.threads[2] << " flags=" << info.flags << " wave=" << info.minimumWaveSize << ',' << info.maximumWaveSize << ',' << info.preferredWaveSize << '\n';
        } else {
            constexpr std::array classes{"SRV", "UAV", "CBV", "Sampler"};
            constexpr std::array kinds{"raw", "structured", "constant"};
            for (const auto& binding : AnyBox::Shader::ReadDxilBufferBindings(data)) {
                std::cout << classes.at(static_cast<unsigned>(binding.resourceClass)) << " range=" << binding.rangeId << " space=" << binding.space << " register=" << binding.firstRegister << " count=" << binding.count << " kind=" << kinds.at(static_cast<unsigned>(binding.kind)) << " stride=" << binding.stride << " bytes=" << binding.byteSize << '\n';
            }
        }
        if (!std::cout) throw std::runtime_error("Cannot write shader assembly");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
