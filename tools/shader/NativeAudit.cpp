#include "NativeAudit.hpp"
#include <anybox/graphics/container/Native.hpp>
#include <anybox/shader/Native.hpp>
#include <sstream>
#include <stdexcept>

namespace {
std::string Decode(std::span<const std::byte> program) {
    AnyBox::Shader::NativeFrontend frontend(program);
    return "instructions=" + std::to_string(frontend.GetDecoded().instructions.size()) + " blocks=" + std::to_string(frontend.GetGraph().blocks.size());
}
}

std::string AnyBox::Shader::Tools::InspectNative(std::span<const std::byte> container) {
    auto programs = Graphics::Container::UnpackNativePrograms(container);
    std::ostringstream output;
    for (std::size_t index = 0; index < programs.size(); ++index) {
        Graphics::Container::FssmView view(programs[index]);
        output << "program=" << index << " wave=" << view.GetWaveSize() << " codeBytes=" << view.GetCode().size() << ' ';
        try { output << Decode(programs[index]); }
        catch (const std::exception& error) { output << "decodeError=" << error.what(); }
        output << '\n';
    }
    return output.str();
}

std::size_t AnyBox::Shader::Tools::AuditNative(std::span<const std::byte> container) {
    auto programs = Graphics::Container::UnpackNativePrograms(container);
    for (std::size_t index = 0; index < programs.size(); ++index) {
        try { (void)Decode(programs[index]); }
        catch (const std::exception& error) { throw std::runtime_error("Native program " + std::to_string(index) + ": " + error.what()); }
    }
    throw std::runtime_error("Xbox native stage parameters frontend is not implemented");
}
