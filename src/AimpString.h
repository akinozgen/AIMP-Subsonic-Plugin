#pragma once

#include <string>
#include "apiCore.h"
#include "apiObjects.h"

class AimpCoreRef {
public:
    explicit AimpCoreRef(IAIMPCore* core = nullptr);
    AimpCoreRef(const AimpCoreRef&) = delete;
    AimpCoreRef& operator=(const AimpCoreRef&) = delete;
    ~AimpCoreRef();

    IAIMPCore* get() const { return core_; }
    void reset(IAIMPCore* core);

private:
    IAIMPCore* core_{nullptr};
};

IAIMPString* MakeAimpString(IAIMPCore* core, const std::wstring& value);
std::wstring FromAimpString(IAIMPString* value);
