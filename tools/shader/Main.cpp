#include <anybox/shader/Inspection.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string_view>
#include <vector>

int main(int argc, char** argv) {
    try {
        if (argc != 3 || std::string_view(argv[1]) != "--inspect") throw std::invalid_argument("Usage: anybox_shader --inspect <shader.dxbc>");
        std::ifstream input(std::filesystem::path(argv[2]), std::ios::binary | std::ios::ate);
        if (!input) throw std::runtime_error("Cannot open shader input");
        auto size = input.tellg();
        if (size <= 0 || static_cast<std::uint64_t>(size) > std::numeric_limits<std::uint32_t>::max()) throw std::length_error("Shader input size is invalid");
        std::vector<std::byte> data(static_cast<std::size_t>(size));
        input.seekg(0);
        if (!input.read(reinterpret_cast<char*>(data.data()), size)) throw std::runtime_error("Cannot read shader input");
        auto assembly = AnyBox::Shader::InspectDxil(data);
        std::cout << assembly;
        if (!std::cout) throw std::runtime_error("Cannot write shader assembly");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
