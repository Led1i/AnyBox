#include <AcpHal.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

class UnwindProbe {
public:
    explicit UnwindProbe(bool& destroyed) : destroyed(destroyed) {}
    ~UnwindProbe() { destroyed = true; }

private:
    bool& destroyed;
};

template<class TCallable>
void Invoke(TCallable&& callable, bool& destroyed) {
    UnwindProbe probe(destroyed);
    std::forward<TCallable>(callable)();
}

template<class TCallable>
void ExpectUnimplemented(const char* functionName, TCallable&& callable) {
    bool destroyed = false;
    bool caught = false;
    try {
        Invoke(std::forward<TCallable>(callable), destroyed);
    } catch (const std::runtime_error& error) {
        caught = true;
        if (error.what() != std::string("AcpHal.dll!") + functionName + " is not implemented") {
            throw std::runtime_error(std::string(functionName) + " reported the wrong exception");
        }
    }
    if (!caught) throw std::runtime_error(std::string(functionName) + " returned without throwing");
    if (!destroyed) throw std::runtime_error(std::string(functionName) + " skipped stack cleanup");
}

}

int main() {
    try {
        IAcpHal* acpHal = nullptr;
        void* allocation = nullptr;
        ExpectUnimplemented("AcpHalAllocateShapeContexts", [] { AcpHalAllocateShapeContexts(nullptr); });
        ExpectUnimplemented("AcpHalCreate", [&] { AcpHalCreate(&acpHal); });
        ExpectUnimplemented("AcpHalReleaseShapeContexts", [] { AcpHalReleaseShapeContexts(); });
        ExpectUnimplemented("ApuAlloc", [&] { ApuAlloc(&allocation, nullptr, 4096, 2048, 2); });
        ExpectUnimplemented("ApuCreateHeap", [] { ApuCreateHeap(4096, 0); });
        ExpectUnimplemented("ApuFree", [] { ApuFree(nullptr); });
        ExpectUnimplemented("ApuIsVirtualAddressValid", [] { ApuIsVirtualAddressValid(nullptr, 64); });
        ExpectUnimplemented("ApuMapApuAddress", [] { ApuMapApuAddress(0); });
        ExpectUnimplemented("ApuMapVirtualAddress", [] { ApuMapVirtualAddress(nullptr); });
        if (acpHal != nullptr || allocation != nullptr) throw std::runtime_error("An unimplemented export changed its output");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
