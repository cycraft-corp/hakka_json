#include <hakka_json_string.hpp>
#include <hakka_json_array.hpp>

#include <gtest/gtest.h>

using namespace hakka;

namespace detail
{
    template <typename T>
    const T *h2t(const JsonHandleCompact &handle)
    {
        auto view = handle.get_view();
        return std::get<const T*>(view);
    }
}

// Test Creation and Dumping
TEST(JsonString, CreateAndDump)
{
    auto json_str = JsonStringCompact::create("Hello, World!");
    ASSERT_TRUE(json_str);
    ASSERT_EQ(json_str.get_type(), HakkaJsonType::HAKKA_JSON_STRING);

    auto view = json_str.get_view();
    auto dump_result = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonStringCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(dump_result);
    ASSERT_EQ(dump_result.value(), "\"Hello, World!\"");
}

// Test Comparison
TEST(JsonString, Compare)
{
    auto json_str1 = JsonStringCompact::create("apple");
    auto json_str2 = JsonStringCompact::create("apple");
    auto json_str3 = JsonStringCompact::create("banana");

    ASSERT_TRUE(json_str1);
    ASSERT_TRUE(json_str2);
    ASSERT_TRUE(json_str3);

    // Compare identical strings
    auto result = detail::h2t<JsonStringCompact>(json_str1)->compare(json_str2);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result, 0);

    // Compare different strings
    result = detail::h2t<JsonStringCompact>(json_str1)->compare(json_str3);
    ASSERT_TRUE(result);
    ASSERT_LT(*result, 0);

    result = detail::h2t<JsonStringCompact>(json_str3)->compare(json_str1);
    ASSERT_TRUE(result);
    ASSERT_GT(*result, 0);
}

// Test Serialization to Bytes
TEST(JsonString, ToBytes)
{
    auto json_str = JsonStringCompact::create("Test String");
    ASSERT_TRUE(json_str);

    char buffer[512];
    uint32_t buffer_size = sizeof(buffer);
    auto result = detail::h2t<JsonStringCompact>(json_str)->to_bytes(buffer, &buffer_size);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    ASSERT_EQ(buffer_size, 13); // "\"Test String\"".size() = 12 + 1 for null terminator
    ASSERT_STREQ(buffer, "\"Test String\"");
}

TEST(JsonString, DumpSize)
{
    {
        auto json_str = JsonStringCompact::create("Test String");
        ASSERT_TRUE(json_str);

        auto size = detail::h2t<JsonStringCompact>(json_str)->dump_size();
        ASSERT_EQ(size, 13);
    }
    {
        auto json_str = JsonStringCompact::create("Hello, World!");
        ASSERT_TRUE(json_str);

        auto size = detail::h2t<JsonStringCompact>(json_str)->dump_size();
        ASSERT_EQ(size, 15);
    }
    // empty string
    {
        auto json_str = JsonStringCompact::create("");
        ASSERT_TRUE(json_str);

        auto size = detail::h2t<JsonStringCompact>(json_str)->dump_size();
        ASSERT_EQ(size, 2);
    }
}

// Test Reference Counting
TEST(JsonString, RefCount)
{
    auto json_str = JsonStringCompact::create("RefCount Test");
    ASSERT_TRUE(json_str);

    ASSERT_EQ(detail::h2t<JsonStringCompact>(json_str)->inc_ref(), 2);
    ASSERT_EQ(detail::h2t<JsonStringCompact>(json_str)->dec_ref(), 1);
}

// Test Getter Method
TEST(JsonString, Get)
{
    auto json_str = JsonStringCompact::create("Getter Test");
    ASSERT_TRUE(json_str);

    auto json_handle = detail::h2t<JsonStringCompact>(json_str)->get();
    ASSERT_TRUE(json_handle);
    ASSERT_EQ(json_handle.value().index(), PrimitiveType{std::string("Getter Test")}.index());
    ASSERT_EQ(std::get<std::string>(json_handle.value()), "Getter Test");
}

// Test Invalid ToBytes (Insufficient Buffer)
TEST(JsonString, InvalidToBytes)
{
    auto json_str = JsonStringCompact::create("This is a very long string that exceeds the buffer size.");
    ASSERT_TRUE(json_str);

    char buffer[10]; // Insufficient buffer size
    uint32_t buffer_size = sizeof(buffer);
    auto result = detail::h2t<JsonStringCompact>(json_str)->to_bytes(buffer, &buffer_size);
    ASSERT_EQ(result, HAKKA_JSON_NOT_ENOUGH_MEMORY);
    ASSERT_EQ(buffer_size, static_cast<uint32_t>(detail::h2t<JsonStringCompact>(json_str)->dump(512).value().size()) + 1);
}

// Test Unicode Handling with Accented Characters
TEST(JsonString, AccentedCharacters)
{
    auto json_str = JsonStringCompact::create("Straße"); // German word with 'ß'
    ASSERT_TRUE(json_str);

    // Test Upper
    auto upper = detail::h2t<JsonStringCompact>(json_str)->upper();
    ASSERT_TRUE(upper);
    auto upper_str = detail::h2t<JsonStringCompact>(upper.value())->get();
    ASSERT_TRUE(upper_str);
    // Depending on locale, 'ß' may become 'SS' in uppercase
    ASSERT_EQ(std::get<std::string>(upper_str.value()), "STRASSE");

    // Test Lower
    auto lower = detail::h2t<JsonStringCompact>(json_str)->lower();
    ASSERT_TRUE(lower);
    auto lower_str = detail::h2t<JsonStringCompact>(lower.value())->get();
    ASSERT_TRUE(lower_str);
    ASSERT_EQ(std::get<std::string>(lower_str.value()), "straße");

    // Test Capitalize
    auto capitalized = detail::h2t<JsonStringCompact>(json_str)->capitalize();
    ASSERT_TRUE(capitalized);
    auto cap_str = detail::h2t<JsonStringCompact>(capitalized.value())->get();
    ASSERT_TRUE(cap_str);
    ASSERT_EQ(std::get<std::string>(cap_str.value()), "Straße");

    // Test IsAlpha
    auto isalpha = detail::h2t<JsonStringCompact>(json_str)->isalpha();
    ASSERT_TRUE(isalpha);
    ASSERT_TRUE(*isalpha);

    // Test IsAscii
    auto isascii = detail::h2t<JsonStringCompact>(json_str)->isascii();
    ASSERT_TRUE(isascii);
    ASSERT_FALSE(*isascii);
}

// Test Unicode Handling with Non-Latin Scripts (Arabic)
TEST(JsonString, NonLatinScripts)
{
    auto json_str = JsonStringCompact::create("العربية"); // Arabic word meaning 'Arabic'
    ASSERT_TRUE(json_str);

    // Test Upper
    auto upper = detail::h2t<JsonStringCompact>(json_str)->upper();
    ASSERT_TRUE(upper);
    auto upper_str = detail::h2t<JsonStringCompact>(upper.value())->get();
    ASSERT_TRUE(upper_str);
    // Arabic script does not have case distinction; should remain the same
    ASSERT_EQ(std::get<std::string>(upper_str.value()), "العربية");

    // Test Lower
    auto lower = detail::h2t<JsonStringCompact>(json_str)->lower();
    ASSERT_TRUE(lower);
    auto lower_str = detail::h2t<JsonStringCompact>(lower.value())->get();
    ASSERT_TRUE(lower_str);
    ASSERT_EQ(std::get<std::string>(lower_str.value()), "العربية");

    // Test IsAlpha
    auto isalpha = detail::h2t<JsonStringCompact>(json_str)->isalpha();
    ASSERT_TRUE(isalpha);
    ASSERT_TRUE(*isalpha);

    // Test IsAscii
    auto isascii = detail::h2t<JsonStringCompact>(json_str)->isascii();
    ASSERT_TRUE(isascii);
    ASSERT_FALSE(*isascii);

    // Test Reverse Slice (assuming your slice supports negative steps)
    auto reversed = detail::h2t<JsonStringCompact>(json_str)->slice(-1, -6, -1);
    ASSERT_TRUE(reversed);
    auto rev_str = detail::h2t<JsonStringCompact>(reversed.value())->get();
    ASSERT_TRUE(rev_str);
    // Arabic text is RTL (Right-to-Left); ensure slicing respects this
    ASSERT_EQ(std::get<std::string>(rev_str.value()), "ةيبرع");
}

// Test Unicode Handling with Emojis and Complex Scripts
TEST(JsonString, EmojisAndComplexScripts)
{
    auto json_str = JsonStringCompact::create("👨‍👩‍👧‍👦 family"); // Emoji with zero-width joiners
    ASSERT_TRUE(json_str);

    // Test Length
    auto length = detail::h2t<JsonStringCompact>(json_str)->length();
    ASSERT_TRUE(length);
    // Depending on how length is computed (code units vs. code points)
    // This test may need adjustment
    // For illustration, we assume the length is computed in code points
    // The emoji '👨‍👩‍👧‍👦' is a single visual emoji but may consist of multiple code points

    // Test IsPrintable
    auto isprintable = detail::h2t<JsonStringCompact>(json_str)->isprintable();
    ASSERT_TRUE(isprintable);
    ASSERT_FALSE(*isprintable); // python is false

    // Test Contains
    auto contains = detail::h2t<JsonStringCompact>(json_str)->find(std::string_view("family"));
    ASSERT_TRUE(contains);
    ASSERT_GT(*contains, 0);

    // Test Split (assuming it can handle splitting on emojis or complex scripts)
    auto splitted = detail::h2t<JsonStringCompact>(json_str)->split(std::string_view(" "));
    ASSERT_TRUE(splitted);
    auto arr = detail::h2t<JsonArrayCompact>(splitted.value());
    ASSERT_TRUE(arr);
    ASSERT_EQ(arr->length(), 2);
    ASSERT_EQ(detail::h2t<JsonStringCompact>(arr->at(0).value())->get().value(), PrimitiveType("👨‍👩‍👧‍👦"));
    ASSERT_EQ(detail::h2t<JsonStringCompact>(arr->at(1).value())->get().value(), PrimitiveType("family"));
}

// Test Case Conversion with Turkish 'İ' and 'i' Dotless I
TEST(JsonString, TurkishLocale)
{
    auto json_str = JsonStringCompact::create("istanbul");
    ASSERT_TRUE(json_str);

    // Test Upper
    auto upper = detail::h2t<JsonStringCompact>(json_str)->upper();
    ASSERT_TRUE(upper);
    auto upper_str = detail::h2t<JsonStringCompact>(upper.value())->get();
    ASSERT_TRUE(upper_str);
    // In Turkish locale, 'i' becomes 'İ' (Latin capital letter I with dot above)
    // Depending on locale settings, this may vary
    // ASSERT_EQ(std::get<std::string>(upper_str.value()), "İSTANBUL");

    // Test Lower with 'İ'
    auto json_str2 = JsonStringCompact::create("İSTANBUL");
    ASSERT_TRUE(json_str2);

    auto lower = detail::h2t<JsonStringCompact>(json_str2)->lower();
    ASSERT_TRUE(lower);
    auto lower_str = detail::h2t<JsonStringCompact>(lower.value())->get();
    ASSERT_TRUE(lower_str);
    // In Turkish locale, 'İ' becomes 'i'
    // ASSERT_EQ(std::get<std::string>(lower_str.value()), "istanbul");
}

// Test String Methods with Japanese Characters
TEST(JsonString, JapaneseCharacters)
{
    auto json_str = JsonStringCompact::create("こんにちは"); // "Hello" in Japanese
    ASSERT_TRUE(json_str);

    // Test Upper and Lower (should have no effect)
    auto upper = detail::h2t<JsonStringCompact>(json_str)->upper();
    ASSERT_TRUE(upper);
    auto upper_str = detail::h2t<JsonStringCompact>(upper.value())->get();
    ASSERT_EQ(std::get<std::string>(upper_str.value()), "こんにちは");

    auto lower = detail::h2t<JsonStringCompact>(json_str)->lower();
    ASSERT_TRUE(lower);
    auto lower_str = detail::h2t<JsonStringCompact>(lower.value())->get();
    ASSERT_EQ(std::get<std::string>(lower_str.value()), "こんにちは");

    // Test IsAlpha
    auto isalpha = detail::h2t<JsonStringCompact>(json_str)->isalpha();
    ASSERT_TRUE(isalpha);
    ASSERT_TRUE(*isalpha);

    // Test IsAscii
    auto isascii = detail::h2t<JsonStringCompact>(json_str)->isascii();
    ASSERT_TRUE(isascii);
    ASSERT_FALSE(*isascii);

    // Test Length
    auto length = detail::h2t<JsonStringCompact>(json_str)->length();
    ASSERT_TRUE(length);
}

// Test String Methods with Right-to-Left Scripts (Hebrew)
TEST(JsonString, HebrewScript)
{
    auto json_str = JsonStringCompact::create("שלום"); // "Hello" in Hebrew
    ASSERT_TRUE(json_str);

    // Test Reverse Slice
    auto reversed = detail::h2t<JsonStringCompact>(json_str)->slice(-1, -5, -1);
    ASSERT_TRUE(reversed);
    auto rev_str = detail::h2t<JsonStringCompact>(reversed.value())->get();
    ASSERT_TRUE(rev_str);
    // Ensure the reversed string is correct
    ASSERT_EQ(std::get<std::string>(rev_str.value()), "םולש");

    // Test IsAlpha
    auto isalpha = detail::h2t<JsonStringCompact>(json_str)->isalpha();
    ASSERT_TRUE(isalpha);
    ASSERT_TRUE(*isalpha);

    // test IsTitle
    auto istitle = detail::h2t<JsonStringCompact>(json_str)->istitle();
    ASSERT_TRUE(istitle);
    ASSERT_FALSE(*istitle);
}

// Test IsDigit with Unicode Digits
TEST(JsonString, UnicodeDigits)
{
    auto json_str = JsonStringCompact::create("１２３４５"); // Full-width digits
    ASSERT_TRUE(json_str);

    // Test IsDigit
    auto isdigit = detail::h2t<JsonStringCompact>(json_str)->isdigit();
    ASSERT_TRUE(isdigit);
    ASSERT_TRUE(*isdigit);

    // Test IsNumeric
    auto isnumeric = detail::h2t<JsonStringCompact>(json_str)->isnumeric();
    ASSERT_TRUE(isnumeric);
    ASSERT_TRUE(*isnumeric);

    // Test IsDecimal
    auto isdecimal = detail::h2t<JsonStringCompact>(json_str)->isdecimal();
    ASSERT_TRUE(isdecimal);
    ASSERT_TRUE(*isdecimal);
}

// Test Methods with Combining Characters
TEST(JsonString, CombiningCharacters)
{
    auto json_str = JsonStringCompact::create("Å"); // 'A' + combining ring above (should look like 'Å')
    ASSERT_TRUE(json_str);

    // Test Upper
    auto upper = detail::h2t<JsonStringCompact>(json_str)->upper();
    ASSERT_TRUE(upper);
    auto upper_str = detail::h2t<JsonStringCompact>(upper.value())->get();
    ASSERT_EQ(std::get<std::string>(upper_str.value()), "Å");

    // Test IsAlpha
    auto isalpha = detail::h2t<JsonStringCompact>(json_str)->isalpha();
    ASSERT_TRUE(isalpha);
    ASSERT_TRUE(*isalpha);
}

// Test String Methods with Directional Characters
TEST(JsonString, DirectionalCharacters)
{
    auto json_str = JsonStringCompact::create("abc\u202Edef"); // 'abc' + Right-To-Left Override + 'def'
    ASSERT_TRUE(json_str);

    // Test IsPrintable
    auto isprintable = detail::h2t<JsonStringCompact>(json_str)->isprintable();
    ASSERT_TRUE(isprintable);
    ASSERT_TRUE(*isprintable);

    // Test Length
    auto length = detail::h2t<JsonStringCompact>(json_str)->length();
    ASSERT_TRUE(length);
    // Verify the length, considering special characters
}

// Test String Methods with Surrogate Pairs
TEST(JsonString, SurrogatePairs)
{
    // Emoji represented by surrogate pairs
    auto json_str = JsonStringCompact::create("\U0001F600"); // Grinning face emoji
    ASSERT_TRUE(json_str);

    // Test IsPrintable
    auto isprintable = detail::h2t<JsonStringCompact>(json_str)->isprintable();
    ASSERT_TRUE(isprintable);
    ASSERT_TRUE(*isprintable);

    // Test Length
    auto length = detail::h2t<JsonStringCompact>(json_str)->length();
    ASSERT_TRUE(length);
    // Verify length considering surrogate pairs
}

// test is printable especially ensure "こんにちは世界"
TEST(JsonString, IsPrintable)
{
    auto json_str = JsonStringCompact::create("こんにちは世界");
    ASSERT_TRUE(json_str);

    auto isprintable = detail::h2t<JsonStringCompact>(json_str)->isprintable();
    ASSERT_TRUE(isprintable);
    ASSERT_TRUE(*isprintable);
}

// test the iterator
TEST(JsonString, Iterator)
{
    {
        auto json_str = JsonStringCompact::create("Hello, World!");
        ASSERT_TRUE(json_str);

        auto iter = detail::h2t<JsonStringCompact>(json_str)->begin();
        auto end = detail::h2t<JsonStringCompact>(json_str)->end();

        std::string str;
        for (; iter != end; ++iter)
        {
            str.push_back(*iter);
        }

        ASSERT_EQ(str, "Hello, World!");
    }

    //  iterator for i18n and emoji
    {
        // auto json_str = JsonString::create("こんにちは👨‍👩‍👧");
        auto json_str = JsonStringCompact::create("こんにちは");
        ASSERT_TRUE(json_str);

        auto iter = detail::h2t<JsonStringCompact>(json_str)->begin();
        auto end = detail::h2t<JsonStringCompact>(json_str)->end();

        std::wstring str;
        for (; iter != end; ++iter)
        {
            str.push_back(*iter);
        }

        // TODO: Fix 👨‍👩‍👧‍👦 support if code point is >16 bits
        // ASSERT_EQ(str, L"こんにちは👨‍👩‍👧‍👦");
        ASSERT_EQ(str, L"こんにちは");
    }
}

// test for "何𢪻諺一票, 何𢪻諺一票", split, iter, strlen...
TEST(JsonString, ChineseCharacters)
{
    // spilt with whitespace
    {
        auto json_str = JsonStringCompact::create("何𢪻諺一票, 何𢪻諺一票");
        ASSERT_TRUE(json_str);

        auto splitted = detail::h2t<JsonStringCompact>(json_str)->split(std::string_view(" "));
        ASSERT_TRUE(splitted);
        auto arr = detail::h2t<JsonArrayCompact>(splitted.value());
        ASSERT_TRUE(arr);
        ASSERT_EQ(arr->length(), 2);
        ASSERT_EQ(detail::h2t<JsonStringCompact>(arr->at(0).value())->get().value(), PrimitiveType("何𢪻諺一票,"));
        ASSERT_EQ(detail::h2t<JsonStringCompact>(arr->at(1).value())->get().value(), PrimitiveType("何𢪻諺一票"));
    }

    // test iter
    {
        auto json_str = JsonStringCompact::create("何𢪻諺一票, 何𢪻諺一票");
        ASSERT_TRUE(json_str);

        auto iter = detail::h2t<JsonStringCompact>(json_str)->begin();
        auto end = detail::h2t<JsonStringCompact>(json_str)->end();

        std::wstring str;
        for (; iter != end; ++iter)
        {
            str.push_back(*iter);
        }

        ASSERT_EQ(str, L"何𢪻諺一票, 何𢪻諺一票");
    }

    // test strlen
    {
        auto json_str = JsonStringCompact::create("何𢪻諺一票, 何𢪻諺一票");
        ASSERT_TRUE(json_str);

        auto length = detail::h2t<JsonStringCompact>(json_str)->length();
        ASSERT_TRUE(length);
        ASSERT_EQ(*length, 14);
    }

    // test slice
    {
        auto json_str = JsonStringCompact::create("何𢪻諺一票, 何𢪻諺一票");
        ASSERT_TRUE(json_str);

        auto sliced = detail::h2t<JsonStringCompact>(json_str)->slice(0, 6); // [0, 6)
        ASSERT_TRUE(sliced);
        auto str = detail::h2t<JsonStringCompact>(sliced.value())->get();
        ASSERT_TRUE(str);
        ASSERT_EQ(std::get<std::string>(str.value()), "何𢪻諺一票");
    }
}

TEST(JsonString, Utf8Length)
{
    auto json_str = JsonStringCompact::create("繁體中文");
    ASSERT_TRUE(json_str);

    auto length = detail::h2t<JsonStringCompact>(json_str)->utf8_length();
    ASSERT_TRUE(length);
    ASSERT_EQ(*length, 12);

    // in the meanwhile, we can test the length method
    auto len = detail::h2t<JsonStringCompact>(json_str)->length();
    ASSERT_TRUE(len);
    ASSERT_EQ(*len, 4);
}

TEST(JsonString, IsPrintable2)
{
    auto json_str = JsonStringCompact::create("!@#$%^&*()_+-=[]{}|;:',.<>/?`~");
    ASSERT_TRUE(json_str);

    auto isprintable = detail::h2t<JsonStringCompact>(json_str)->isprintable();
    ASSERT_TRUE(isprintable);
    ASSERT_TRUE(*isprintable);
}
