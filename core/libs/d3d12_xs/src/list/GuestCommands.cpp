#include "List.hpp"
#include <stdexcept>

void AnyBox::Graphics::CommandList::resetGuestCommands() noexcept {
    guest.commandWrite = inlineCommands.data();
    guest.dispatchEnd = inlineCommands.data() + inlineCommands.size();
    guest.commandEnd = guest.dispatchEnd;
}

void AnyBox::Graphics::CommandList::validateGuestCommands() const {
    auto begin = reinterpret_cast<std::uintptr_t>(inlineCommands.data());
    auto end = begin + sizeof(inlineCommands);
    auto write = reinterpret_cast<std::uintptr_t>(guest.commandWrite);
    if (write < begin || write > end || (write - begin) % sizeof(std::uint32_t) != 0 || reinterpret_cast<std::uintptr_t>(guest.dispatchEnd) != end || reinterpret_cast<std::uintptr_t>(guest.commandEnd) != end) throw std::logic_error("Xbox command stream bounds are corrupted");
    if (write != begin) throw std::runtime_error("Xbox inline command stream translation is not implemented");
}
