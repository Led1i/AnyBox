#pragma once

#include "../device/Device.hpp"
#include <stdexcept>
#include <string>
#include <utility>

namespace AnyBox::Graphics {
template<class TObject, std::size_t TSlot>
[[noreturn]] void UnsupportedObjectMethod(void*, ...) { throw std::runtime_error(std::string(ANYBOX_LIBRARY_NAME) + "!Object method " + std::to_string(TSlot) + " is not implemented"); }

template<class TObject, std::size_t... TSlots>
DeviceTable MakeObjectTable(std::index_sequence<TSlots...>) {
    DeviceTable table{MethodAddress(&UnsupportedObjectMethod<TObject, TSlots>)...};
    table[0] = MethodAddress(&TObject::QueryInterface);
    table[1] = MethodAddress(&TObject::AddRef);
    table[2] = MethodAddress(&TObject::Release);
    table[3] = MethodAddress(&TObject::GetPrivateData);
    table[4] = MethodAddress(&TObject::SetPrivateData);
    table[5] = MethodAddress(&TObject::SetPrivateDataInterface);
    table[6] = MethodAddress(&TObject::SetName);
    table[7] = MethodAddress(&TObject::GetDevice);
    return table;
}

template<class TObject>
const DeviceTable& GetObjectTable() {
    static const auto table = MakeObjectTable<TObject>(std::make_index_sequence<128>{});
    return table;
}
}
