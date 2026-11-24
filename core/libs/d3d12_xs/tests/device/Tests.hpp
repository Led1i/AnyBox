#pragma once

#include "../../src/device/Device.hpp"
#include <anybox/ExceptionTest.hpp>
#include <cstring>
#include <stdexcept>

namespace AnyBox::Graphics::Tests {
inline void Require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

template<class TFunction, class TObject>
TFunction Method(TObject* device, std::size_t slot) {
    const DeviceTable* table;
    std::memcpy(&table, device, sizeof(table));
    TFunction result;
    static_assert(sizeof(result) == sizeof(DeviceMethod));
    std::memcpy(&result, (*table)[slot].data(), sizeof(result));
    return result;
}

template<class TException, class TCallable>
void Reject(TCallable&& callable) {
    int destructors = 0;
    bool caught = false;
    try { AnyBox::Tests::Invoke(callable, destructors); } catch (const TException&) { caught = true; }
    Require(caught && destructors == 1, "Graphics device exception did not unwind its caller");
}

void CheckProperties(Device* device);
void CheckQueues(Device* device);
void CheckFences(Device* device);
void CheckAllocators(Device* device);
void CheckLists(Device* device);
void CheckRoots(Device* device);
}
