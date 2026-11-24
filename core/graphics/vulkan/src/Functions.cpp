#include <anybox/vulkan/Context.hpp>

void AnyBox::Vulkan::Context::loadInstanceFunctions() {
    instanceFunctions.destroyInstance = RequireFunction<PFN_vkDestroyInstance>(getInstanceProcAddr(instance, "vkDestroyInstance"), "vkDestroyInstance");
    instanceFunctions.enumeratePhysicalDevices = RequireFunction<PFN_vkEnumeratePhysicalDevices>(getInstanceProcAddr(instance, "vkEnumeratePhysicalDevices"), "vkEnumeratePhysicalDevices");
    instanceFunctions.getPhysicalDeviceProperties = RequireFunction<PFN_vkGetPhysicalDeviceProperties2>(getInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2"), "vkGetPhysicalDeviceProperties2");
    instanceFunctions.getMemoryProperties = RequireFunction<PFN_vkGetPhysicalDeviceMemoryProperties>(getInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties"), "vkGetPhysicalDeviceMemoryProperties");
    instanceFunctions.getQueueFamilies = RequireFunction<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(getInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties"), "vkGetPhysicalDeviceQueueFamilyProperties");
    instanceFunctions.getFeatures = RequireFunction<PFN_vkGetPhysicalDeviceFeatures2>(getInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures2"), "vkGetPhysicalDeviceFeatures2");
    instanceFunctions.enumerateExtensions = RequireFunction<PFN_vkEnumerateDeviceExtensionProperties>(getInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties"), "vkEnumerateDeviceExtensionProperties");
    instanceFunctions.createDevice = RequireFunction<PFN_vkCreateDevice>(getInstanceProcAddr(instance, "vkCreateDevice"), "vkCreateDevice");
    instanceFunctions.getDeviceProcAddr = RequireFunction<PFN_vkGetDeviceProcAddr>(getInstanceProcAddr(instance, "vkGetDeviceProcAddr"), "vkGetDeviceProcAddr");
    instanceFunctions.getExternalBufferProperties = RequireFunction<PFN_vkGetPhysicalDeviceExternalBufferProperties>(getInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalBufferProperties"), "vkGetPhysicalDeviceExternalBufferProperties");
}

void AnyBox::Vulkan::Context::loadDeviceFunctions() {
#define ANYBOX_VULKAN_FUNCTION(field, name) deviceFunctions.field = RequireFunction<PFN_##name>(instanceFunctions.getDeviceProcAddr(device, #name), #name)
    ANYBOX_VULKAN_FUNCTION(destroyDevice, vkDestroyDevice);
    ANYBOX_VULKAN_FUNCTION(getDeviceQueue, vkGetDeviceQueue);
    ANYBOX_VULKAN_FUNCTION(createBuffer, vkCreateBuffer);
    ANYBOX_VULKAN_FUNCTION(destroyBuffer, vkDestroyBuffer);
    ANYBOX_VULKAN_FUNCTION(getBufferRequirements, vkGetBufferMemoryRequirements2);
    ANYBOX_VULKAN_FUNCTION(allocateMemory, vkAllocateMemory);
    ANYBOX_VULKAN_FUNCTION(freeMemory, vkFreeMemory);
    ANYBOX_VULKAN_FUNCTION(bindBufferMemory, vkBindBufferMemory);
    ANYBOX_VULKAN_FUNCTION(getHostPointerProperties, vkGetMemoryHostPointerPropertiesEXT);
    ANYBOX_VULKAN_FUNCTION(getBufferAddress, vkGetBufferDeviceAddress);
    ANYBOX_VULKAN_FUNCTION(mapMemory, vkMapMemory);
    ANYBOX_VULKAN_FUNCTION(unmapMemory, vkUnmapMemory);
    ANYBOX_VULKAN_FUNCTION(createCommandPool, vkCreateCommandPool);
    ANYBOX_VULKAN_FUNCTION(destroyCommandPool, vkDestroyCommandPool);
    ANYBOX_VULKAN_FUNCTION(allocateCommandBuffers, vkAllocateCommandBuffers);
    ANYBOX_VULKAN_FUNCTION(beginCommandBuffer, vkBeginCommandBuffer);
    ANYBOX_VULKAN_FUNCTION(endCommandBuffer, vkEndCommandBuffer);
    ANYBOX_VULKAN_FUNCTION(pipelineBarrier, vkCmdPipelineBarrier);
    ANYBOX_VULKAN_FUNCTION(copyBuffer, vkCmdCopyBuffer);
    ANYBOX_VULKAN_FUNCTION(createFence, vkCreateFence);
    ANYBOX_VULKAN_FUNCTION(destroyFence, vkDestroyFence);
    ANYBOX_VULKAN_FUNCTION(queueSubmit, vkQueueSubmit);
    ANYBOX_VULKAN_FUNCTION(waitForFences, vkWaitForFences);
    ANYBOX_VULKAN_FUNCTION(queueWaitIdle, vkQueueWaitIdle);
#undef ANYBOX_VULKAN_FUNCTION
}
