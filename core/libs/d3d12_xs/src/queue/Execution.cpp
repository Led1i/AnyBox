#include "Queue.hpp"
#include "../list/List.hpp"
#include <array>
#include <span>
#include <stdexcept>
#include <vector>

void AnyBox::Graphics::Queue::ExecuteCommandLists(Queue* self, std::uint32_t count, void* const* lists) {
    if (count != 0 && !lists) throw std::invalid_argument("Command list array is null");
    std::array<ID3D12CommandList*, 16> local;
    std::span<ID3D12CommandList*> native(local);
    if (count > local.size()) {
        thread_local std::vector<ID3D12CommandList*> expanded;
        expanded.resize(count);
        native = expanded;
    }
    auto type = self->description.Type;
    if (static_cast<std::uint32_t>(type) == 0x10) type = D3D12_COMMAND_LIST_TYPE_COPY;
    for (std::uint32_t index = 0; index < count; ++index) {
        auto* list = CommandList::From(lists[index]);
        if (list->GetParent() != self->device) throw std::invalid_argument("Command list and queue devices differ");
        if (!list->IsClosed()) throw std::logic_error("Recording command list cannot be executed");
        if (list->GetNativeType() != type) throw std::invalid_argument("Command list and queue types differ");
        native[index] = list->GetHost();
    }
    self->host->ExecuteCommandLists(count, count == 0 ? nullptr : native.data());
}
