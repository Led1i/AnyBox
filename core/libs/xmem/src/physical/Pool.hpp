#ifndef ANYBOX_XMEM_PHYSICAL_POOL_HPP
#define ANYBOX_XMEM_PHYSICAL_POOL_HPP

#include <anybox/platform/Windows.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <vector>

namespace AnyBox::XMem {

constexpr std::size_t PhysicalPageSize = 65536;

struct Section {
    HANDLE handle;
    explicit Section(HANDLE handle) : handle(handle) {}
    ~Section();
    Section(const Section&) = delete;
    Section& operator=(const Section&) = delete;
};

struct Page {
    std::shared_ptr<Section> section;
    std::uint64_t offset;
    std::uint64_t frame;
};

struct MappingChange {
    void* address;
    std::optional<Page> previous;
    std::optional<Page> next;
};

class Pool {
public:
    void Allocate(std::uint64_t flags, std::size_t count, std::uint64_t* frames);
    void Free(std::span<const std::uint64_t> frames);
    void Map(std::span<void* const> addresses, const std::uint64_t* frames);

private:
    std::mutex mutex;
    std::uint64_t nextFrame = 16;
    std::map<std::uint64_t, Page> pages;
    std::map<std::uintptr_t, Page> mappings;
};

Pool& GetPool();
void EnsurePlaceholder(void* address);
void ApplyMapping(const MappingChange& change);
void RestoreMapping(const MappingChange& change);

}

#endif
