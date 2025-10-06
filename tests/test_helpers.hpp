#ifndef HAKKA_JSON_TEST_HELPERS_HPP
#define HAKKA_JSON_TEST_HELPERS_HPP

#include <string>
#include <cstdint>

namespace hakka_test {

/**
 * @brief Append a UTF-32 code point to a std::wstring with proper encoding
 *
 * On Windows, wchar_t is 16-bit (UTF-16) and requires surrogate pairs for
 * code points >= 0x10000 (outside the Basic Multilingual Plane).
 * On Unix-like systems, wchar_t is 32-bit (UTF-32) and can store any code point directly.
 *
 * @param wstr The wide string to append to
 * @param utf32 The UTF-32 code point (char32_t or uint32_t)
 */
inline void append_utf32_to_wstring(std::wstring& wstr, uint32_t utf32)
{
#ifdef _WIN32
    // Windows wchar_t is UTF-16, need surrogate pairs for code points >= 0x10000
    if (utf32 >= 0x10000)
    {
        uint32_t c = utf32 - 0x10000;
        wstr.push_back(static_cast<wchar_t>(0xD800 + (c >> 10)));     // High surrogate
        wstr.push_back(static_cast<wchar_t>(0xDC00 + (c & 0x3FF)));   // Low surrogate
    }
    else
    {
        wstr.push_back(static_cast<wchar_t>(utf32));
    }
#else
    // Unix-like systems: wchar_t is UTF-32
    wstr.push_back(static_cast<wchar_t>(utf32));
#endif
}

} // namespace hakka_test

#endif // HAKKA_JSON_TEST_HELPERS_HPP
