#include <hakka_json_int.hpp>
#include <hakka_json_string.hpp>

#include <gtest/gtest.h>
#include <limits>

using namespace hakka;

TEST(JsonInt, Create)
{
    auto json_int = JsonIntCompact::create(42);
    ASSERT_TRUE(json_int);
    ASSERT_EQ(json_int.get_type(), HakkaJsonType::HAKKA_JSON_INT);

    auto view = json_int.get_view();
    auto result = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result, "42");
}

TEST(JsonInt, Compare)
{
    auto json_int1 = JsonIntCompact::create(42);
    auto json_int2 = JsonIntCompact::create(42);
    auto json_int3 = JsonIntCompact::create(100);

    ASSERT_TRUE(json_int1);
    ASSERT_TRUE(json_int2);
    ASSERT_TRUE(json_int3);

    auto view1 = json_int1.get_view();
    auto result = std::visit([&json_int2](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->compare(json_int2);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result, 0);

    result = std::visit([&json_int3](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->compare(json_int3);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view1);
    ASSERT_TRUE(result);
    ASSERT_LT(*result, 0);

    auto view3 = json_int3.get_view();
    result = std::visit([&json_int1](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->compare(json_int1);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view3);
    ASSERT_TRUE(result);
    ASSERT_GT(*result, 0);
}

TEST(JsonInt, ToBytes)
{
    auto json_int = JsonIntCompact::create(42);
    ASSERT_TRUE(json_int);

    char buffer[512];
    uint32_t buffer_size = sizeof(buffer);

    auto view = json_int.get_view();
    auto result = std::visit([&buffer, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->to_bytes(buffer, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);

    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    ASSERT_EQ(buffer_size, 2);
    ASSERT_STREQ(buffer, "42");
}

TEST(JsonInt, DumpSize)
{
    {
        auto json_int = JsonIntCompact::create(42);
        ASSERT_TRUE(json_int);

        auto view = json_int.get_view();
        auto size = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->dump_size();
            }
            return 0;
        }, view);
        ASSERT_EQ(size, 2); // "42".size() = 2
    }
    {
        auto json_int = JsonIntCompact::create(123456);
        ASSERT_TRUE(json_int);

        auto view = json_int.get_view();
        auto size = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->dump_size();
            }
            return 0;
        }, view);
        ASSERT_EQ(size, 6); // "123456".size() = 6
    }
}

TEST(JsonInt, RefCount)
{
    auto json_int = JsonIntCompact::create(42);
    ASSERT_TRUE(json_int);

    auto view = json_int.get_view();
    auto inc_result = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->inc_ref();
        }
        return 0;
    }, view);
    ASSERT_EQ(inc_result, 2);

    auto dec_result = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->dec_ref();
        }
        return 0;
    }, view);
    ASSERT_EQ(dec_result, 1);
}

TEST(JsonInt, Get)
{
    auto json_int = JsonIntCompact::create(42);
    ASSERT_TRUE(json_int);

    auto view = json_int.get_view();
    auto json_handle = std::visit([](auto&& ptr) -> tl::expected<PrimitiveType, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->get();
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);

    ASSERT_TRUE(json_handle);
    ASSERT_EQ(json_handle.value().index(), PrimitiveType{(int64_t)42}.index());
    ASSERT_EQ(std::get<int64_t>(json_handle.value()), 42);
}

TEST(JsonInt, InvalidToBytes)
{
    auto json_int = JsonIntCompact::create(123456);
    ASSERT_TRUE(json_int);

    char buffer[4]; // Insufficient buffer size
    uint32_t buffer_size = sizeof(buffer);

    auto view = json_int.get_view();
    auto result = std::visit([&buffer, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->to_bytes(buffer, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);

    ASSERT_EQ(result, HAKKA_JSON_NOT_ENOUGH_MEMORY);
}

TEST(JsonInt, RefCountAlive)
{
    JsonHandleCompact int_out_handle;
    {
        auto json_handle = JsonIntCompact::create(42); // 1
        ASSERT_TRUE(json_handle);
        int_out_handle = json_handle;         // copy ctor, 1-> 2

        auto view = json_handle.get_view();
        auto inc_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->inc_ref();
            }
            return 0;
        }, view);
        ASSERT_EQ(inc_result, 3); // 2 -> 3
    } // 3 -> 2

    // check if the object is still alive
    ASSERT_TRUE(int_out_handle.get_type() == HakkaJsonType::HAKKA_JSON_INT);

    auto view = int_out_handle.get_view();
    auto get_result = std::visit([](auto&& ptr) -> tl::expected<PrimitiveType, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->get();
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(get_result);
    ASSERT_EQ(std::get<int64_t>(get_result.value()), 42);

    auto dec_result = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->dec_ref();
        }
        return 0;
    }, view);
    ASSERT_EQ(dec_result, 1); // 2 -> 1

    // check if the object is still alive
    ASSERT_TRUE(int_out_handle.get_type() == HakkaJsonType::HAKKA_JSON_INT);

    view = int_out_handle.get_view();
    get_result = std::visit([](auto&& ptr) -> tl::expected<PrimitiveType, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->get();
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(get_result);
    ASSERT_EQ(std::get<int64_t>(get_result.value()), 42);
} // Release the object correctly

TEST(JsonInt, BoundaryValues)
{
    auto json_min = JsonIntCompact::create(std::numeric_limits<int64_t>::min());
    auto json_max = JsonIntCompact::create(std::numeric_limits<int64_t>::max());

    ASSERT_TRUE(json_min);
    ASSERT_TRUE(json_max);

    auto view_min = json_min.get_view();
    auto dump_min = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_min);
    ASSERT_TRUE(dump_min);
    ASSERT_EQ(dump_min.value(), std::to_string(std::numeric_limits<int64_t>::min()));

    auto view_max = json_max.get_view();
    auto dump_max = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_max);
    ASSERT_TRUE(dump_max);
    ASSERT_EQ(dump_max.value(), std::to_string(std::numeric_limits<int64_t>::max()));
}

TEST(JsonInt, InvalidCompare)
{
    auto json_int = JsonIntCompact::create(42);
    auto json_str = JsonStringCompact::create("42");

    ASSERT_TRUE(json_int);
    ASSERT_TRUE(json_str);

    auto view = json_int.get_view();
    auto result = std::visit([&json_str](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->compare(json_str);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);

    ASSERT_FALSE(result); // Expect failure
    ASSERT_EQ(result.error(), HAKKA_JSON_TYPE_ERROR);
}

TEST(JsonInt, EdgeCaseToBytes)
{
    auto json_int = JsonIntCompact::create(42);
    ASSERT_TRUE(json_int);

    auto view = json_int.get_view();

    char buffer_exact[3];
    uint32_t buffer_size = sizeof(buffer_exact);
    auto result = std::visit([&buffer_exact, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->to_bytes(buffer_exact, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    ASSERT_STREQ(buffer_exact, "42");

    char buffer_small[2]; // Too small
    buffer_size = sizeof(buffer_small);
    result = std::visit([&buffer_small, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->to_bytes(buffer_small, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);
    ASSERT_EQ(result, HAKKA_JSON_NOT_ENOUGH_MEMORY);

    char buffer_large[512]; // Excessively large
    buffer_size = sizeof(buffer_large);
    result = std::visit([&buffer_large, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->to_bytes(buffer_large, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    ASSERT_STREQ(buffer_large, "42");
}

TEST(JsonInt, Hash)
{
    // Test that hash returns the value directly (42 -> 42)
    {
        auto json_int = JsonIntCompact::create(42);
        ASSERT_TRUE(json_int);

        auto view = json_int.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);
        ASSERT_EQ(hash_result, 42);
    }

    // Test with zero
    {
        auto json_int = JsonIntCompact::create(0);
        ASSERT_TRUE(json_int);

        auto view = json_int.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);
        ASSERT_EQ(hash_result, 0);
    }

    // Test with negative value (cast to uint64_t)
    {
        auto json_int = JsonIntCompact::create(-42);
        ASSERT_TRUE(json_int);

        auto view = json_int.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);
        ASSERT_EQ(hash_result, static_cast<uint64_t>(-42));
    }

    // Test with large positive value
    {
        auto json_int = JsonIntCompact::create(1000000);
        ASSERT_TRUE(json_int);

        auto view = json_int.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);
        ASSERT_EQ(hash_result, 1000000);
    }

    // Test with max positive value that fits in uint64_t
    {
        auto json_int = JsonIntCompact::create(std::numeric_limits<int64_t>::max());
        ASSERT_TRUE(json_int);

        auto view = json_int.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);
        ASSERT_EQ(hash_result, static_cast<uint64_t>(std::numeric_limits<int64_t>::max()));
    }
}