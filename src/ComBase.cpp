#include "ComBase.h"

ULONG ComBase::AddRefImpl() {
    return ++refCount_;
}

ULONG ComBase::ReleaseImpl() {
    const ULONG value = --refCount_;
    if (value == 0) {
        delete this;
    }
    return value;
}
