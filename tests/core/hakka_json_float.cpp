#include <hakka_json_float.hpp>
#include <hakka_json_string.hpp>

#include <gtest/gtest.h>
#include <limits>
#include <cmath>

using namespace hakka;

TEST(JsonFloat, Create)
{
    auto json_float = JsonFloatCompact::create(42.42);
    ASSERT_TRUE(json_float);
    ASSERT_EQ(json_float.get_type(), HakkaJsonType::HAKKA_JSON_FLOAT);

    // Parse the dumped string and compare as double using ASSERT_NEAR
    auto view = json_float.get_view();
    auto dump_result = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(dump_result);
    double dumped_value = std::stod(dump_result.value());
    ASSERT_NEAR(dumped_value, 42.42, 1e-6);
}

TEST(JsonFloat, Compare)
{
    auto json_float1 = JsonFloatCompact::create(42.42);
    auto json_float2 = JsonFloatCompact::create(42.42);
    auto json_float3 = JsonFloatCompact::create(100.123);

    ASSERT_TRUE(json_float1);
    ASSERT_TRUE(json_float2);
    ASSERT_TRUE(json_float3);

    auto view1 = json_float1.get_view();
    auto result = std::visit([&json_float2](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->compare(json_float2);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result, 0); // Equal

    result = std::visit([&json_float3](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->compare(json_float3);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view1);
    ASSERT_TRUE(result);
    ASSERT_LT(*result, 0); // json_float1 is less than json_float3

    auto view3 = json_float3.get_view();
    result = std::visit([&json_float1](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->compare(json_float1);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view3);
    ASSERT_TRUE(result);
    ASSERT_GT(*result, 0); // json_float3 is greater than json_float1
}

TEST(JsonFloat, ToBytes)
{
    auto json_float = JsonFloatCompact::create(42.42);
    ASSERT_TRUE(json_float);

    char buffer[512];
    uint32_t buffer_size = sizeof(buffer);

    auto view = json_float.get_view();
    auto result = std::visit([&buffer, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->to_bytes(buffer, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    ASSERT_GT(buffer_size, 0);

    // Parse the buffer and compare using ASSERT_NEAR
    double buffer_value = std::stod(buffer);
    ASSERT_NEAR(buffer_value, 42.42, 1e-6);
}

TEST(JsonFloat, DumpSize)
{
    {
        auto json_float = JsonFloatCompact::create(42.42);
        ASSERT_TRUE(json_float);

        auto view = json_float.get_view();
        auto size = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->dump_size();
            }
            return 0;
        }, view);
        ASSERT_EQ(size, 5); // "42.42".size() = 5
    }

    // floating point have some format issue, the size might more than expected
    {
        auto json_float = JsonFloatCompact::create(std::numeric_limits<double>::max());
        ASSERT_TRUE(json_float);

        auto view = json_float.get_view();
        auto size = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->dump_size();
            }
            return 0;
        }, view);
        ASSERT_GE(size, 1); // At least 1 character
    }
}

TEST(JsonFloat, RefCount)
{
    auto json_float = JsonFloatCompact::create(42.42);
    ASSERT_TRUE(json_float);

    auto view = json_float.get_view();
    auto inc_result = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->inc_ref();
        }
        return 0;
    }, view);
    ASSERT_EQ(inc_result, 2); // Increment reference count

    auto dec_result = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dec_ref();
        }
        return 0;
    }, view);
    ASSERT_EQ(dec_result, 1); // Decrement reference count
}

TEST(JsonFloat, Get)
{
    auto json_float = JsonFloatCompact::create(42.42);
    ASSERT_TRUE(json_float);

    auto view = json_float.get_view();
    auto json_handle = std::visit([](auto&& ptr) -> tl::expected<PrimitiveType, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->get();
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(json_handle);

    // Compare using ASSERT_NEAR
    ASSERT_EQ(json_handle.value().index(), PrimitiveType{42.42}.index());
    ASSERT_NEAR(std::get<double>(json_handle.value()), 42.42, 1e-6);
}

TEST(JsonFloat, InvalidToBytes)
{
    auto json_float = JsonFloatCompact::create(123456.789);
    ASSERT_TRUE(json_float);

    char buffer[8]; // Insufficient buffer size
    uint32_t buffer_size = sizeof(buffer);

    auto view = json_float.get_view();
    auto result = std::visit([&buffer, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->to_bytes(buffer, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
}

TEST(JsonFloat, RefCountAlive)
{
    JsonHandleCompact float_out_handle;
    {
        auto json_handle = JsonFloatCompact::create(42.42); // Ref count = 1
        ASSERT_TRUE(json_handle);
        float_out_handle = json_handle;       // Copy constructor, Ref count = 2

        auto view = json_handle.get_view();
        auto inc_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->inc_ref();
            }
            return 0;
        }, view);
        ASSERT_EQ(inc_result, 3); // Ref count = 3
    } // Ref count decrements to 2

    // Check if the object is still alive
    ASSERT_TRUE(float_out_handle.get_type() == HakkaJsonType::HAKKA_JSON_FLOAT);

    auto view = float_out_handle.get_view();
    auto get_result = std::visit([](auto&& ptr) -> tl::expected<PrimitiveType, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->get();
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(get_result);
    ASSERT_NEAR(std::get<double>(get_result.value()), 42.42, 1e-6);

    auto dec_result = std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dec_ref();
        }
        return 0;
    }, view);
    ASSERT_EQ(dec_result, 1); // Ref count = 1

    // Check if the object is still alive
    ASSERT_TRUE(float_out_handle.get_type() == HakkaJsonType::HAKKA_JSON_FLOAT);

    view = float_out_handle.get_view();
    get_result = std::visit([](auto&& ptr) -> tl::expected<PrimitiveType, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->get();
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_TRUE(get_result);
    ASSERT_NEAR(std::get<double>(get_result.value()), 42.42, 1e-6);
} // Ref count decrements to 0, object released

TEST(JsonFloat, BoundaryValues)
{
    auto json_min = JsonFloatCompact::create(-std::numeric_limits<double>::max());
    auto json_max = JsonFloatCompact::create(std::numeric_limits<double>::max());
    auto json_zero = JsonFloatCompact::create(0.0);
    auto json_neg_zero = JsonFloatCompact::create(-0.0);
    auto json_inf = JsonFloatCompact::create(std::numeric_limits<double>::infinity());
    auto json_neg_inf = JsonFloatCompact::create(-std::numeric_limits<double>::infinity());
    auto json_nan = JsonFloatCompact::create(std::numeric_limits<double>::quiet_NaN());

    ASSERT_TRUE(json_min);
    ASSERT_TRUE(json_max);
    ASSERT_TRUE(json_zero);
    ASSERT_TRUE(json_neg_zero);
    ASSERT_TRUE(json_inf);
    ASSERT_TRUE(json_neg_inf);
    ASSERT_TRUE(json_nan);

    // Test dump for min and max
    // %g format specifier is used to avoid scientific notation
    auto view_min = json_min.get_view();
    auto dump_min = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_min);
    ASSERT_TRUE(dump_min);
    ASSERT_EQ(dump_min.value(), "-1.79769e+308");

    auto view_max = json_max.get_view();
    auto dump_max = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_max);
    ASSERT_TRUE(dump_max);
    ASSERT_EQ(dump_max.value(), "1.79769e+308");

    auto view_zero = json_zero.get_view();
    auto dump_zero = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_zero);
    ASSERT_TRUE(dump_zero);
    ASSERT_EQ(dump_zero.value(), "0");

    // ASSERT_EQ(json_neg_zero->dump(512).value(), "-0"); // Negative zero is not supported

    auto view_inf = json_inf.get_view();
    auto dump_inf = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_inf);
    ASSERT_TRUE(dump_inf);
    ASSERT_NE(dump_inf.value().find("inf"), std::string::npos);

    auto view_neg_inf = json_neg_inf.get_view();
    auto dump_neg_inf = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_neg_inf);
    ASSERT_TRUE(dump_neg_inf);
    ASSERT_NE(dump_neg_inf.value().find("-inf"), std::string::npos);

    auto view_nan = json_nan.get_view();
    auto dump_nan = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_nan);
    ASSERT_TRUE(dump_nan);
    ASSERT_TRUE(std::isnan(std::stod(dump_nan.value())));
}

TEST(JsonFloat, InvalidCompare)
{
    auto json_float = JsonFloatCompact::create(42.42);
    auto json_str = JsonStringCompact::create("42.42");

    ASSERT_TRUE(json_float);
    ASSERT_TRUE(json_str);

    auto view = json_float.get_view();
    auto result = std::visit([&json_str](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->compare(json_str);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view);
    ASSERT_FALSE(result); // Expect failure
    ASSERT_EQ(result.error(), HAKKA_JSON_TYPE_ERROR);
}

TEST(JsonFloat, EdgeCaseToBytes)
{
    auto json_float = JsonFloatCompact::create(42.42);
    ASSERT_TRUE(json_float);

    auto view = json_float.get_view();

    // Exact buffer size
    std::string float_str = "42.42";
    char buffer_exact[8];
    uint32_t buffer_size = sizeof(buffer_exact);
    auto result = std::visit([&buffer_exact, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->to_bytes(buffer_exact, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    ASSERT_STREQ(buffer_exact, "42.42");
    ASSERT_EQ(buffer_size, float_str.size());

    // Buffer too small
    char buffer_small[4]; // Too small for "42.42"
    buffer_size = sizeof(buffer_small);
    result = std::visit([&buffer_small, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->to_bytes(buffer_small, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);
    ASSERT_EQ(result, HAKKA_JSON_NOT_ENOUGH_MEMORY);

    // Excessively large buffer
    char buffer_large[512];
    buffer_size = sizeof(buffer_large);
    result = std::visit([&buffer_large, &buffer_size](auto&& ptr) -> HakkaJsonResultEnum {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->to_bytes(buffer_large, &buffer_size);
        }
        return HAKKA_JSON_TYPE_ERROR;
    }, view);
    ASSERT_EQ(result, HAKKA_JSON_SUCCESS);
    ASSERT_STREQ(buffer_large, "42.42");
    ASSERT_EQ(buffer_size, float_str.size());
}

TEST(JsonFloat, SpecialValues)
{
    // Test Infinity
    auto json_inf = JsonFloatCompact::create(std::numeric_limits<double>::infinity());
    ASSERT_TRUE(json_inf);

    auto view_inf = json_inf.get_view();
    auto dump_inf = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_inf);
    ASSERT_TRUE(dump_inf);
    ASSERT_NE(dump_inf.value().find("inf"), std::string::npos);

    // Test -Infinity
    auto json_neg_inf = JsonFloatCompact::create(-std::numeric_limits<double>::infinity());
    ASSERT_TRUE(json_neg_inf);

    auto view_neg_inf = json_neg_inf.get_view();
    auto dump_neg_inf = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_neg_inf);
    ASSERT_TRUE(dump_neg_inf);
    ASSERT_NE(dump_neg_inf.value().find("-inf"), std::string::npos);

    // Test NaN
    auto json_nan = JsonFloatCompact::create(std::numeric_limits<double>::quiet_NaN());
    ASSERT_TRUE(json_nan);

    auto view_nan = json_nan.get_view();
    auto dump_nan = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_nan);
    ASSERT_TRUE(dump_nan);
    ASSERT_NE(dump_nan.value().find("nan"), std::string::npos);
}

TEST(JsonFloat, NaNHandling)
{
    // Test positive NaN (quiet_NaN)
    auto json_pos_nan = JsonFloatCompact::create(std::numeric_limits<double>::quiet_NaN());
    ASSERT_TRUE(json_pos_nan);
    ASSERT_EQ(json_pos_nan.get_type(), HakkaJsonType::HAKKA_JSON_FLOAT);

    auto view_pos_nan = json_pos_nan.get_view();
    auto dump_pos_nan = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_pos_nan);
    ASSERT_TRUE(dump_pos_nan);
    ASSERT_NE(dump_pos_nan.value().find("nan"), std::string::npos);

    auto get_pos_nan = std::visit([](auto&& ptr) -> tl::expected<PrimitiveType, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->get();
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_pos_nan);
    ASSERT_TRUE(get_pos_nan);
    ASSERT_TRUE(std::isnan(std::get<double>(get_pos_nan.value())));

    // Test negative NaN (-quiet_NaN)
    auto json_neg_nan = JsonFloatCompact::create(-std::numeric_limits<double>::quiet_NaN());
    ASSERT_TRUE(json_neg_nan);
    ASSERT_EQ(json_neg_nan.get_type(), HakkaJsonType::HAKKA_JSON_FLOAT);

    auto view_neg_nan = json_neg_nan.get_view();
    auto dump_neg_nan = std::visit([](auto&& ptr) -> tl::expected<std::string, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->dump(512);
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_neg_nan);
    ASSERT_TRUE(dump_neg_nan);
    ASSERT_NE(dump_neg_nan.value().find("nan"), std::string::npos);

    auto get_neg_nan = std::visit([](auto&& ptr) -> tl::expected<PrimitiveType, HakkaJsonResultEnum> {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->get();
        }
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }, view_neg_nan);
    ASSERT_TRUE(get_neg_nan);
    ASSERT_TRUE(std::isnan(std::get<double>(get_neg_nan.value())));

    // Both should be valid floats and both should output strings containing "nan"
    ASSERT_TRUE(dump_pos_nan);
    ASSERT_TRUE(dump_neg_nan);
}

TEST(JsonFloat, Interning)
{
    auto json_float1 = JsonFloatCompact::create(42.42);
    ASSERT_TRUE(json_float1);

    auto json_float2 = JsonFloatCompact::create(42.42);
    ASSERT_TRUE(json_float2);

    // The underlying tokens should be identical for interned values
    ASSERT_EQ(static_cast<uint64_t>(json_float1), static_cast<uint64_t>(json_float2));
}

TEST(JsonFloat, PerformanceTest)
{
    const int iterations = 1000000;
    auto json_float = JsonFloatCompact::create(42.42);
    ASSERT_TRUE(json_float);

    auto view = json_float.get_view();
    for (int i = 0; i < iterations; ++i)
    {
        auto result = std::visit([&json_float](auto&& ptr) -> tl::expected<int, HakkaJsonResultEnum> {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->compare(json_float);
            }
            return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
        }, view);
        ASSERT_TRUE(result);
        ASSERT_EQ(*result, 0);
    }
}
