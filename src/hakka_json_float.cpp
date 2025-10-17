#include <hakka_json_float.hpp>
#include <hakka_compare.hpp>
#include <handles/scalar_manager.hpp>
#include <handles/strict_fp_block.hpp>

#include <bit>
#include <cmath>
#include <cstring>
#include <string>
#include <cstdio>

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
    // Python hash algorithm constants (64-bit system)
    constexpr int _PyHASH_BITS = 61;
    constexpr uint64_t _PyHASH_MODULUS = (1ULL << _PyHASH_BITS) - 1;  // 2305843009213693951
    constexpr int64_t _PyHASH_INF = 314159;

    // Helper lambda for Python's pointer-like hash
    // Rotates bits right by 4 to avoid hash collisions
    auto pointer_hash = [](uint64_t x) -> uint64_t {
        // Bottom 3 or 4 bits are likely to be 0; rotate x by 4 to the right
        x = (x >> 4) | (x << (8 * sizeof(uint64_t) - 4));

        // Python converts -1 to -2 to avoid using -1 as sentinel
        if (x == static_cast<uint64_t>(-1)) {
            x = static_cast<uint64_t>(-2);
        }
        return x;
    };

    // Check if this is a nanboxed special value (NULL, TRUE, FALSE, INVALID)
    // Use exact bitwise comparison against the actual NaN constants
    if (is_exact_nan_value(value, TRUE_NAN)) {
        return 1;  // hash(True) = 1 in Python
    }
    if (is_exact_nan_value(value, FALSE_NAN)) {
        return 0;  // hash(False) = 0 in Python
    }
    if (is_exact_nan_value(value, NULL_NAN)) {
        // hash(None) in Python - use pointer-like hash on a fixed address
        // In CPython, this is _Py_HashPointer(&_Py_NoneStruct)
        uint64_t null_nan_bits = std::bit_cast<uint64_t>(NULL_NAN);
        return pointer_hash(null_nan_bits);
    }
    if (is_exact_nan_value(value, INVALID_NAN)) {
        // INVALID has no Python equivalent, use distinct hash
        uint64_t invalid_nan_bits = std::bit_cast<uint64_t>(INVALID_NAN);
        return pointer_hash(invalid_nan_bits);
    }

    // Get bit representation for regular NaN check
    uint64_t val_bits = std::bit_cast<uint64_t>(value);

    // For regular floats, use Python's hash algorithm
    // Handle special cases: infinity and NaN
    if (!std::isfinite(value)) {
        if (std::isinf(value)) {
            return value > 0 ? _PyHASH_INF : static_cast<uint64_t>(-_PyHASH_INF);
        } else {
            // Regular NaN: use pointer-like hash
            return pointer_hash(val_bits);
        }
    }

    // Python's hash algorithm for finite floats
    int e;
    double m = std::frexp(value, &e);

    int sign = 1;
    if (m < 0) {
        sign = -1;
        m = -m;
    }

    // Process 28 bits at a time
    uint64_t x = 0;
    while (m) {
        x = ((x << 28) & _PyHASH_MODULUS) | (x >> (_PyHASH_BITS - 28));
        m *= 268435456.0;  // 2^28
        e -= 28;
        uint64_t y = static_cast<uint64_t>(m);  // Pull out integer part
        m -= y;
        x += y;
        if (x >= _PyHASH_MODULUS)
            x -= _PyHASH_MODULUS;
    }

    // Adjust for the exponent; first reduce it modulo _PyHASH_BITS
    e = e >= 0 ? e % _PyHASH_BITS : _PyHASH_BITS - 1 - ((-1 - e) % _PyHASH_BITS);
    x = ((x << e) & _PyHASH_MODULUS) | (x >> (_PyHASH_BITS - e));

    // Apply sign
    x = x * sign;

    // Python converts -1 to -2 to avoid using -1 as sentinel
    if (x == static_cast<uint64_t>(-1))
        x = static_cast<uint64_t>(-2);

    return x;
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
