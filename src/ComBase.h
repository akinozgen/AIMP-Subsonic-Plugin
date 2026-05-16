#pragma once

#include <atomic>
#include <unknwn.h>

class ComBase {
public:
    ComBase() = default;
    virtual ~ComBase() = default;

    ULONG AddRefImpl();
    ULONG ReleaseImpl();

private:
    std::atomic<ULONG> refCount_{1};
};

template <typename T>
void SafeRelease(T*& value) {
    if (value) {
        value->Release();
        value = nullptr;
    }
}
