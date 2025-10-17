#include <hakka_json_float.hpp>

#include <gtest/gtest.h>

using namespace hakka;

TEST(JsonBool, Create)
{
    auto json_bool = JsonFloatCompact::create(true);
    ASSERT_TRUE(json_bool);
    ASSERT_EQ(json_bool.get_type(), HakkaJsonType::HAKKA_JSON_BOOL);

    auto view = json_bool.get_view();
    auto dump_result = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(dump_result);
    ASSERT_EQ(*dump_result, "true");
}

TEST(JsonBool, Compare)
{
    auto json_bool_true = JsonFloatCompact::create(true);
    auto json_bool_false = JsonFloatCompact::create(false);
    auto json_bool_true2 = JsonFloatCompact::create(true);

    ASSERT_TRUE(json_bool_true);
    ASSERT_TRUE(json_bool_false);
    ASSERT_TRUE(json_bool_true2);

    auto view1 = json_bool_true.get_view();
    auto result = std::visit([&json_bool_false](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->compare(json_bool_false);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result, 1);

    result = std::visit([&json_bool_true2](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->compare(json_bool_true2);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result, 0);
}

TEST(JsonBool, ToBytes)
{
    auto json_bool = JsonFloatCompact::create(true);
    ASSERT_TRUE(json_bool);

    char buffer[512];
    uint32_t buffer_size = sizeof(buffer);

    auto view = json_bool.get_view();
    auto result = std::visit([&buffer, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->to_bytes(buffer, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);

    ASSERT_EQ(result, HakkaJsonResultEnum::HAKKA_JSON_SUCCESS);
    ASSERT_EQ(buffer_size, 4);
    ASSERT_STREQ(buffer, "true");
}

TEST(JsonBool, DumpSize)
{
    {
        auto json_bool = JsonFloatCompact::create(true);
        ASSERT_TRUE(json_bool);

        auto view = json_bool.get_view();
        auto size = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
                return reinterpret_cast<const JsonFloatCompact*>(ptr)->dump_size();
            }
            return 0;
        }, view);
        ASSERT_EQ(size, 4); // "true".size() = 4
    }
    {
        auto json_bool = JsonFloatCompact::create(false);
        ASSERT_TRUE(json_bool);

        auto view = json_bool.get_view();
        auto size = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
                return reinterpret_cast<const JsonFloatCompact*>(ptr)->dump_size();
            }
            return 0;
        }, view);
        ASSERT_EQ(size, 5); // "false".size() = 5
    }
}

TEST(JsonBool, RefCount)
{
    auto json_bool = JsonFloatCompact::create(true);
    ASSERT_TRUE(json_bool);

    auto view = json_bool.get_view();
    auto inc_result = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->inc_ref();
        }
        return 0;
    }, view);
    ASSERT_EQ(inc_result, 2);

    auto dec_result = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->dec_ref();
        }
        return 0;
    }, view);
    ASSERT_EQ(dec_result, 1);
}

TEST(JsonBool, Get)
{
    auto json_bool = JsonFloatCompact::create(true);
    ASSERT_TRUE(json_bool);

    auto view = json_bool.get_view();
    auto json_handle = std::visit([](auto&& ptr) -> tl::expected<PrimitiveType, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->get();
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);

    ASSERT_TRUE(json_handle);
    ASSERT_EQ(json_handle.value().index(), PrimitiveType{true}.index());
    ASSERT_EQ(std::get<bool>(json_handle.value()), true);
}

TEST(TestJsonBool, CreateTrue)
{
    auto json_bool = JsonFloatCompact::create(true);
    ASSERT_TRUE(json_bool);
    ASSERT_EQ(json_bool.get_type(), HakkaJsonType::HAKKA_JSON_BOOL);

    auto view = json_bool.get_view();
    auto dump_result = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(dump_result);
    ASSERT_EQ(*dump_result, "true");
}

TEST(TestJsonBool, CreateFalse)
{
    auto json_bool = JsonFloatCompact::create(false);
    ASSERT_TRUE(json_bool);
    ASSERT_EQ(json_bool.get_type(), HakkaJsonType::HAKKA_JSON_BOOL);

    auto view = json_bool.get_view();
    auto dump_result = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(dump_result);
    ASSERT_EQ(*dump_result, "false");
}

TEST(TestJsonBool, CompareTrueFalse)
{
    auto json_bool_true = JsonFloatCompact::create(true);
    auto json_bool_false = JsonFloatCompact::create(false);

    auto view = json_bool_true.get_view();
    auto result = std::visit([&json_bool_false](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->compare(json_bool_false);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result, 1);
}

TEST(TestJsonBool, CompareTrueTrue)
{
    auto json_bool_true1 = JsonFloatCompact::create(true);
    auto json_bool_true2 = JsonFloatCompact::create(true);

    auto view = json_bool_true1.get_view();
    auto result = std::visit([&json_bool_true2](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->compare(json_bool_true2);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result, 0);
}

TEST(TestJsonBool, ToBytesTrue)
{
    auto json_bool = JsonFloatCompact::create(true);
    ASSERT_TRUE(json_bool);

    char buffer[512];
    uint32_t buffer_size = sizeof(buffer);

    auto view = json_bool.get_view();
    auto result = std::visit([&buffer, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->to_bytes(buffer, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);

    ASSERT_EQ(result, HakkaJsonResultEnum::HAKKA_JSON_SUCCESS);
    ASSERT_EQ(buffer_size, 4);
    ASSERT_STREQ(buffer, "true");
}

TEST(TestJsonBool, ToBytesFalse)
{
    auto json_bool = JsonFloatCompact::create(false);
    ASSERT_TRUE(json_bool);

    char buffer[512];
    uint32_t buffer_size = sizeof(buffer);

    auto view = json_bool.get_view();
    auto result = std::visit([&buffer, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->to_bytes(buffer, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);

    ASSERT_EQ(result, HakkaJsonResultEnum::HAKKA_JSON_SUCCESS);
    ASSERT_EQ(buffer_size, 5);
    ASSERT_STREQ(buffer, "false");
}

TEST(TestJsonBool, InvalidToBytes)
{
    auto json_bool = JsonFloatCompact::create(true);
    ASSERT_TRUE(json_bool);

    char buffer[4]; // Insufficient buffer size
    uint32_t buffer_size = sizeof(buffer);

    auto view = json_bool.get_view();
    auto result = std::visit([&buffer, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->to_bytes(buffer, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);

    ASSERT_NE(result, HakkaJsonResultEnum::HAKKA_JSON_SUCCESS);
}

TEST(JsonBool, Hash)
{
    // Test that true returns Python's hash(True) = 1
    {
        auto json_bool = JsonFloatCompact::create(true);
        ASSERT_TRUE(json_bool);

        auto view = json_bool.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
                return reinterpret_cast<const JsonFloatCompact*>(ptr)->hash();
            }
            return 0;
        }, view);

        // Python: hash(True) = 1
        ASSERT_EQ(hash_result, 1);
    }

    // Test that false returns Python's hash(False) = 0
    {
        auto json_bool = JsonFloatCompact::create(false);
        ASSERT_TRUE(json_bool);

        auto view = json_bool.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
                return reinterpret_cast<const JsonFloatCompact*>(ptr)->hash();
            }
            return 0;
        }, view);

        // Python: hash(False) = 0
        ASSERT_EQ(hash_result, 0);
    }
}

TEST(JsonBool, HashConsistency)
{
    // Test that same bool values produce same hash
    {
        auto json_bool1 = JsonFloatCompact::create(true);
        auto json_bool2 = JsonFloatCompact::create(true);
        ASSERT_TRUE(json_bool1);
        ASSERT_TRUE(json_bool2);

        auto view1 = json_bool1.get_view();
        auto hash1 = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
                return reinterpret_cast<const JsonFloatCompact*>(ptr)->hash();
            }
            return 0;
        }, view1);

        auto view2 = json_bool2.get_view();
        auto hash2 = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
                return reinterpret_cast<const JsonFloatCompact*>(ptr)->hash();
            }
            return 0;
        }, view2);

        ASSERT_EQ(hash1, hash2);
    }

    // Test that true and false have different hashes
    {
        auto json_true = JsonFloatCompact::create(true);
        auto json_false = JsonFloatCompact::create(false);
        ASSERT_TRUE(json_true);
        ASSERT_TRUE(json_false);

        auto view_true = json_true.get_view();
        auto hash_true = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
                return reinterpret_cast<const JsonFloatCompact*>(ptr)->hash();
            }
            return 0;
        }, view_true);

        auto view_false = json_false.get_view();
        auto hash_false = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
                return reinterpret_cast<const JsonFloatCompact*>(ptr)->hash();
            }
            return 0;
        }, view_false);

        ASSERT_NE(hash_true, hash_false);
    }
}