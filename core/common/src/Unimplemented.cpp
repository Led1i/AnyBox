#include <anybox/Unimplemented.hpp>

#include <stdexcept>
#include <string>

[[noreturn]] void AnyBox::NotImplemented(const char* libraryName, const char* functionName) {
    throw std::runtime_error(std::string(libraryName) + "!" + functionName + " is not implemented");
}
