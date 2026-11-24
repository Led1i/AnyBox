#include <anybox/vulkan/Context.hpp>
#include <algorithm>
#include <cstring>

void AnyBox::Vulkan::Context::selectAdapter(std::uint32_t adapterIndex, std::uint32_t queueCount) {
    std::uint32_t count = 0;
    Check(instanceFunctions.enumeratePhysicalDevices(instance, &count, nullptr), "Count Vulkan adapters");
    std::vector<VkPhysicalDevice> devices(count);
    Check(instanceFunctions.enumeratePhysicalDevices(instance, &count, devices.data()), "Enumerate Vulkan adapters");
    if (adapterIndex >= count) throw std::out_of_range("Vulkan adapter index is unavailable");
    physicalDevice = devices[adapterIndex];
    VkPhysicalDeviceExternalMemoryHostPropertiesEXT host{};
    host.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT;
    VkPhysicalDeviceProperties2 physical{};
    physical.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    physical.pNext = &host;
    instanceFunctions.getPhysicalDeviceProperties(physicalDevice, &physical);
    properties = physical.properties;
    if (properties.apiVersion < VK_API_VERSION_1_2) throw std::runtime_error("Vulkan adapter does not support API version 1.2");
    hostPointerAlignment = host.minImportedHostPointerAlignment;
    instanceFunctions.getMemoryProperties(physicalDevice, &memoryProperties);
    instanceFunctions.getQueueFamilies(physicalDevice, &count, nullptr);
    std::vector<VkQueueFamilyProperties> families(count);
    instanceFunctions.getQueueFamilies(physicalDevice, &count, families.data());
    constexpr auto required = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
    auto family = std::ranges::find_if(families, [&](const auto& value) { return (value.queueFlags & required) == required && value.queueCount >= queueCount; });
    if (family == families.end()) throw std::runtime_error("Vulkan adapter has no universal queue family with the requested queue count");
    queueFamily = static_cast<std::uint32_t>(family - families.begin());
    Check(instanceFunctions.enumerateExtensions(physicalDevice, nullptr, &count, nullptr), "Count Vulkan device extensions");
    std::vector<VkExtensionProperties> extensions(count);
    Check(instanceFunctions.enumerateExtensions(physicalDevice, nullptr, &count, extensions.data()), "Enumerate Vulkan device extensions");
    if (std::ranges::none_of(extensions, [](const auto& value) { return std::strcmp(value.extensionName, VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME) == 0; })) throw std::runtime_error("Vulkan host memory imports are unsupported");
    if (hostPointerAlignment == 0) throw std::runtime_error("Vulkan host pointer alignment is zero");
}

void AnyBox::Vulkan::Context::createDevice(std::uint32_t queueCount) {
    VkPhysicalDeviceBufferDeviceAddressFeatures address{};
    address.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    VkPhysicalDeviceFeatures2 features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features.pNext = &address;
    instanceFunctions.getFeatures(physicalDevice, &features);
    if (!address.bufferDeviceAddress) throw std::runtime_error("Vulkan buffer device addresses are unsupported");
    if (!features.features.shaderInt64) throw std::runtime_error("Vulkan shader 64-bit integers are unsupported");
    VkPhysicalDeviceFeatures enabled{};
    enabled.shaderInt64 = VK_TRUE;
    address.bufferDeviceAddressCaptureReplay = VK_FALSE;
    address.bufferDeviceAddressMultiDevice = VK_FALSE;
    std::vector<float> priorities(queueCount, 1.0f);
    VkDeviceQueueCreateInfo queue{};
    queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue.queueFamilyIndex = queueFamily;
    queue.queueCount = queueCount;
    queue.pQueuePriorities = priorities.data();
    const char* extensions[]{VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME};
    VkDeviceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.pNext = &address;
    info.pEnabledFeatures = &enabled;
    info.queueCreateInfoCount = 1;
    info.pQueueCreateInfos = &queue;
    info.enabledExtensionCount = 1;
    info.ppEnabledExtensionNames = extensions;
    Check(instanceFunctions.createDevice(physicalDevice, &info, nullptr, &device), "Create Vulkan device");
}
