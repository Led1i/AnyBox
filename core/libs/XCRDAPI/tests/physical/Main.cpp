#include "Tests.hpp"

#include <iostream>

int main() {
    try {
        AnyBox::Tests::CheckRequestPadding();
        AnyBox::Tests::CheckPhysicalCopies();
        AnyBox::Tests::CheckPhysicalMemoryDecode();
        AnyBox::Tests::CheckPhysicalFileRead();
        AnyBox::Tests::CheckPhysicalOverlaps();
        AnyBox::Tests::CheckPhysicalLifetime();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
