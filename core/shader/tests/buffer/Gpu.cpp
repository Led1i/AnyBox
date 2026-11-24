#include "Descriptors.hpp"
#include "ComputeCompiler.hpp"
#include "Commands.hpp"
#include "Fixtures.hpp"
#include <anybox/vulkan/ComputePipeline.hpp>
#include <BdaAbi.hpp>
#include <cstring>
#include <vector>

namespace BufferTests {
void CheckGpu(const AnyBox::Shader::Dxil::ComputeCompilation& shader, std::uint32_t expected) {
    namespace Abi = ShaderRecompiler::BdaAbi;
    using namespace AnyBox::Vulkan;
    Context context(0);
    if (!context.SupportsByteStorage()) throw std::runtime_error("DXIL BDA test requires Vulkan byte storage support");
    constexpr auto usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    constexpr auto properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    auto input = Buffer::Allocate(context, 64, usage, properties);
    auto output = Buffer::Allocate(context, 4, usage, properties);
    auto faults = Buffer::Allocate(context, Abi::FaultBufferBytes, usage, properties);
    struct PageTable {
        Abi::Header header;
        std::array<Abi::Range, 2> ranges;
    };
    static_assert(sizeof(PageTable) == 80);
    auto table = Buffer::Allocate(context, sizeof(PageTable), usage, properties);
    PageTable pages{{Abi::Version, 2, sizeof(Abi::Range), 0}, {{{0x100000000, 0x100000040, input.GetAddress(), Abi::Read, 0}, {0x200000000, 0x200000004, output.GetAddress(), Abi::Write, 0}}}};
    for (std::size_t index = 0; index < input.Map().size(); ++index) input.Map()[index] = static_cast<std::byte>((index * 3 + 7) & 255);
    std::memset(output.Map().data(), 255, output.Map().size());
    std::memset(faults.Map().data(), 0, faults.Map().size());
    std::memcpy(table.Map().data(), &pages, sizeof(pages));
    Descriptors descriptors(context, table, faults);
    std::array layouts{descriptors.GetLayout()};
    std::array sets{descriptors.GetSet()};
    ComputePipeline pipeline(context, shader.code, shader.bindings.pushConstantSizeBytes, "main", layouts);
    AnyBox::Vulkan::Tests::Commands commands(context);
    auto buffer = commands.Begin();
    VkMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    context.GetFunctions().pipelineBarrier(buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
    std::vector<std::byte> constants(shader.bindings.pushConstantSizeBytes);
    pipeline.Dispatch(buffer, constants, {1, 1, 1}, sets);
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    context.GetFunctions().pipelineBarrier(buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &barrier, 0, nullptr, 0, nullptr);
    commands.Submit(buffer);
    Abi::Fault fault{};
    std::memcpy(&fault, faults.Map().data(), sizeof(fault));
    if (fault.state != Abi::FaultState::Empty) throw std::runtime_error("DXIL BDA GPU test recorded fault " + std::to_string(static_cast<unsigned>(fault.reason)));
    std::uint32_t value = 0;
    std::memcpy(&value, output.Map().data(), sizeof(value));
    if (value != expected) throw std::runtime_error("DXIL packed i16 GPU result mismatch: " + std::to_string(value));
}
}
