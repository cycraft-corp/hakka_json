#include <hakka_json_float.hpp>
#include <hakka_compare.hpp>
#include <handles/scalar_manager.hpp>
#include <handles/strict_fp_block.hpp>

#include <bit>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>

using namespace hakka;

inline static bool is_exact_nan_value(double val, double target)
{
    // Use bitwise comparison to check for exact match
    uint64_t val_bits = std::bit_cast<uint64_t>(val);
    uint64_t target_bits = std::bit_cast<uint64_t>(target);
    return val_bits == target_bits;
}

JsonFloatCompact::JsonFloatCompact(ValueType value) : JsonPrimitiveCompact(value)
{
}

JsonFloatCompact::~JsonFloatCompact() { dec_ref(); }

JsonHandleCompact JsonFloatCompact::create(ValueType value) {
    // Normalize NaN values to avoid collision with special NaN-boxed values (NULL, TRUE, FALSE, INVALID)
    // If the value is NaN and matches one of our special NaN patterns, convert it to a canonical positive NaN
    if (std::isnan(value))
    {
        // Static const to compute the bit patterns only once (not constexpr since NaN constants are constinit)
        static const uint64_t null_nan_bits = std::bit_cast<uint64_t>(NULL_NAN);
        static const uint64_t true_nan_bits = std::bit_cast<uint64_t>(TRUE_NAN);
        static const uint64_t false_nan_bits = std::bit_cast<uint64_t>(FALSE_NAN);
        static const uint64_t invalid_nan_bits = std::bit_cast<uint64_t>(INVALID_NAN);
        
        uint64_t bits = std::bit_cast<uint64_t>(value);
        
        // If the NaN value collides with any special NaN, normalize it to a canonical positive NaN
        if (bits == null_nan_bits || bits == true_nan_bits || bits == false_nan_bits || bits == invalid_nan_bits)
        {
            // Use the standard positive quiet NaN (0x7ff8000000000000) which doesn't collide
            value = std::numeric_limits<double>::quiet_NaN();
        }
    }
    
    return JsonHandleCompact(ScalarManagerCompact::get_instance().create(std::move(value)));
}

JsonHandleCompact JsonFloatCompact::create(bool value) {
    return JsonHandleCompact(ScalarManagerCompact::get_instance().create(std::move(value)));
}

JsonHandleCompact JsonFloatCompact::create(std::nullptr_t value) {
    return JsonHandleCompact(ScalarManagerCompact::get_instance().create(std::move(value)));
}

std::unique_ptr<JsonFloatCompact> JsonFloatCompact::create_unique(ValueType value) {
    return std::unique_ptr<JsonFloatCompact>(new (std::nothrow) JsonFloatCompact(value));
}

JsonHandleCompact JsonFloatCompact::create() {
    return JsonHandleCompact(0); // for invalid
}

uint64_t JsonFloatCompact::inc_ref_impl() const {
    return ref_count.fetch_add(1, std::memory_order_relaxed) + 1;
}

uint64_t JsonFloatCompact::dec_ref_impl() const {
    return ref_count.fetch_sub(1, std::memory_order_relaxed) - 1;
}

tl::expected<std::string, HakkaJsonResultEnum> JsonFloatCompact::dump_impl([[maybe_unused]] uint32_t) const {
    try {
        const auto& type_enum = type();
        if (type_enum == HAKKA_JSON_NULL)
        {
            return std::string("null");
        }
        else if (type_enum == HAKKA_JSON_BOOL)
        {
            if (is_exact_nan_value(value_, TRUE_NAN))
            {
                return std::string("true");
            }
            else
            {
                return std::string("false");
            }
        }
        else if (type_enum == HAKKA_JSON_INVALID)
        {
            return std::string("INVALID");
        }
        else
        { // this is the normal float
            char buffer[128];
            std::snprintf(buffer, sizeof(buffer), "%g", value_);
            return std::string(buffer);
        }
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

HakkaJsonResultEnum JsonFloatCompact::to_bytes_impl(char *buffer, uint32_t *buffer_size) const {
    try {
        std::string float_str = dump(1).value();
        uint32_t required_size = static_cast<uint32_t>(float_str.size()) + 1; // +1 for null terminator

        if (*buffer_size < required_size) {
            *buffer_size = required_size;
            return HAKKA_JSON_NOT_ENOUGH_MEMORY;
        }

        std::memcpy(buffer, float_str.c_str(), required_size);
        *buffer_size = static_cast<uint32_t>(float_str.size());
        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

HakkaJsonType JsonFloatCompact::type_impl() const {
    // get the value.
    double value = value_;
    if (is_exact_nan_value(value, NULL_NAN))
    {
        return HAKKA_JSON_NULL;
    }
    else if (is_exact_nan_value(value, TRUE_NAN))
    {
        return HAKKA_JSON_BOOL;
    }
    else if (is_exact_nan_value(value, FALSE_NAN))
    {
        return HAKKA_JSON_BOOL;
    }
    else if (is_exact_nan_value(value, INVALID_NAN))
    {
        return HAKKA_JSON_INVALID;
    }
    return HAKKA_JSON_FLOAT;
}

tl::expected<int, HakkaJsonResultEnum> JsonFloatCompact::compare_impl(const JsonHandleCompact &other) const {
    if (other.get_type() != HAKKA_JSON_INT &&
        other.get_type() != HAKKA_JSON_FLOAT &&
        other.get_type() != HAKKA_JSON_BOOL &&
        other.get_type() != HAKKA_JSON_NULL)
    {
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }

    if (type() == HAKKA_JSON_NULL) {
        return hakka::compare(UniformCompactPointerView((const JsonNullCompact*)this), other.get_view(), 0);
    }
    else if (type() == HAKKA_JSON_BOOL) {
        return hakka::compare(UniformCompactPointerView((const JsonBoolCompact*)this), other.get_view(), 0);
    }
    else if (type() == HAKKA_JSON_FLOAT) {
        return hakka::compare(UniformCompactPointerView(this), other.get_view(), 0);
    }
    else {
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }
}


uint64_t JsonFloatCompact::hash_impl() const
{
    return free_hash(value_);
}

uint64_t JsonFloatCompact::dump_size_impl() const
{
    return dump(0).value_or("").size();
}

uint64_t JsonFloatCompact::free_hash(double value)
{
    // Check if this is a NaN-boxed sentinel value (False, True, Null, Invalid)
    // These get unique hashes based on their bit patterns to avoid collisions
    if (is_exact_nan_value(value, FALSE_NAN) || 
        is_exact_nan_value(value, TRUE_NAN) ||
        is_exact_nan_value(value, NULL_NAN) || 
        is_exact_nan_value(value, INVALID_NAN)) {
        // Hash the bit representation to get a unique value
        uint64_t val_bits = std::bit_cast<uint64_t>(value);
        return std::hash<uint64_t>{}(val_bits);
    }
    
    // For regular floats: if the value is an exact integer, hash it as an integer
    // This provides better interoperability with integer types
    double intpart;
    if (std::modf(value, &intpart) == 0.0 && std::isfinite(value)) {
        // Value is an integer-valued float
        if (value >= static_cast<double>(INT64_MIN) && value <= static_cast<double>(INT64_MAX)) {
            int64_t int_val = static_cast<int64_t>(value);
            return std::hash<int64_t>{}(int_val);
        }
    }
    
    // For non-integer floats, hash the bit representation
    uint64_t val_bits = std::bit_cast<uint64_t>(value);
    return std::hash<uint64_t>{}(val_bits);
}

tl::expected<PrimitiveType, HakkaJsonResultEnum> JsonFloatCompact::get_impl() const
{
    double value = value_;

    // Return appropriate type based on NaN-boxing
    if (is_exact_nan_value(value, NULL_NAN))
    {
        return PrimitiveType{nullptr};
    }
    else if (is_exact_nan_value(value, TRUE_NAN))
    {
        return PrimitiveType(true);
    }
    else if (is_exact_nan_value(value, FALSE_NAN))
    {
        return PrimitiveType(false);
    }
    else if (is_exact_nan_value(value, INVALID_NAN))
    {
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }

    // Otherwise, just return the floating-point number
    return PrimitiveType(value);
}
