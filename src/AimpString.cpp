#include "AimpString.h"

#include "apiCore.h"
#include "ComBase.h"

AimpCoreRef::AimpCoreRef(IAIMPCore* core) {
    reset(core);
}

AimpCoreRef::~AimpCoreRef() {
    reset(nullptr);
}

void AimpCoreRef::reset(IAIMPCore* core) {
    if (core_) {
        core_->Release();
    }
    core_ = core;
    if (core_) {
        core_->AddRef();
    }
}

IAIMPString* MakeAimpString(IAIMPCore* core, const std::wstring& value) {
    if (!core) {
        return nullptr;
    }

    IAIMPString* result = nullptr;
    if (FAILED(core->CreateObject(IID_IAIMPString, reinterpret_cast<void**>(&result)))) {
        return nullptr;
    }

    result->SetData(const_cast<TChar*>(value.c_str()), static_cast<int>(value.size()));
    return result;
}

std::wstring FromAimpString(IAIMPString* value) {
    if (!value || !value->GetData() || value->GetLength() <= 0) {
        return {};
    }
    return std::wstring(value->GetData(), value->GetData() + value->GetLength());
}
