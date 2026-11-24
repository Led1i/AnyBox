#include "Tests.hpp"
#include <iostream>

int main() {
    try {
        AnyBox::Relinker::Tests::CheckWriteback();
        AnyBox::Relinker::Tests::CheckExecution();
        AnyBox::Relinker::Tests::CheckImages();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
