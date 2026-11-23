#ifndef ANYBOX_STORAGE_READ_LEASE_HPP
#define ANYBOX_STORAGE_READ_LEASE_HPP

#include "../common/Handle.hpp"

namespace AnyBox::Storage {

class File;

class ReadLease {
public:
    ReadLease() = default;
    ReadLease(File* owner, Handle handle) noexcept;
    ReadLease(ReadLease&& other) noexcept;
    ReadLease(const ReadLease&) = delete;
    ReadLease& operator=(const ReadLease&) = delete;
    ReadLease& operator=(ReadLease&&) = delete;
    ~ReadLease();
    HANDLE Get() const;

private:
    File* owner = nullptr;
    Handle handle;
};

}

#endif
