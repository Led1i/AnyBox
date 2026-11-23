#include <xapu.hpp>
#include <anybox/ExceptionTest.hpp>

int main() {
    return AnyBox::Tests::Run([] {
        XApuConnection* connection = nullptr;
        AnyBox::Tests::ExpectUnimplemented("XApuConnect", [&] { XApuConnect(nullptr, nullptr, &connection); });
        AnyBox::Tests::ExpectUnimplemented("XApuDequeueResult", [&] { XApuDequeueResult(nullptr, nullptr); });
        AnyBox::Tests::ExpectUnimplemented("XApuDisconnect", [&] { XApuDisconnect(nullptr); });
        AnyBox::Tests::ExpectUnimplemented("XApuEnqueueCommand", [&] { XApuEnqueueCommand(nullptr, nullptr, 0); });
        AnyBox::Tests::ExpectUnimplemented("XApuGetResultCount", [&] { XApuGetResultCount(nullptr, 0); });
        if (connection != nullptr) throw std::runtime_error("An unimplemented export changed its output");
    });
}
