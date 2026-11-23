#include "Tests.hpp"

#include <iostream>

int main() {
    try {
        AnyBox::Tests::CheckQuery();
        AnyBox::Tests::CheckVirtualAllocation();
        AnyBox::Tests::CheckStatistics();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
