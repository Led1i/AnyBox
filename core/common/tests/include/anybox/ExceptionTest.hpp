#ifndef ANYBOX_EXCEPTION_TEST_HPP
#define ANYBOX_EXCEPTION_TEST_HPP

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace AnyBox::Tests {

class UnwindProbe {
public:
    explicit UnwindProbe(int& destructionCount) : destructionCount(destructionCount) {}
    ~UnwindProbe() { ++destructionCount; }

private:
    int& destructionCount;
};

template<class TCallable>
void Invoke(TCallable&& callable, int& destructionCount) {
    UnwindProbe probe(destructionCount);
    std::forward<TCallable>(callable)();
}

template<class TCallable>
void ExpectUnimplemented(const char* functionName, TCallable&& callable) {
    int destructionCount = 0;
    bool caught = false;
    try {
        Invoke(std::forward<TCallable>(callable), destructionCount);
    } catch (const std::runtime_error& error) {
        caught = true;
        if (error.what() != std::string(ANYBOX_LIBRARY_NAME) + "!" + functionName + " is not implemented") {
            throw std::runtime_error(std::string(functionName) + " reported the wrong exception");
        }
    }
    if (!caught) throw std::runtime_error(std::string(functionName) + " returned without throwing");
    if (destructionCount != 1) throw std::runtime_error(std::string(functionName) + " did not unwind exactly once");
}

template<class TCallable>
int Run(TCallable&& callable) {
    try {
        std::forward<TCallable>(callable)();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

}

#endif
