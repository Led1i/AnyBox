#pragma once

#include "Functions.hpp"
#include <anybox/platform/Windows.hpp>
#include <vector>
#include <cstdint>

namespace AnyBox::Vulkan {
class Context {
public:
    explicit Context(std::uint32_t adapterIndex, std::uint32_t queueCount = 1);
    ~Context();
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    VkDevice GetDevice() const noexcept { return device; }
    VkPhysicalDevice GetPhysicalDevice() const noexcept { return physicalDevice; }
    const DeviceFunctions& GetFunctions() const noexcept { return deviceFunctions; }
    const VkPhysicalDeviceProperties& GetProperties() const noexcept { return properties; }
    const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const noexcept { return memoryProperties; }
    VkDeviceSize GetHostPointerAlignment() const noexcept { return hostPointerAlignment; }
    std::uint32_t GetQueueFamily() const noexcept { return queueFamily; }
    VkQueue GetQueue(std::uint32_t index) const;
    VkExternalBufferProperties GetExternalBufferProperties(VkBufferUsageFlags usage) const;

private:
    void loadInstanceFunctions();
    void loadDeviceFunctions();
    void selectAdapter(std::uint32_t adapterIndex, std::uint32_t queueCount);
    void createDevice(std::uint32_t queueCount);
    void release() noexcept;
    HMODULE module = nullptr;
    PFN_vkGetInstanceProcAddr getInstanceProcAddr = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    InstanceFunctions instanceFunctions{};
    DeviceFunctions deviceFunctions{};
    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    VkDeviceSize hostPointerAlignment = 0;
    std::uint32_t queueFamily = 0;
    std::vector<VkQueue> queues;
};
}
