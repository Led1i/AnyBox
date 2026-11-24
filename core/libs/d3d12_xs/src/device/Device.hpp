#pragma once

#include <d3d12_xsTypes.hpp>
#include <anybox/Types.hpp>
#include <anybox/platform/Windows.hpp>
#include <d3d12.h>
#include "../object/Properties.hpp"
#include <array>
#include <atomic>
#include <cstring>
#include <type_traits>

namespace AnyBox::Graphics {
using DeviceMethod = std::array<std::byte, sizeof(void (*)())>;
using DeviceTable = std::array<DeviceMethod, 128>;

template<class TFunction>
DeviceMethod MethodAddress(TFunction function) {
    DeviceMethod result{};
    static_assert(sizeof(function) == sizeof(result));
    std::memcpy(result.data(), &function, sizeof(function));
    return result;
}

inline constexpr Guid XboxDeviceId{0x5405C344, 0xD457, 0x444E, {0xB4, 0xDD, 0x23, 0x66, 0xE4, 0x5A, 0xEE, 0x39}};
inline constexpr Guid UnknownId{0, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46}};
const DeviceTable& GetDeviceTable();
struct XboxQueueDescription;
GUID NativeGuid(const Guid& value);
void CheckGraphicsResult(HRESULT result, const char* operation);

class Device : public ObjectProperties<Device> {
public:
    Device(ID3D12Device* host, const D3D12XboxCreateDeviceParameters& parameters);
    ~Device();
    static std::int32_t QueryInterface(Device* self, const Guid& interfaceId, void** object);
    static std::uint32_t AddRef(Device* self);
    static std::uint32_t Release(Device* self);
    ID3D12Device* GetHost() const noexcept { return host; }
    bool Matches(const D3D12XboxCreateDeviceParameters& value) const noexcept;
    static std::uint32_t TryAddRef(Device* self);
    static std::uint32_t GetNodeCount(Device* self);
    static D3D12_HEAP_PROPERTIES GetCustomHeapProperties(Device* self, std::uint32_t nodeMask, D3D12_HEAP_TYPE type);
    static HRESULT CreateCommandQueue(Device* self, const D3D12_COMMAND_QUEUE_DESC* description, const Guid& interfaceId, void** queue);
    static HRESULT CreateFence(Device* self, std::uint64_t initialValue, D3D12_FENCE_FLAGS flags, const Guid& interfaceId, void** fence);
    static HRESULT CreateCommandQueueX(Device* self, const XboxQueueDescription* description, const Guid& interfaceId, void** queue);
    void ReleaseQueueSelection(std::uint32_t selection) noexcept;

private:
    const DeviceTable* vtable;
    std::atomic<std::uint32_t> references{1};
    std::atomic<std::uint32_t> computeQueues{0};
    ID3D12Device* host;
    D3D12XboxCreateDeviceParameters parameters;
};

static_assert(std::is_standard_layout_v<Device>);
static_assert(sizeof(DeviceMethod) == 8);
Device* AcquireDevice(ID3D12Device* host, const D3D12XboxCreateDeviceParameters& parameters);
void ForgetDevice(Device* device);
}
