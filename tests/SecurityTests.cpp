#include <iostream>
#include <string>
#include <windows.h>

#include "Diagnostics.h"

HMODULE g_moduleHandle = nullptr;

namespace {

bool Contains(const std::wstring& text, const std::wstring& needle) {
    return text.find(needle) != std::wstring::npos;
}

bool Expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << "\n";
        return false;
    }
    return true;
}

} // namespace

int main() {
    bool ok = true;

    const std::wstring redactedUrl = RedactSensitiveUrl(
        L"https://music.example/rest/stream.view?u=demo_user&t=token123&s=salt123&p=plain&id=song");
    ok &= Expect(!Contains(redactedUrl, L"demo_user"), "username query value was not redacted");
    ok &= Expect(!Contains(redactedUrl, L"token123"), "token query value was not redacted");
    ok &= Expect(!Contains(redactedUrl, L"salt123"), "salt query value was not redacted");
    ok &= Expect(!Contains(redactedUrl, L"plain"), "password query value was not redacted");
    ok &= Expect(Contains(redactedUrl, L"id=song"), "non-sensitive query value was unexpectedly removed");

    const std::wstring redactedText = RedactSensitiveText(
        L"password=secret token: abc salt=\"def\" Authorization: Bearer Value");
    ok &= Expect(!Contains(redactedText, L"secret"), "password text value was not redacted");
    ok &= Expect(!Contains(redactedText, L"abc"), "token text value was not redacted");
    ok &= Expect(!Contains(redactedText, L"def"), "salt text value was not redacted");
    ok &= Expect(!Contains(redactedText, L"Bearer Value"), "authorization text value was not redacted");

    return ok ? 0 : 1;
}
