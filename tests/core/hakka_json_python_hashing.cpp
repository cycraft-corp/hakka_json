#include <hakka_json_float.hpp>
#include <hakka_json_int.hpp>
#include <gtest/gtest.h>

using namespace hakka;

// Helper to get hash from a handle, handling bool types
uint64_t get_hash(const JsonHandleCompact& handle) {
    auto view = handle.get_view();
    return std::visit([](auto&& ptr) -> uint64_t {
        using T = std::decay_t<decltype(ptr)>;
        if constexpr (std::is_same_v<T, const JsonFloatCompact*>) {
            return ptr->hash();
        }
        else if constexpr (std::is_same_v<T, const JsonBoolCompact*>) {
            // JsonBoolCompact is an alias for JsonFloatCompact in the NaN-boxing implementation
            // TRUE and FALSE are represented as special NaN values in JsonFloatCompact
            // This reinterpret_cast is safe because they're the same underlying type
            return reinterpret_cast<const JsonFloatCompact*>(ptr)->hash();
        }
        else if constexpr (std::is_same_v<T, const JsonIntCompact*>) {
            return ptr->hash();
        }
        // Should never reach here for scalar types
        assert(false && "Unexpected type in get_hash");
        return static_cast<uint64_t>(-1); // Distinctive value to catch issues
    }, view);
}

// Test that hash values are Python-compatible
TEST(PythonHashing, IntegerFloatHashEquality)
{
    // Test that hash(0.0) == hash(0)
    auto float_zero = JsonFloatCompact::create(0.0);
    auto int_zero = JsonIntCompact::create(0);
    
    ASSERT_EQ(get_hash(float_zero), get_hash(int_zero)) << "hash(0.0) should equal hash(0)";
    
    // Test that hash(1.0) == hash(1)
    auto float_one = JsonFloatCompact::create(1.0);
    auto int_one = JsonIntCompact::create(1);
    
    ASSERT_EQ(get_hash(float_one), get_hash(int_one)) << "hash(1.0) should equal hash(1)";
}

TEST(PythonHashing, BoolHashEquality)
{
    // Test that hash(False) == hash(0)
    auto json_false = JsonFloatCompact::create(false);
    auto int_zero = JsonIntCompact::create(0);
    
    ASSERT_EQ(get_hash(json_false), get_hash(int_zero)) << "hash(False) should equal hash(0)";
    
    // Test that hash(True) == hash(1)
    auto json_true = JsonFloatCompact::create(true);
    auto int_one = JsonIntCompact::create(1);
    
    ASSERT_EQ(get_hash(json_true), get_hash(int_one)) << "hash(True) should equal hash(1)";
}

TEST(PythonHashing, DistinctValues)
{
    // Test that 0.0 and False are distinct values even though they hash the same
    auto float_zero = JsonFloatCompact::create(0.0);
    auto json_false = JsonFloatCompact::create(false);
    
    // They should have different types
    ASSERT_EQ(float_zero.get_type(), HakkaJsonType::HAKKA_JSON_FLOAT);
    ASSERT_EQ(json_false.get_type(), HakkaJsonType::HAKKA_JSON_BOOL);
    
    // They should have the same hash (Python-compatible)
    ASSERT_EQ(get_hash(float_zero), get_hash(json_false)) << "hash(0.0) should equal hash(False)";
    
    // But they should be distinct objects (different tokens)
    ASSERT_NE(static_cast<uint64_t>(float_zero), static_cast<uint64_t>(json_false)) 
        << "0.0 and False should be distinct objects";
}
