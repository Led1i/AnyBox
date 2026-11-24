#include "Resource.hpp"
#include "../object/PrivateData.hpp"
#include <anybox/platform/WindowsError.hpp>
#include <cwchar>
#include <limits>

AnyBox::Graphics::PrivateData& AnyBox::Graphics::Resource::getPrivateData() {
    auto* current = privateData.load(std::memory_order_acquire);
    if (current) return *current;
    auto created = std::make_unique<PrivateData>();
    if (privateData.compare_exchange_strong(current, created.get(), std::memory_order_acq_rel, std::memory_order_acquire)) return *created.release();
    return *current;
}

HRESULT AnyBox::Graphics::Resource::GetPrivateData(Resource* self, const Guid& key, std::uint32_t* size, void* data) {
    auto result = self->getPrivateData().Get(key, size, data);
    CheckGraphicsResult(result, "Get resource private data");
    return result;
}

HRESULT AnyBox::Graphics::Resource::SetPrivateData(Resource* self, const Guid& key, std::uint32_t size, const void* data) { return self->getPrivateData().Set(key, size, data); }
HRESULT AnyBox::Graphics::Resource::SetPrivateDataInterface(Resource* self, const Guid& key, const IUnknown* object) { return self->getPrivateData().SetInterface(key, object); }

HRESULT AnyBox::Graphics::Resource::SetName(Resource* self, const wchar_t* name) {
    if (!name) throw std::invalid_argument("Graphics object name is null");
    auto length = std::wcslen(name);
    if (length >= std::numeric_limits<std::uint32_t>::max() / sizeof(wchar_t)) throw std::length_error("Graphics object name is too long");
    constexpr Guid nameId{0x4CCA5FD8, 0x921F, 0x42C8, {0x85, 0x66, 0x70, 0xCA, 0xF2, 0xA9, 0xB7, 0x41}};
    return SetPrivateData(self, nameId, static_cast<std::uint32_t>((length + 1) * sizeof(wchar_t)), name);
}
