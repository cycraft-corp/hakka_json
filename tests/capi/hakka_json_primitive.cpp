// hakka_json_primitive_test.cpp

#include <gtest/gtest.h>
#include "hakka_json_primitive.h"
#include <cstring>

// Helper function to create a string handle
HakkaJsonResultEnum CreateStringHandle(HakkaHandle *handle, const std::string &str)
{
    return CreateHakkaString(handle, reinterpret_cast<const uint8_t *>(str.c_str()), static_cast<uint32_t>(str.size()));
}

// Test Fixture for Hakka JSON Primitive Tests
class HakkaJsonPrimitiveTest : public ::testing::Test
{
protected:
    // SetUp and TearDown can be used if needed
    void SetUp() override {}
    void TearDown() override {}
};

// -----------------------------
// Primitive Type Tests
// -----------------------------

TEST_F(HakkaJsonPrimitiveTest, CreateAndGetInt)
{
    HakkaHandle handle;
    ASSERT_EQ(CreateHakkaInt(&handle, 42), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    int64_t value = 0;
    ASSERT_EQ(GetHakkaInt(handle, &value), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(value, 42);

    // Test type
    HakkaJsonType type;
    ASSERT_EQ(HakkaType(handle, &type), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(type, HAKKA_JSON_INT);

    // Clean up
    HakkaRelease(&handle);
    EXPECT_EQ(handle, 0);
}

TEST_F(HakkaJsonPrimitiveTest, CreateAndGetFloat)
{
    HakkaHandle handle;
    ASSERT_EQ(CreateHakkaFloat(&handle, 3.14f), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    double value = 0.0f;
    ASSERT_EQ(GetHakkaFloat(handle, &value), HAKKA_JSON_SUCCESS);
    EXPECT_FLOAT_EQ(value, 3.14f);

    // Test type
    HakkaJsonType type;
    ASSERT_EQ(HakkaType(handle, &type), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(type, HAKKA_JSON_FLOAT);

    // Clean up
    HakkaRelease(&handle);
    EXPECT_EQ(handle, 0);
}

TEST_F(HakkaJsonPrimitiveTest, CreateAndGetBool)
{
    HakkaHandle handle;
    ASSERT_EQ(CreateHakkaBool(&handle, 1), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    C_BOOL value = 0;
    ASSERT_EQ(GetHakkaBool(handle, &value), HAKKA_JSON_SUCCESS);
    EXPECT_TRUE(value);

    // Test type
    HakkaJsonType type;
    ASSERT_EQ(HakkaType(handle, &type), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(type, HAKKA_JSON_BOOL);

    // Clean up
    HakkaRelease(&handle);
    EXPECT_EQ(handle, 0);
}

TEST_F(HakkaJsonPrimitiveTest, CreateAndGetNull)
{
    HakkaHandle handle;
    ASSERT_EQ(CreateHakkaNull(&handle), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    // Test validity
    C_BOOL is_valid = 0;
    ASSERT_EQ(HakkaIsValid(handle, &is_valid), HAKKA_JSON_SUCCESS);
    EXPECT_TRUE(is_valid);

    // Test type
    HakkaJsonType type;
    ASSERT_EQ(HakkaType(handle, &type), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(type, HAKKA_JSON_NULL);

    // Clean up
    HakkaRelease(&handle);
    EXPECT_EQ(handle, 0);
}

TEST_F(HakkaJsonPrimitiveTest, CreateAndGetInvalid)
{
    HakkaHandle handle;
    ASSERT_EQ(CreateHakkaInvalid(&handle), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(handle, 0);

    // Test validity, A handle holds an invalid value, is a valid handle
    C_BOOL is_valid = 0;
    ASSERT_EQ(HakkaIsValid(handle, &is_valid), HAKKA_JSON_SUCCESS);
    EXPECT_FALSE(is_valid);

    // Test type
    HakkaJsonType type;
    ASSERT_EQ(HakkaType(handle, &type), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(type, HAKKA_JSON_INVALID);

    // Clean up
    HakkaRelease(&handle);
    EXPECT_EQ(handle, 0);
}

// -----------------------------
// String Type Tests
// -----------------------------

TEST_F(HakkaJsonPrimitiveTest, CreateAndGetString)
{
    HakkaHandle handle;
    std::string test_str = "Hello, Hakka!";
    ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    // Retrieve string
    uint32_t buffer_size = static_cast<uint32_t>(test_str.size() + 1);
    uint8_t buffer[50] = {0};
    ASSERT_EQ(GetHakkaString(handle, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), test_str.c_str());

    // Test length
    uint32_t length = 0;
    ASSERT_EQ(GetHakkaStringLength(handle, &length), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(length, test_str.size());

    // Test type
    HakkaJsonType type;
    ASSERT_EQ(HakkaType(handle, &type), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(type, HAKKA_JSON_STRING);

    // Clean up
    HakkaRelease(&handle);
    EXPECT_EQ(handle, 0);
}

TEST_F(HakkaJsonPrimitiveTest, CreateStringWithNullValue)
{
    HakkaHandle handle = 0;
    ASSERT_EQ(CreateHakkaString(&handle, nullptr, 0), HAKKA_JSON_INVALID_ARGUMENT);
    EXPECT_EQ(handle, 0);
}

// -----------------------------
// Base Function Tests
// -----------------------------

TEST_F(HakkaJsonPrimitiveTest, DumpJsonInt)
{
    HakkaHandle handle;
    ASSERT_EQ(CreateHakkaInt(&handle, 100), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    uint8_t buffer[50] = {0};
    uint64_t buffer_size = sizeof(buffer);
    ASSERT_EQ(HakkaDump(handle, 2, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "100");

    // Clean up
    HakkaRelease(&handle);
    EXPECT_EQ(handle, 0);
}

TEST_F(HakkaJsonPrimitiveTest, DumpJsonString)
{
    HakkaHandle handle;
    std::string test_str = "Test String";
    ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    uint8_t buffer[50] = {0};
    uint64_t buffer_size = sizeof(buffer);
    ASSERT_EQ(HakkaDump(handle, 2, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), "\"Test String\"");

    // Clean up
    HakkaRelease(&handle);
    EXPECT_EQ(handle, 0);
}

TEST_F(HakkaJsonPrimitiveTest, CompareHandles)
{
    HakkaHandle handle1;
    HakkaHandle handle2;
    HakkaHandle handle3;

    ASSERT_EQ(CreateHakkaInt(&handle1, 10), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(CreateHakkaInt(&handle2, 10), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(CreateHakkaInt(&handle3, 20), HAKKA_JSON_SUCCESS);

    int32_t result = 0;
    ASSERT_EQ(HakkaCompare(handle1, handle2, &result), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(result, 0); // Equal

    ASSERT_EQ(HakkaCompare(handle1, handle3, &result), HAKKA_JSON_SUCCESS);
    EXPECT_LT(result, 0); // handle1 < handle3

    // Clean up
    HakkaRelease(&handle1);
    HakkaRelease(&handle2);
    HakkaRelease(&handle3);
}

TEST_F(HakkaJsonPrimitiveTest, HashFunction)
{
    HakkaHandle handle1;
    HakkaHandle handle2;

    ASSERT_EQ(CreateHakkaInt(&handle1, 12345), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(CreateHakkaInt(&handle2, 12345), HAKKA_JSON_SUCCESS);

    uint64_t hash1 = 0, hash2 = 0;
    ASSERT_EQ(HakkaHash(handle1, &hash1), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(HakkaHash(handle2, &hash2), HAKKA_JSON_SUCCESS);
    EXPECT_EQ(hash1, hash2); // Hashes should be equal for same values

    // Clean up
    HakkaRelease(&handle1);
    HakkaRelease(&handle2);
}

TEST_F(HakkaJsonPrimitiveTest, DumpSizeFunction)
{
    { // int
        HakkaHandle handle;
        ASSERT_EQ(CreateHakkaInt(&handle, 500), HAKKA_JSON_SUCCESS);
        ASSERT_NE(handle, 0);

        uint64_t capacity = 0;
        ASSERT_EQ(HakkaDumpSize(handle, &capacity), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(capacity, 3); // 500 is 3 characters

        // Clean up
        HakkaRelease(&handle);
    }
    {
        // string
        HakkaHandle handle;
        std::string test_str = "Test String";
        ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
        ASSERT_NE(handle, 0);

        uint64_t capacity = 0;
        ASSERT_EQ(HakkaDumpSize(handle, &capacity), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(capacity, 13); // "Test String" is 13 characters

        // Clean up
        HakkaRelease(&handle);
    }
    {
        // null
        HakkaHandle handle;
        ASSERT_EQ(CreateHakkaNull(&handle), HAKKA_JSON_SUCCESS);
        ASSERT_NE(handle, 0);

        uint64_t capacity = 0;
        ASSERT_EQ(HakkaDumpSize(handle, &capacity), HAKKA_JSON_SUCCESS);
        EXPECT_EQ(capacity, 4); // "null" is 4 characters

        // Clean up
        HakkaRelease(&handle);
    }
    {
        // invalid
        HakkaHandle handle;
        ASSERT_EQ(CreateHakkaInvalid(&handle), HAKKA_JSON_SUCCESS);
        ASSERT_EQ(handle, 0);

        uint64_t capacity = 0;
        ASSERT_EQ(HakkaDumpSize(handle, &capacity), HAKKA_JSON_SUCCESS);
        EXPECT_EQ(capacity, 7); // invalid is 7 characters

        // Clean up
        HakkaRelease(&handle);
    }
}

// reclaim
TEST_F(HakkaJsonPrimitiveTest, ReclaimFunction)
{
    HakkaHandle handle;
    ASSERT_EQ(CreateHakkaInt(&handle, 500), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    ASSERT_EQ(HakkaReclaim(handle), HAKKA_JSON_SUCCESS);
    HakkaHandle reclaimed_handle = handle;

    // Clean up: should release twice
    HakkaRelease(&handle);
    EXPECT_EQ(handle, 0);

    // Use the reclaimed handle
    uint64_t capacity = 0;
    ASSERT_EQ(HakkaDumpSize(reclaimed_handle, &capacity), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(capacity, 3); // 500 is 3 characters

    HakkaRelease(&reclaimed_handle);
    EXPECT_EQ(reclaimed_handle, 0);
}

// -----------------------------
// String Manipulation Function Tests
// -----------------------------

TEST_F(HakkaJsonPrimitiveTest, StringCapitalize)
{
    HakkaHandle handle;
    std::string test_str = "hello world";
    std::string expected = "Hello world";

    ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    HakkaHandle capitalized;
    ASSERT_EQ(GetHakkaStringCapitalize(handle, &capitalized), HAKKA_JSON_SUCCESS);
    ASSERT_NE(capitalized, 0);

    uint8_t buffer[50] = {0};
    uint32_t buffer_size = sizeof(buffer);
    ASSERT_EQ(GetHakkaString(capitalized, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), expected.c_str());

    // Clean up
    HakkaRelease(&handle);
    HakkaRelease(&capitalized);
}

TEST_F(HakkaJsonPrimitiveTest, StringCasefold)
{
    HakkaHandle handle;
    std::string test_str = "HeLLo WoRLd";
    std::string expected = "hello world";

    ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    HakkaHandle casefolded;
    ASSERT_EQ(GetHakkaStringCasefold(handle, &casefolded), HAKKA_JSON_SUCCESS);
    ASSERT_NE(casefolded, 0);

    uint8_t buffer[50] = {0};
    uint32_t buffer_size = sizeof(buffer);
    ASSERT_EQ(GetHakkaString(casefolded, buffer, &buffer_size), HAKKA_JSON_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<const char *>(buffer), expected.c_str());

    // Clean up
    HakkaRelease(&handle);
    HakkaRelease(&casefolded);
}

TEST_F(HakkaJsonPrimitiveTest, StringCount)
{
    HakkaHandle handle;
    std::string test_str = "banana";
    std::string substring = "an";
    int64_t count = 0;

    ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    ASSERT_EQ(GetHakkaStringCount(handle,
                                  reinterpret_cast<const uint8_t *>(substring.c_str()),
                                  static_cast<uint32_t>(substring.size()),
                                  &count),
              HAKKA_JSON_SUCCESS);
    EXPECT_EQ(count, 2);

    // Clean up
    HakkaRelease(&handle);
}

TEST_F(HakkaJsonPrimitiveTest, StringEndswith)
{
    HakkaHandle handle;
    std::string test_str = "hello world";
    std::string suffix = "world";
    C_BOOL result = 0;

    ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    ASSERT_EQ(GetHakkaStringEndswith(handle,
                                     reinterpret_cast<const uint8_t *>(suffix.c_str()),
                                     static_cast<uint32_t>(suffix.size()),
                                     &result),
              HAKKA_JSON_SUCCESS);
    EXPECT_TRUE(result);

    // Test with incorrect suffix
    std::string wrong_suffix = "hello";
    ASSERT_EQ(GetHakkaStringEndswith(handle,
                                     reinterpret_cast<const uint8_t *>(wrong_suffix.c_str()),
                                     static_cast<uint32_t>(wrong_suffix.size()),
                                     &result),
              HAKKA_JSON_SUCCESS);
    EXPECT_FALSE(result);

    // Clean up
    HakkaRelease(&handle);
}

// Additional string manipulation tests would follow a similar pattern...

// -----------------------------
// String Testing Function Tests
// -----------------------------

TEST_F(HakkaJsonPrimitiveTest, StringIsalnum)
{
    HakkaHandle handle;
    std::string test_str = "Hello123";
    C_BOOL result = 0;

    ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    ASSERT_EQ(GetHakkaStringIsalnum(handle, &result), HAKKA_JSON_SUCCESS);
    EXPECT_TRUE(result);

    // Test with non-alnum string
    std::string non_alnum = "Hello World!";
    HakkaRelease(&handle);
    ASSERT_EQ(CreateStringHandle(&handle, non_alnum), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    ASSERT_EQ(GetHakkaStringIsalnum(handle, &result), HAKKA_JSON_SUCCESS);
    EXPECT_FALSE(result);

    // Clean up
    HakkaRelease(&handle);
}

TEST_F(HakkaJsonPrimitiveTest, StringIsalpha)
{
    HakkaHandle handle;
    std::string test_str = "HelloWorld";
    C_BOOL result = 0;

    ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    ASSERT_EQ(GetHakkaStringIsalpha(handle, &result), HAKKA_JSON_SUCCESS);
    EXPECT_TRUE(result);

    // Test with non-alpha string
    std::string non_alpha = "Hello123";
    HakkaRelease(&handle);
    ASSERT_EQ(CreateStringHandle(&handle, non_alpha), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    ASSERT_EQ(GetHakkaStringIsalpha(handle, &result), HAKKA_JSON_SUCCESS);
    EXPECT_FALSE(result);

    // Clean up
    HakkaRelease(&handle);
}

TEST_F(HakkaJsonPrimitiveTest, StringIsascii)
{
    HakkaHandle handle;
    std::string ascii_str = "Hello, World!";
    C_BOOL result = 0;

    ASSERT_EQ(CreateStringHandle(&handle, ascii_str), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    ASSERT_EQ(GetHakkaStringIsascii(handle, &result), HAKKA_JSON_SUCCESS);
    EXPECT_TRUE(result);

    // Test with non-ascii string
    std::string non_ascii = "こんにちは"; // "Hello" in Japanese
    HakkaRelease(&handle);
    ASSERT_EQ(CreateStringHandle(&handle, non_ascii), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    ASSERT_EQ(GetHakkaStringIsascii(handle, &result), HAKKA_JSON_SUCCESS);
    EXPECT_FALSE(result);

    // Clean up
    HakkaRelease(&handle);
}

// Additional string testing functions would follow a similar pattern...

// -----------------------------
// Error Handling Tests
// -----------------------------

TEST_F(HakkaJsonPrimitiveTest, CreateWithNullHandle)
{
    // Attempt to create an int with a null handle
    ASSERT_EQ(CreateHakkaInt(nullptr, 10), HAKKA_JSON_INVALID_ARGUMENT);

    // Attempt to create a string with null handle
    std::string test_str = "Test";
    ASSERT_EQ(CreateHakkaString(nullptr, reinterpret_cast<const uint8_t *>(test_str.c_str()), static_cast<uint32_t>(test_str.size())), HAKKA_JSON_INVALID_ARGUMENT);
}

TEST_F(HakkaJsonPrimitiveTest, GetWithNullPointers)
{
    HakkaHandle handle;
    ASSERT_EQ(CreateHakkaInt(&handle, 50), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    // Attempt to get int with null value pointer
    ASSERT_EQ(GetHakkaInt(handle, nullptr), HAKKA_JSON_INVALID_ARGUMENT);

    // Attempt to get string with null buffer
    ASSERT_EQ(GetHakkaString(handle, nullptr, nullptr), HAKKA_JSON_INVALID_ARGUMENT);

    // Clean up
    HakkaRelease(&handle);
}

TEST_F(HakkaJsonPrimitiveTest, GetWrongType)
{
    HakkaHandle int_handle;
    HakkaHandle string_handle;
    ASSERT_EQ(CreateHakkaInt(&int_handle, 25), HAKKA_JSON_SUCCESS);
    ASSERT_EQ(CreateStringHandle(&string_handle, "Test"), HAKKA_JSON_SUCCESS);

    int64_t int_value = 0;
    double float_value = 0.0f;

    // Attempt to get float from int handle
    ASSERT_EQ(GetHakkaFloat(int_handle, &float_value), HAKKA_JSON_TYPE_ERROR);

    // Attempt to get int from string handle
    ASSERT_EQ(GetHakkaInt(string_handle, &int_value), HAKKA_JSON_TYPE_ERROR);

    // Clean up
    HakkaRelease(&int_handle);
    HakkaRelease(&string_handle);
}

TEST_F(HakkaJsonPrimitiveTest, DumpWithInsufficientBuffer)
{
    HakkaHandle handle;
    std::string test_str = "Buffer Test";
    ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    uint8_t buffer[5] = {0}; // Intentionally small buffer
    uint64_t buffer_size = sizeof(buffer);
    ASSERT_EQ(HakkaDump(handle, 2, buffer, &buffer_size), HAKKA_JSON_NOT_ENOUGH_MEMORY);
    EXPECT_EQ(buffer_size, static_cast<uint64_t>(test_str.size() + 2));

    // Clean up
    HakkaRelease(&handle);
}

// Test string iterator
TEST_F(HakkaJsonPrimitiveTest, StringIterator)
{
    { // Normal string
        HakkaHandle handle;
        std::string test_str = "Hello, Hakka!";
        ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
        ASSERT_NE(handle, 0);

        HakkaStringIter iter = 0;
        ASSERT_EQ(CreateHakkaStringBegin(handle, &iter), HAKKA_JSON_SUCCESS);
        ASSERT_NE(iter, 0);

        // use a loop to loop through the string, until the MoveHakkaStringNext returns an HAKKA_JSON_ITERATOR_END
        uint32_t utf32 = 0;
        std::string result;
        do
        {
            ASSERT_EQ(GetHakkaStringDeref(iter, &utf32), HAKKA_JSON_SUCCESS);
            result.push_back(static_cast<char>(utf32));
        } while (MoveHakkaStringNext(iter) == HAKKA_JSON_SUCCESS);
        HakkaStringIterRelease(&iter);

        // Check if the result is the same as the original string
        EXPECT_EQ(result, test_str);

        // Clean up
        HakkaRelease(&handle);
    }

    { // Empty string
        HakkaHandle handle;
        std::string test_str = "";
        ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
        ASSERT_NE(handle, 0);

        HakkaStringIter iter = 0;
        ASSERT_EQ(CreateHakkaStringBegin(handle, &iter), HAKKA_JSON_SUCCESS);
        ASSERT_NE(iter, 0);

        // use a loop to loop through the string, until the MoveHakkaStringNext returns an HAKKA_JSON_ITERATOR_END
        uint32_t utf32 = 0;
        std::string result;
        do
        {
            ASSERT_EQ(GetHakkaStringDeref(iter, &utf32), HAKKA_JSON_ITERATOR_END);
            if (utf32 != 0)
                result.push_back(static_cast<char>(utf32));
        } while (MoveHakkaStringNext(iter) == HAKKA_JSON_SUCCESS);
        HakkaStringIterRelease(&iter);

        // Check if the result is the same as the original string
        EXPECT_EQ(result, test_str);

        // Clean up
        HakkaRelease(&handle);
    }

    { // String with Hindi characters
        HakkaHandle handle;
        std::string test_str = "नमस्ते";
        ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
        ASSERT_NE(handle, 0);

        HakkaStringIter iter = 0;
        ASSERT_EQ(CreateHakkaStringBegin(handle, &iter), HAKKA_JSON_SUCCESS);
        ASSERT_NE(iter, 0);

        // use a loop to loop through the string, until the MoveHakkaStringNext returns an HAKKA_JSON_ITERATOR_END
        uint32_t utf32 = 0;
        std::wstring result;
        do
        {
            ASSERT_EQ(GetHakkaStringDeref(iter, &utf32), HAKKA_JSON_SUCCESS);
            result.push_back(static_cast<wchar_t>(utf32));
        } while (MoveHakkaStringNext(iter) == HAKKA_JSON_SUCCESS);
        HakkaStringIterRelease(&iter);

        // Check if the result is the same as the original string
        EXPECT_EQ(result, L"नमस्ते");

        // Clean up
        HakkaRelease(&handle);
    }

    { // String with complex emojis
        HakkaHandle handle;
        std::string test_str = "👋🏼👋🏼👋🏼";
        ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
        ASSERT_NE(handle, 0);

        HakkaStringIter iter = 0;
        ASSERT_EQ(CreateHakkaStringBegin(handle, &iter), HAKKA_JSON_SUCCESS);
        ASSERT_NE(iter, 0);

        // use a loop to loop through the string, until the MoveHakkaStringNext returns an HAKKA_JSON_ITERATOR_END
        uint32_t utf32 = 0;
        std::wstring result;
        do
        {
            ASSERT_EQ(GetHakkaStringDeref(iter, &utf32), HAKKA_JSON_SUCCESS);
            result.push_back(static_cast<wchar_t>(utf32));
        } while (MoveHakkaStringNext(iter) == HAKKA_JSON_SUCCESS);
        HakkaStringIterRelease(&iter);

        // Check if the result is the same as the original string
        EXPECT_EQ(result, L"👋🏼👋🏼👋🏼");

        // Clean up
        HakkaRelease(&handle);
    }
}

// GetHakkaStringUTF8Length
TEST_F(HakkaJsonPrimitiveTest, GetHakkaStringUTF8Length)
{
    { // Normal string
        HakkaHandle handle;
        std::string test_str = "Hello, Hakka!";
        ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
        ASSERT_NE(handle, 0);

        uint64_t length = 0;
        ASSERT_EQ(GetHakkaStringUTF8Length(handle, &length), HAKKA_JSON_SUCCESS);
        EXPECT_EQ(length, test_str.size());

        // Clean up
        HakkaRelease(&handle);
    }

    { // Empty string
        HakkaHandle handle;
        std::string test_str = "";
        ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
        ASSERT_NE(handle, 0);

        uint64_t length = 0;
        ASSERT_EQ(GetHakkaStringUTF8Length(handle, &length), HAKKA_JSON_SUCCESS);
        EXPECT_EQ(length, test_str.size());

        // Clean up
        HakkaRelease(&handle);
    }

    { // String with Hindi characters
        HakkaHandle handle;
        std::string test_str = "नमस्ते";
        ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
        ASSERT_NE(handle, 0);

        uint64_t length = 0;
        ASSERT_EQ(GetHakkaStringUTF8Length(handle, &length), HAKKA_JSON_SUCCESS);
        EXPECT_EQ(length, test_str.size());

        // Clean up
        HakkaRelease(&handle);
    }

    { // String with complex emojis
        HakkaHandle handle;
        std::string test_str = "👋🏼👋🏼👋🏼";
        ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
        ASSERT_NE(handle, 0);

        uint64_t length = 0;
        ASSERT_EQ(GetHakkaStringUTF8Length(handle, &length), HAKKA_JSON_SUCCESS);
        EXPECT_EQ(length, test_str.size());

        // Clean up
        HakkaRelease(&handle);
    }
}

// extern_c HakkaJsonResultEnum GetHakkaStringIsprintable(HakkaHandle handle, C_BOOL *result);
// test GetHakkaStringIsprintable, especially ensure "こんにちは世界"
TEST_F(HakkaJsonPrimitiveTest, GetHakkaStringIsprintable)
{
    HakkaHandle handle;
    std::string test_str = "こんにちは世界";
    C_BOOL result = 0;

    ASSERT_EQ(CreateStringHandle(&handle, test_str), HAKKA_JSON_SUCCESS);
    ASSERT_NE(handle, 0);

    ASSERT_EQ(GetHakkaStringIsprintable(handle, &result), HAKKA_JSON_SUCCESS);
    EXPECT_TRUE(result);

    // Clean up
    HakkaRelease(&handle);
}
