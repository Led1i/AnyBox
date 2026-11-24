#include "Device.hpp"
#include <stdexcept>
#include <string>
#include <utility>

namespace {
template<std::size_t TSlot>
[[noreturn]] void Unsupported(void*, ...) {
    throw std::runtime_error(std::string(ANYBOX_LIBRARY_NAME) + "!Device method " + std::to_string(TSlot) + " is not implemented");
}

template<std::size_t... TSlots>
AnyBox::Graphics::DeviceTable MakeTable(std::index_sequence<TSlots...>) {
    using namespace AnyBox::Graphics;
    DeviceTable result{MethodAddress(&Unsupported<TSlots>)...};
    result[0] = MethodAddress(&Device::QueryInterface);
    result[1] = MethodAddress(&Device::AddRef);
    result[2] = MethodAddress(&Device::Release);
    result[3] = MethodAddress(&Device::GetPrivateData);
    result[4] = MethodAddress(&Device::SetPrivateData);
    result[5] = MethodAddress(&Device::SetPrivateDataInterface);
    result[6] = MethodAddress(&Device::SetName);
    result[7] = MethodAddress(&Device::GetNodeCount);
    result[8] = MethodAddress(&Device::CreateCommandQueue);
    result[9] = MethodAddress(&Device::CreateCommandAllocator);
    result[26] = MethodAddress(&Device::GetCustomHeapProperties);
    result[36] = MethodAddress(&Device::CreateFence);
    result[48] = MethodAddress(&Device::CreateCommandQueueX);
    return result;
}
}

const AnyBox::Graphics::DeviceTable& AnyBox::Graphics::GetDeviceTable() {
    static const auto table = MakeTable(std::make_index_sequence<128>{});
    return table;
}
