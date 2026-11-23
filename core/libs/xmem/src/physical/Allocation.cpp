#include "Pool.hpp"

#include <anybox/platform/WindowsError.hpp>

#include <exception>
#include <limits>
#include <set>
#include <stdexcept>

AnyBox::XMem::Section::~Section() {
    if (!CloseHandle(handle)) std::terminate();
}

void AnyBox::XMem::Pool::Allocate(std::uint32_t flags, std::size_t count, std::uint64_t* frames) {
    if ((flags & ~0x380U) != 0) throw std::invalid_argument("Unsupported XMem physical allocation flags");
    if (count == 0 || !frames) throw std::invalid_argument("XMem physical allocation requires a count and output");
    if (count > std::numeric_limits<std::uint64_t>::max() / PhysicalPageSize) throw std::overflow_error("XMem physical allocation size overflows");
    auto size = static_cast<std::uint64_t>(count) * PhysicalPageSize;
    auto handle = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, static_cast<DWORD>(size >> 32), static_cast<DWORD>(size), nullptr);
    if (!handle) Windows::ThrowLastError("CreateFileMappingW physical pages");
    std::shared_ptr<Section> section;
    try {
        section = std::make_shared<Section>(handle);
    } catch (...) {
        CloseHandle(handle);
        throw;
    }
    std::lock_guard lock(mutex);
    if (count > (std::numeric_limits<std::uint64_t>::max() - nextFrame) / 16) throw std::overflow_error("XMem physical frame identifiers exhausted");
    std::map<std::uint64_t, Page> added;
    for (std::size_t i = 0; i < count; ++i) {
        auto frame = nextFrame + i * 16;
        added.emplace(frame, Page{section, i * PhysicalPageSize, frame});
    }
    pages.merge(added);
    for (std::size_t i = 0; i < count; ++i) frames[i] = nextFrame + i * 16;
    nextFrame += count * 16;
}

void AnyBox::XMem::Pool::Free(std::span<const std::uint64_t> frames) {
    std::lock_guard lock(mutex);
    std::set<std::uint64_t> requested;
    for (auto frame : frames) {
        if (!requested.insert(frame).second) throw std::invalid_argument("Duplicate XMem physical frame in free request");
        if (!pages.contains(frame)) throw std::invalid_argument("XMem physical frame is not allocated");
    }
    for (const auto& [address, mapping] : mappings) {
        if (requested.contains(mapping.frame)) throw std::logic_error("XMem physical frame must be unmapped before release");
    }
    for (auto frame : frames) pages.erase(frame);
}

AnyBox::XMem::Pool& AnyBox::XMem::GetPool() {
    static Pool pool;
    return pool;
}
