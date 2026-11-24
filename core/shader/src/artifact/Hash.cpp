#include <anybox/shader/Artifact.hpp>
#include <anybox/platform/Windows.hpp>
#include <bcrypt.h>
#include <limits>
#include <stdexcept>

AnyBox::Shader::ShaderDigest AnyBox::Shader::HashShader(std::span<const std::byte> data) {
    if (data.size() > std::numeric_limits<ULONG>::max()) throw std::length_error("Shader digest input exceeds the Windows hash API limit");
    ShaderDigest result;
    auto* input = reinterpret_cast<PUCHAR>(const_cast<std::byte*>(data.data()));
    auto* output = reinterpret_cast<PUCHAR>(result.data());
    auto status = BCryptHash(BCRYPT_SHA256_ALG_HANDLE, nullptr, 0, input, static_cast<ULONG>(data.size()), output, static_cast<ULONG>(result.size()));
    if (status < 0) throw std::runtime_error("Windows SHA-256 computation failed");
    return result;
}

std::string AnyBox::Shader::DigestName(const ShaderDigest& digest) {
    constexpr char alphabet[] = "0123456789abcdef";
    std::string result;
    result.reserve(digest.size() * 2);
    for (auto byte : digest) {
        auto value = std::to_integer<unsigned>(byte);
        result.push_back(alphabet[value >> 4]);
        result.push_back(alphabet[value & 15]);
    }
    return result;
}
