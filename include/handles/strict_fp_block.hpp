#ifndef __HAKKA_JSON_STRICT_FP_BLOCK_HPP__
#define __HAKKA_JSON_STRICT_FP_BLOCK_HPP__
#pragma once

#include <bit>
#include <cstdint>
#include <limits>
#include <type_traits>

#if defined(_MSC_VER)
  #define STRICT_FP_BEGIN           \
    __pragma(float_control(precise, on, push)) \
    __pragma(float_control(except,  on, push)) \
    __pragma(fenv_access(on))

  #define STRICT_FP_END             \
    __pragma(float_control(pop))    \
    __pragma(float_control(pop))

  #define STRICT_FP

#elif defined(__clang__)
  #define STRICT_FP_BEGIN                              \
    _Pragma("float_control(precise, on, push)")        \
    _Pragma("float_control(except,  on, push)")        \
    _Pragma("clang fp reassociate(off)")               \
    _Pragma("clang fp exceptions(strict)")             \
    _Pragma("clang fp contract(on)")

  #define STRICT_FP_END                \
    _Pragma("float_control(pop)")      \
    _Pragma("float_control(pop)")

  #define STRICT_FP

#elif defined(__GNUC__)
  #define STRICT_FP_BEGIN                                   \
    _Pragma("GCC push_options")                              \
    _Pragma("GCC optimize (\"no-finite-math-only\")")        \
    _Pragma("GCC optimize (\"signed-zeros\")")               \
    _Pragma("GCC optimize (\"trapping-math\")")              \
    _Pragma("GCC optimize (\"rounding-math\")")              \
    _Pragma("GCC optimize (\"no-associative-math\")")        \
    _Pragma("GCC optimize (\"no-reciprocal-math\")")         \
    _Pragma("GCC optimize (\"math-errno\")")

  #define STRICT_FP_END _Pragma("GCC pop_options")

  #define STRICT_FP __attribute__((optimize( \
      "no-finite-math-only", "signed-zeros", "trapping-math", \
      "rounding-math", "no-associative-math", "no-reciprocal-math", "math-errno")))
#else
  #define STRICT_FP_BEGIN
  #define STRICT_FP_END
  #define STRICT_FP
#endif

namespace hakka_json_internal {

constexpr bool is_ieee754_compliant = 
    std::numeric_limits<double>::is_iec559 &&
    std::numeric_limits<double>::has_quiet_NaN;

template<typename T>
constexpr bool validate_nan_representation(T value) noexcept {
    static_assert(std::is_floating_point_v<T>, "T must be floating-point");
    
    if constexpr (sizeof(T) == sizeof(std::uint64_t)) {
        std::uint64_t bits = std::bit_cast<std::uint64_t>(value);
        constexpr std::uint64_t EXP_MASK = 0x7FF0000000000000ull;
        constexpr std::uint64_t FRAC_MASK = 0x000FFFFFFFFFFFFFull;
        
        return ((bits & EXP_MASK) == EXP_MASK) && ((bits & FRAC_MASK) != 0);
    }
    return false;
}

} // namespace hakka_json_internal

STRICT_FP_BEGIN

namespace hakka_json_nan_detail {

[[gnu::used, gnu::noinline]] 
STRICT_FP 
static constexpr double construct_nan_bitwise(int offset) noexcept {
    // IEEE-754 double qNaN: exponent=0x7FF, bit 51 (quiet bit)=1, rest=payload
    // Use positive NaN base (0x7FF8...) to avoid collision with negative NaN (0xFFF8...)
    constexpr std::uint64_t QNAN_BASE = 0x7FF8000000000000ull;
    constexpr std::uint64_t PAYLOAD_MASK = 0x0007FFFFFFFFFFFFull;
    
    std::uint64_t payload = static_cast<std::uint64_t>(offset) & PAYLOAD_MASK;
    std::uint64_t nan_bits = QNAN_BASE | payload;
    
    return std::bit_cast<double>(nan_bits);
}

[[gnu::used, gnu::noinline]]
STRICT_FP
static constexpr double construct_nan_standard(int offset) noexcept {
    if (std::is_constant_evaluated()) {
        return construct_nan_bitwise(offset);
    } else if constexpr (hakka_json_internal::is_ieee754_compliant) {
        volatile double base = std::numeric_limits<double>::quiet_NaN();
        
        if (offset == 0) return base;
        
        std::uint64_t bits = std::bit_cast<std::uint64_t>(base);
        constexpr std::uint64_t PAYLOAD_MASK = 0x0007FFFFFFFFFFFFull;
        bits = (bits & ~PAYLOAD_MASK) | (static_cast<std::uint64_t>(offset) & PAYLOAD_MASK);
        
        volatile double result = std::bit_cast<double>(bits);
        return result;
    } else {
        return construct_nan_bitwise(offset);
    }
}

[[gnu::used, gnu::noinline]]
STRICT_FP
static constexpr double construct_nan_validated(int offset) noexcept {
    double candidate = construct_nan_bitwise(offset);
    
    if (std::is_constant_evaluated()) {
        return candidate;
    }
    
    if (!hakka_json_internal::validate_nan_representation(candidate)) {
        return construct_nan_standard(offset);
    }
    
    return candidate;
}

} // namespace hakka_json_nan_detail

[[gnu::used, gnu::noinline]]
STRICT_FP 
static consteval double get_nan(int offset) noexcept {
    return hakka_json_nan_detail::construct_nan_validated(offset);
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505) // unreferenced function with internal linkage has been removed
#endif

[[gnu::used, gnu::noinline]]
STRICT_FP
static double get_nan_runtime(int offset) noexcept {
    volatile int voffset = offset;
    volatile double result = hakka_json_nan_detail::construct_nan_bitwise(voffset);
    return result;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Use offset starting from 1024 to avoid collision with implementation-defined NaN values
// that typically start from 0, 1, 2, etc.
constexpr int NAN_OFFSET_BASE = 1024;

inline constinit double NULL_NAN    = get_nan(NAN_OFFSET_BASE + 0);
inline constinit double TRUE_NAN    = get_nan(NAN_OFFSET_BASE + 1);
inline constinit double FALSE_NAN   = get_nan(NAN_OFFSET_BASE + 2);
inline constinit double INVALID_NAN = get_nan(NAN_OFFSET_BASE + 3);

[[gnu::used, gnu::cold]]
static bool validate_nan_constants() noexcept {
    using hakka_json_internal::validate_nan_representation;
    
    return validate_nan_representation(NULL_NAN) &&
           validate_nan_representation(TRUE_NAN) &&
           validate_nan_representation(FALSE_NAN) &&
           validate_nan_representation(INVALID_NAN) &&
           (std::bit_cast<std::uint64_t>(NULL_NAN) != std::bit_cast<std::uint64_t>(TRUE_NAN)) &&
           (std::bit_cast<std::uint64_t>(TRUE_NAN) != std::bit_cast<std::uint64_t>(FALSE_NAN)) &&
           (std::bit_cast<std::uint64_t>(FALSE_NAN) != std::bit_cast<std::uint64_t>(INVALID_NAN));
}

STRICT_FP_END

#if defined(HAKKA_JSON_NAN_DEBUG) || !defined(NDEBUG)
#include <cstdio>
#include <cinttypes>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505) // unreferenced function with internal linkage has been removed
#endif

[[gnu::used, gnu::cold]]
static void dump_nan_info() noexcept {
    std::printf("=== NaN Configuration ===\n");
    std::printf("IEEE-754 compliant: %s\n", 
                hakka_json_internal::is_ieee754_compliant ? "yes" : "no");
    std::printf("has_quiet_NaN: %s\n", 
                std::numeric_limits<double>::has_quiet_NaN ? "yes" : "no");
    std::printf("has_signaling_NaN: %s\n", 
                std::numeric_limits<double>::has_signaling_NaN ? "yes" : "no");
    
    std::printf("\nNaN bit patterns:\n");
    std::printf("NULL_NAN   : 0x%016" PRIx64 "\n", std::bit_cast<std::uint64_t>(NULL_NAN));
    std::printf("TRUE_NAN   : 0x%016" PRIx64 "\n", std::bit_cast<std::uint64_t>(TRUE_NAN));
    std::printf("FALSE_NAN  : 0x%016" PRIx64 "\n", std::bit_cast<std::uint64_t>(FALSE_NAN));
    std::printf("INVALID_NAN: 0x%016" PRIx64 "\n", std::bit_cast<std::uint64_t>(INVALID_NAN));
    
    std::printf("\nValidation: %s\n", 
                validate_nan_constants() ? "PASS" : "FAIL");
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

#endif // __HAKKA_JSON_STRICT_FP_BLOCK_HPP__