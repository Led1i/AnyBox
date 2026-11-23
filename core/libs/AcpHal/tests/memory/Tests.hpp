#ifndef ANYBOX_ACPHAL_MEMORY_TESTS_HPP
#define ANYBOX_ACPHAL_MEMORY_TESTS_HPP

#include <AcpHal.hpp>

#include <stdexcept>

namespace AnyBox::Tests {

inline void Require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

template<class TException, class TCallable>
void ExpectException(TCallable&& callable) {
    try {
        callable();
    } catch (const TException&) {
        return;
    }
    throw std::runtime_error("Expected APU error was not raised");
}

void CheckAllocation();
void CheckConcurrency();
void CheckErrors();
void CheckMapping();

}

#endif
