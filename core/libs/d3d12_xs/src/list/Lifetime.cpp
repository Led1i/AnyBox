#include "List.hpp"
#include <stdexcept>

AnyBox::Graphics::CommandList::CommandList(ID3D12GraphicsCommandList* host, Device* device, Allocator* allocator, D3D12_COMMAND_LIST_TYPE type) : vtable(&GetListTable()), host(host), device(device), allocator(allocator), type(type) {
    Device::AddRef(device);
    Allocator::AddRef(allocator);
}

AnyBox::Graphics::CommandList::~CommandList() {
    host->Release();
    Allocator::Release(allocator);
    Device::Release(device);
}

AnyBox::Graphics::CommandList* AnyBox::Graphics::CommandList::From(void* object) {
    if (!object) throw std::invalid_argument("Command list is null");
    const ListTable* table;
    std::memcpy(&table, object, sizeof(table));
    if (table != &GetListTable()) throw std::invalid_argument("Foreign command list interface is unsupported");
    return static_cast<CommandList*>(object);
}

bool AnyBox::Graphics::CommandList::SupportsCreation(D3D12_COMMAND_LIST_TYPE type, const Guid& interfaceId) noexcept {
    if (interfaceId == DmaListId) return static_cast<std::uint32_t>(type) == 0x10;
    return interfaceId == CommandListId || interfaceId == GraphicsListId || interfaceId == XboxGraphicsListId || interfaceId == UnknownId || interfaceId == ObjectId || interfaceId == DeviceChildId;
}

bool AnyBox::Graphics::CommandList::Supports(const Guid& interfaceId) const noexcept { return SupportsCreation(type, interfaceId); }

D3D12_COMMAND_LIST_TYPE AnyBox::Graphics::CommandList::GetType(CommandList* self) { return self->type; }
