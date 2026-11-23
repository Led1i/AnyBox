#ifndef ANYBOX_IACPHAL_HPP
#define ANYBOX_IACPHAL_HPP

#include <cstdint>

struct AcpHalMessage {
    std::uint32_t type;
    std::uint32_t reserved;
    std::uint64_t value;
    std::uint64_t id;
};

struct IAcpHal {
    virtual std::int32_t Connect(std::uint32_t commandCapacity, std::uint32_t messageCapacity) = 0;
    virtual std::int32_t Disconnect() = 0;
    virtual std::int32_t SubmitCommand(std::uint32_t type, std::uint64_t id, std::uint32_t frame, const void* data, std::uint32_t flags) = 0;
    virtual bool PopMessage(AcpHalMessage* message) = 0;
    virtual std::uint32_t GetNumMessages() = 0;
    virtual void Release() = 0;
};

static_assert(sizeof(AcpHalMessage) == 24);

#endif
