#pragma once

#include <anybox/Types.hpp>
#include <anybox/platform/Windows.hpp>
#include <unknwn.h>
#include <stdexcept>

namespace AnyBox::Graphics {
GUID NativeGuid(const Guid& value);
void CheckGraphicsResult(HRESULT result, const char* operation);

template<class TObject>
class ObjectProperties {
public:
    static HRESULT GetPrivateData(TObject* self, const Guid& key, std::uint32_t* size, void* data) {
        if (!size) throw std::invalid_argument("Graphics private data size output is null");
        auto result = self->GetHost()->GetPrivateData(NativeGuid(key), size, data);
        CheckGraphicsResult(result, "ID3D12Object::GetPrivateData");
        return result;
    }

    static HRESULT SetPrivateData(TObject* self, const Guid& key, std::uint32_t size, const void* data) {
        if (size != 0 && !data) throw std::invalid_argument("Graphics private data is null");
        auto result = self->GetHost()->SetPrivateData(NativeGuid(key), size, data);
        CheckGraphicsResult(result, "ID3D12Object::SetPrivateData");
        return result;
    }

    static HRESULT SetPrivateDataInterface(TObject* self, const Guid& key, const IUnknown* object) {
        auto result = self->GetHost()->SetPrivateDataInterface(NativeGuid(key), object);
        CheckGraphicsResult(result, "ID3D12Object::SetPrivateDataInterface");
        return result;
    }

    static HRESULT SetName(TObject* self, const wchar_t* name) {
        if (!name) throw std::invalid_argument("Graphics object name is null");
        auto result = self->GetHost()->SetName(name);
        CheckGraphicsResult(result, "ID3D12Object::SetName");
        return result;
    }
};
}
