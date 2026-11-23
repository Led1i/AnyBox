#include <chad.hpp>
#include <anybox/ExceptionTest.hpp>

int main() {
    return AnyBox::Tests::Run([] {
        XDspConnection* connection = nullptr;
        XDspStream* stream = nullptr;
        XDspStatus* status = nullptr;
        AnyBox::Tests::ExpectUnimplemented("XDspActivate", [&] { XDspActivate(nullptr, nullptr, &status, &stream); });
        AnyBox::Tests::ExpectUnimplemented("XDspConnectWithMaximumStreamLimit", [&] { XDspConnectWithMaximumStreamLimit(nullptr, 0, 0, 128, 1, &connection); });
        AnyBox::Tests::ExpectUnimplemented("XDspDeactivate", [&] { XDspDeactivate(nullptr); });
        AnyBox::Tests::ExpectUnimplemented("XDspDisconnect", [&] { XDspDisconnect(nullptr); });
        AnyBox::Tests::ExpectUnimplemented("XDspSubmitCommand", [&] { XDspSubmitCommand(nullptr, nullptr, nullptr); });
        if (connection != nullptr || stream != nullptr || status != nullptr) throw std::runtime_error("An unimplemented export changed its output");
    });
}
