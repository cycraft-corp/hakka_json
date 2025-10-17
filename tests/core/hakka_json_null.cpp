#include <hakka_json_float.hpp>

#include <gtest/gtest.h>

using namespace hakka;

TEST(JsonNull, Create)
{
    auto json_null = JsonFloatCompact::create(nullptr);
    ASSERT_TRUE(json_null);
    ASSERT_EQ(json_null.get_type(), HakkaJsonType::HAKKA_JSON_NULL);

    auto view = json_null.get_view();
    auto dump_result = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonNullCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(dump_result);
    ASSERT_EQ(*dump_result, "null");
}

TEST(JsonNull, ToBytes)
{
    auto json_null = JsonFloatCompact::create(nullptr);
    ASSERT_TRUE(json_null);

    char buffer[512];
    uint32_t buffer_size = sizeof(buffer);

    auto view = json_null.get_view();
    auto result = std::visit([&buffer, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonNullCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->to_bytes(buffer, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);

    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    ASSERT_EQ(buffer_size, 4); // "NULL" + null terminator
    ASSERT_STREQ(buffer, "null");
}

TEST(JsonNull, DumpSize)
{
    auto json_null = JsonFloatCompact::create(nullptr);
    ASSERT_TRUE(json_null);

    auto view = json_null.get_view();
    auto size = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonNullCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->dump_size();
        }
        return 0;
    }, view);
    ASSERT_EQ(size, 4); // "null".size() = 4
}

TEST(JsonNull, InvalidToBytes)
{
    auto json_null = JsonFloatCompact::create(nullptr);
    ASSERT_TRUE(json_null);

    char buffer[3]; // Insufficient buffer size
    uint32_t buffer_size = sizeof(buffer);

    auto view = json_null.get_view();
    auto result = std::visit([&buffer, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonNullCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->to_bytes(buffer, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);

    ASSERT_EQ(result, HAKKA_JSON_NOT_ENOUGH_MEMORY);
    ASSERT_EQ(buffer_size, 5); // Required size for "NULL" + null terminator
}

TEST(JsonNull, Compare)
{
    auto json_null1 = JsonFloatCompact::create(nullptr);
    auto json_null2 = JsonFloatCompact::create(nullptr);

    ASSERT_TRUE(json_null1);
    ASSERT_TRUE(json_null2);

    // Compare with another JsonNull
    auto view1 = json_null1.get_view();
    auto result = std::visit([&json_null2](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonNullCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->compare(json_null2);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view1);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(*result, 0); // NULL == NULL

    // Compare with invalid type
    auto json_invalid = JsonFloatCompact::create();
    result = std::visit([&json_invalid](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonNullCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->compare(json_invalid);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view1);
    ASSERT_FALSE(result.has_value()); // no way to compare with invalid type
}

TEST(JsonNull, Get)
{
    auto json_null = JsonFloatCompact::create(nullptr);
    ASSERT_TRUE(json_null);

    auto view = json_null.get_view();
    auto json_handle = std::visit([](auto&& ptr) -> tl::expected<PrimitiveType, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonNullCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->get();
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);

    ASSERT_TRUE(json_handle);
    ASSERT_EQ(json_handle.value().index(), PrimitiveType{nullptr}.index());
    ASSERT_EQ(std::get<std::nullptr_t>(json_handle.value()), nullptr);
}

TEST(JsonNull, Hash)
{
    // Test that null uses Python's hash(None) (pointer-like hash on fixed address)
    auto json_null = JsonFloatCompact::create(nullptr);
    ASSERT_TRUE(json_null);

    auto view = json_null.get_view();
    auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonNullCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->hash();
        }
        return 0;
    }, view);

    // NULL_NAN is 0xFFF8000000000000
    // Python uses pointer-like hash on None singleton: _Py_HashPointer(&_Py_NoneStruct)
    // We compute the expected pointer-like hash
    uint64_t null_nan_bits = 0xFFF8000000000000ull;
    uint64_t expected = (null_nan_bits >> 4) | (null_nan_bits << 60);
    if (expected == static_cast<uint64_t>(-1)) {
        expected = static_cast<uint64_t>(-2);
    }

    ASSERT_EQ(hash_result, expected);
}

TEST(JsonNull, HashConsistency)
{
    // Test that same null values produce same hash
    auto json_null1 = JsonFloatCompact::create(nullptr);
    auto json_null2 = JsonFloatCompact::create(nullptr);
    ASSERT_TRUE(json_null1);
    ASSERT_TRUE(json_null2);

    auto view1 = json_null1.get_view();
    auto hash1 = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonNullCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->hash();
        }
        return 0;
    }, view1);

    auto view2 = json_null2.get_view();
    auto hash2 = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonNullCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->hash();
        }
        return 0;
    }, view2);

    ASSERT_EQ(hash1, hash2);
}
