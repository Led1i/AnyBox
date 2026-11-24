#include "Tests.hpp"

namespace {
class Commands {
public:
    explicit Commands(AnyBox::Vulkan::Context& context) : context(context) {
        const auto& functions = context.GetFunctions();
        VkCommandPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        info.queueFamilyIndex = context.GetQueueFamily();
        AnyBox::Vulkan::Check(functions.createCommandPool(context.GetDevice(), &info, nullptr, &pool), "Create Vulkan transfer test pool");
    }
    ~Commands() {
        const auto& functions = context.GetFunctions();
        if (pending) {
            auto result = functions.queueWaitIdle(context.GetQueue(0));
            if (result != VK_SUCCESS && result != VK_ERROR_DEVICE_LOST) std::terminate();
        }
        if (fence) functions.destroyFence(context.GetDevice(), fence, nullptr);
        functions.destroyCommandPool(context.GetDevice(), pool, nullptr);
    }
    VkCommandBuffer Begin() {
        VkCommandBufferAllocateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.commandPool = pool;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = 1;
        VkCommandBuffer buffer = VK_NULL_HANDLE;
        AnyBox::Vulkan::Check(context.GetFunctions().allocateCommandBuffers(context.GetDevice(), &info, &buffer), "Allocate Vulkan transfer test commands");
        VkCommandBufferBeginInfo begin{};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        AnyBox::Vulkan::Check(context.GetFunctions().beginCommandBuffer(buffer, &begin), "Begin Vulkan transfer test commands");
        return buffer;
    }
    void Submit(VkCommandBuffer buffer) {
        const auto& functions = context.GetFunctions();
        AnyBox::Vulkan::Check(functions.endCommandBuffer(buffer), "End Vulkan transfer test commands");
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        AnyBox::Vulkan::Check(functions.createFence(context.GetDevice(), &fenceInfo, nullptr, &fence), "Create Vulkan transfer test fence");
        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &buffer;
        AnyBox::Vulkan::Check(functions.queueSubmit(context.GetQueue(0), 1, &info, fence), "Submit Vulkan transfer test commands");
        pending = true;
        AnyBox::Vulkan::Check(functions.waitForFences(context.GetDevice(), 1, &fence, VK_TRUE, 5000000000ull), "Wait Vulkan transfer test completion");
        pending = false;
    }

private:
    AnyBox::Vulkan::Context& context;
    VkCommandPool pool = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    bool pending = false;
};
}

void AnyBox::Vulkan::Tests::Copy(Context& context, const Buffer& source, const Buffer& destination, VkDeviceSize size) {
    if (size == 0 || size > source.GetSize() || size > destination.GetSize()) throw std::invalid_argument("Vulkan transfer test range is invalid");
    Commands commands(context);
    auto buffer = commands.Begin();
    const auto& functions = context.GetFunctions();
    VkMemoryBarrier before{};
    before.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    before.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    before.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    functions.pipelineBarrier(buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &before, 0, nullptr, 0, nullptr);
    VkBufferCopy region{0, 0, size};
    functions.copyBuffer(buffer, source.GetHandle(), destination.GetHandle(), 1, &region);
    VkMemoryBarrier after{};
    after.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    after.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    after.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    functions.pipelineBarrier(buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &after, 0, nullptr, 0, nullptr);
    commands.Submit(buffer);
}
