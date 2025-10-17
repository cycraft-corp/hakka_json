#include <hakka_json_float.hpp>
#include <hakka_json_int.hpp>
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

TEST(JsonFloat, Hash)
{
    // Test basic hash functionality
    {
        auto json_float = JsonFloatCompact::create(42.5);
        ASSERT_TRUE(json_float);

        auto view = json_float.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);
        // Just verify hash is computed (non-zero for non-zero value)
        ASSERT_NE(hash_result, 0);
    }

    // Test hash consistency: same value should produce same hash
    {
        auto json_float1 = JsonFloatCompact::create(42.5);
        auto json_float2 = JsonFloatCompact::create(42.5);
        ASSERT_TRUE(json_float1);
        ASSERT_TRUE(json_float2);

        auto view1 = json_float1.get_view();
        auto hash1 = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view1);

        auto view2 = json_float2.get_view();
        auto hash2 = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view2);

        ASSERT_EQ(hash1, hash2);
    }

    // Test zero hash
    {
        auto json_float = JsonFloatCompact::create(0.0);
        ASSERT_TRUE(json_float);

        auto view = json_float.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);
        ASSERT_EQ(hash_result, 0);
    }

    // Test negative value
    {
        auto json_float = JsonFloatCompact::create(-42.5);
        ASSERT_TRUE(json_float);

        auto view = json_float.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);
        ASSERT_NE(hash_result, 0);
    }

    // Test infinity hashes
    {
        auto json_inf = JsonFloatCompact::create(std::numeric_limits<double>::infinity());
        ASSERT_TRUE(json_inf);

        auto view = json_inf.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);
        ASSERT_EQ(hash_result, 314159); // _PyHASH_INF
    }

    {
        auto json_neg_inf = JsonFloatCompact::create(-std::numeric_limits<double>::infinity());
        ASSERT_TRUE(json_neg_inf);

        auto view = json_neg_inf.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);
        ASSERT_EQ(hash_result, static_cast<uint64_t>(-314159)); // -_PyHASH_INF
    }

    // Test NaN hash uses pointer-like hash (rotated bits)
    {
        auto json_nan = JsonFloatCompact::create(std::numeric_limits<double>::quiet_NaN());
        ASSERT_TRUE(json_nan);

        auto view = json_nan.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);

        // Verify pointer-like hash is applied: get the bit representation
        double nan_val = std::numeric_limits<double>::quiet_NaN();
        uint64_t nan_bits = std::bit_cast<uint64_t>(nan_val);

        // Compute expected pointer-like hash (rotate right by 4)
        uint64_t expected = (nan_bits >> 4) | (nan_bits << 60);
        if (expected == static_cast<uint64_t>(-1)) {
            expected = static_cast<uint64_t>(-2);
        }

        ASSERT_EQ(hash_result, expected);
    }

    // Test that different NaN bit patterns produce different hashes
    {
        // Create NaN with different bit patterns using bit_cast
        uint64_t nan_bits1 = 0x7FF8000000000001ull; // Quiet NaN with payload
        uint64_t nan_bits2 = 0x7FF8000000000002ull; // Different quiet NaN

        double nan1 = std::bit_cast<double>(nan_bits1);
        double nan2 = std::bit_cast<double>(nan_bits2);

        // Both should be NaN
        ASSERT_TRUE(std::isnan(nan1));
        ASSERT_TRUE(std::isnan(nan2));

        // Get their hashes via free_hash
        uint64_t hash1 = JsonFloatCompact::free_hash(nan1);
        uint64_t hash2 = JsonFloatCompact::free_hash(nan2);

        // Since they have different bit patterns, pointer-like hash should differ
        ASSERT_NE(hash1, hash2);
    }

    // Test large value
    {
        auto json_float = JsonFloatCompact::create(1e100);
        ASSERT_TRUE(json_float);

        auto view = json_float.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);
        ASSERT_NE(hash_result, 0);
    }

    // Test small value
    {
        auto json_float = JsonFloatCompact::create(1e-100);
        ASSERT_TRUE(json_float);

        auto view = json_float.get_view();
        auto hash_result = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view);
        ASSERT_NE(hash_result, 0);
    }
}

TEST(JsonFloat, HashNumericEquality)
{
    // Test that hash(3.0) == hash(3) (Python-compatible behavior)
    // This is the key property of Python's hash algorithm for numeric types
    {
        auto json_float = JsonFloatCompact::create(3.0);
        auto json_int = JsonIntCompact::create(3);
        ASSERT_TRUE(json_float);
        ASSERT_TRUE(json_int);

        auto view_float = json_float.get_view();
        auto hash_float = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view_float);

        auto view_int = json_int.get_view();
        auto hash_int = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view_int);

        ASSERT_EQ(hash_float, hash_int);
        ASSERT_EQ(hash_float, 3); // Direct value mapping for integers
    }

    // Test with larger value
    {
        auto json_float = JsonFloatCompact::create(42.0);
        auto json_int = JsonIntCompact::create(42);
        ASSERT_TRUE(json_float);
        ASSERT_TRUE(json_int);

        auto view_float = json_float.get_view();
        auto hash_float = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view_float);

        auto view_int = json_int.get_view();
        auto hash_int = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view_int);

        ASSERT_EQ(hash_float, hash_int);
        ASSERT_EQ(hash_float, 42);
    }

    // Test with zero
    {
        auto json_float = JsonFloatCompact::create(0.0);
        auto json_int = JsonIntCompact::create(0);
        ASSERT_TRUE(json_float);
        ASSERT_TRUE(json_int);

        auto view_float = json_float.get_view();
        auto hash_float = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view_float);

        auto view_int = json_int.get_view();
        auto hash_int = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view_int);

        ASSERT_EQ(hash_float, hash_int);
        ASSERT_EQ(hash_float, 0);
    }

    // Test with negative value
    {
        auto json_float = JsonFloatCompact::create(-42.0);
        auto json_int = JsonIntCompact::create(-42);
        ASSERT_TRUE(json_float);
        ASSERT_TRUE(json_int);

        auto view_float = json_float.get_view();
        auto hash_float = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view_float);

        auto view_int = json_int.get_view();
        auto hash_int = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view_int);

        ASSERT_EQ(hash_float, hash_int);
        ASSERT_EQ(hash_float, static_cast<uint64_t>(-42));
    }

    // Test that non-integer floats have different hashes
    {
        auto json_float = JsonFloatCompact::create(3.5);
        auto json_int = JsonIntCompact::create(3);
        ASSERT_TRUE(json_float);
        ASSERT_TRUE(json_int);

        auto view_float = json_float.get_view();
        auto hash_float = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view_float);

        auto view_int = json_int.get_view();
        auto hash_int = std::visit([](auto&& ptr) -> uint64_t {
            using T = std::decay_t<decltype(ptr)>;
            if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
                return ptr->hash();
            }
            return 0;
        }, view_int);

        ASSERT_NE(hash_float, hash_int);
    }
}
