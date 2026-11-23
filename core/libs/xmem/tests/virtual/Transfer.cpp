#include "Tests.hpp"

#include <system_error>

void AnyBox::Tests::CheckTransfer() {
    Require(XMemIsToolsMemAvailable() == 0, "Host incorrectly reports development-kit memory");
    for (auto direction : {0U, 1U}) {
        Require(XMemTransferMemory(direction, 0) != 0, "Zero-sized tools memory transfer failed");
        for (std::size_t size : {std::size_t{1}, std::size_t{0x200000}, std::size_t{0x1000000}}) {
            bool caught = false;
            try {
                XMemTransferMemory(direction, size);
            } catch (const std::system_error& error) {
                caught = error.code().category() == std::system_category() && error.code().value() == ERROR_NO_SYSTEM_RESOURCES;
            }
            Require(caught, "Tools memory transfer fabricated an available resource");
        }
    }
    ExpectException<std::invalid_argument>([] { XMemTransferMemory(2, 0); });
    ExpectException<std::invalid_argument>([] { XMemTransferMemory(0xFFFFFFFF, 1); });
}
