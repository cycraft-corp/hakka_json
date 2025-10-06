#include <hakka_json_float.hpp>

#include <gtest/gtest.h>

using namespace hakka;

TEST(JsonInvalid, Create)
{
    auto json_invalid = JsonFloatCompact::create();
    ASSERT_FALSE(json_invalid);
    ASSERT_EQ(json_invalid.get_type(), HakkaJsonType::HAKKA_JSON_INVALID);

    auto view = json_invalid.get_view();
    auto dump_result = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonInvalidCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(dump_result);
    ASSERT_EQ(*dump_result, "INVALID");
}

TEST(JsonInvalid, ToBytes)
{
    auto json_invalid = JsonFloatCompact::create();
    ASSERT_FALSE(json_invalid);

    char buffer[512];
    uint32_t buffer_size = sizeof(buffer);

    auto view = json_invalid.get_view();
    auto result = std::visit([&buffer, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonInvalidCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->to_bytes(buffer, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);

    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    ASSERT_EQ(buffer_size, 7); // "INVALID" + null terminator
    ASSERT_STREQ(buffer, "INVALID");
}

TEST(JsonInvalid, DumpSize)
{
    auto json_invalid = JsonFloatCompact::create();
    ASSERT_FALSE(json_invalid);

    auto view = json_invalid.get_view();
    auto size = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonInvalidCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->dump_size();
        }
        return 0;
    }, view);
    ASSERT_EQ(size, 7); // "INVALID".size() = 7
}

TEST(JsonInvalid, InvalidToBytes)
{
    auto json_invalid = JsonFloatCompact::create();
    ASSERT_FALSE(json_invalid);

    char buffer[6]; // Insufficient buffer size
    uint32_t buffer_size = sizeof(buffer);

    auto view = json_invalid.get_view();
    auto result = std::visit([&buffer, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonInvalidCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->to_bytes(buffer, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);

    ASSERT_EQ(result, HAKKA_JSON_NOT_ENOUGH_MEMORY);
    ASSERT_EQ(buffer_size, 8); // Required size for "INVALID" + null terminator
}

TEST(JsonInvalid, Compare)
{
    auto json_invalid1 = JsonFloatCompact::create();
    auto json_invalid2 = JsonFloatCompact::create();

    ASSERT_FALSE(json_invalid1);
    ASSERT_FALSE(json_invalid2);

    // Compare with another JsonInvalid
    auto view1 = json_invalid1.get_view();
    auto result = std::visit([&json_invalid2](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonInvalidCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->compare(json_invalid2);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view1);
    ASSERT_FALSE(result.has_value()); // Expect failure
}
