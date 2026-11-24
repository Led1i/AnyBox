#pragma once

#include <cstdint>
#include <map>
#include <mutex>

namespace AnyBox::Texture {
class Resource;

class AddressSpace {
public:
    ~AddressSpace();
    Resource& Resolve(std::uint64_t address) const;

private:
    friend class Resource;
    void insert(std::uint64_t address, Resource& resource);
    void remove(std::uint64_t address) noexcept;
    mutable std::mutex mutex;
    std::map<std::uint64_t, Resource*> resources;
};
}
