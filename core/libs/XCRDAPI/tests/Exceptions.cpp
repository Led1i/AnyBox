#include <XCRDAPI.hpp>
#include <anybox/ExceptionTest.hpp>

int main() {
    return AnyBox::Tests::Run([] {
        AnyBox::Guid interfaceId{};
        void* factory = nullptr;
        AnyBox::Tests::ExpectUnimplemented("DStorageGetFactory", [&] { DStorageGetFactory(interfaceId, &factory); });
        if (factory != nullptr) throw std::runtime_error("DStorageGetFactory changed its output");
    });
}
