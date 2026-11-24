#include <anybox/relinker/Relink.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

int main(int argc, char** argv) {
    try {
        if (argc != 4 || std::string_view(argv[1]) != "--clflush") throw std::invalid_argument("Usage: anybox_relink --clflush <input.exe> <output.exe>");
        std::filesystem::path inputPath(argv[2]), outputPath(argv[3]);
        if (std::filesystem::exists(outputPath) && std::filesystem::equivalent(inputPath, outputPath)) throw std::invalid_argument("Relinker output must differ from its input");
        std::ifstream input(inputPath, std::ios::binary | std::ios::ate);
        if (!input) throw std::runtime_error("Cannot open relinker input");
        auto length = input.tellg();
        if (length < 0) throw std::runtime_error("Cannot determine relinker input size");
        std::vector<std::byte> bytes(static_cast<std::size_t>(length));
        input.seekg(0);
        if (!input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()))) throw std::runtime_error("Cannot read relinker input");
        AnyBox::Relinker::Image image(std::move(bytes));
        auto result = AnyBox::Relinker::LowerCacheWriteback(image);
        std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
        if (!output) throw std::runtime_error("Cannot open relinker output");
        auto data = image.GetBytes();
        output.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        output.close();
        if (!output) throw std::runtime_error("Cannot write relinker output");
        std::cout << "Decoded " << result.instructionCount << " instructions; lowered " << result.writebackAddresses.size() << " CLWB instructions\n";
        for (auto address : result.writebackAddresses) std::cout << "CLFLUSH RVA 0x" << std::hex << address << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
