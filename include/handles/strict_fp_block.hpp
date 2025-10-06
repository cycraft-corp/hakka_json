#ifndef __HAKKA_JSON_STRICT_FP_BLOCK_HPP__
#define __HAKKA_JSON_STRICT_FP_BLOCK_HPP__
#pragma once
#include <bit>
#include <cstdint>

#if defined(_MSC_VER)
// MSVC: use float_control/fenv_access/fp_contract
//   precise,on   -> disables many fast-math transforms, honors NaN/Inf
//   except,on    -> (optional) preserves exception behavior
//   fp_contract  -> keep default (or set off if you want to avoid FMA)
  #define STRICT_FP_BEGIN           \
    __pragma(float_control(precise, on, push)) \
    __pragma(float_control(except,  on, push)) \
    __pragma(fenv_access(on))

  #define STRICT_FP_END             \
    __pragma(float_control(pop))    \
    __pragma(float_control(pop))

  #define STRICT_FP  /* per-fn attribute not needed on MSVC */

#elif defined(__clang__)
// Clang: two knobs:
//  1) MSVC-compatible pragma float_control (Clang implements it)
//  2) Clang's own #pragma clang fp … for fine-grained control
  #define STRICT_FP_BEGIN                              \
    _Pragma("float_control(precise, on, push)")        \
    _Pragma("float_control(except,  on, push)")        \
    _Pragma("clang fp reassociate(off)")               \
    _Pragma("clang fp exceptions(strict)")             \
    _Pragma("clang fp contract(on)")

  #define STRICT_FP_END                \
    _Pragma("float_control(pop)")      \
    _Pragma("float_control(pop)")

  // Per-function attribute (Clang accepts GCC-style 'optimize' in limited form;
  // 'optnone' is the nuclear option; we avoid it here.)
  #define STRICT_FP  /* not using per-fn attr by default */

#elif defined(__GNUC__)
// GCC: #pragma GCC optimize("no-fast-math") is **not honored** (design/bug) —
// use push_options + disable the sub-flags fast-math would set.
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

  // Per-function alternative if you prefer attributes:
  // (Same sub-flags; works even when the pragma is ignored.)
  #define STRICT_FP __attribute__((optimize( \
      "no-finite-math-only", "signed-zeros", "trapping-math", \
      "rounding-math", "no-associative-math", "no-reciprocal-math", "math-errno")))
#else
  #define STRICT_FP_BEGIN
  #define STRICT_FP_END
  #define STRICT_FP
#endif


STRICT_FP_BEGIN

// If you want per-function granularity on GCC instead of the block,
// change the next line to:  STRICT_FP static consteval double get_nan(int offset)
static consteval double get_nan(int offset)
{
    constexpr std::uint64_t NAN_BASE = 0xFFF8000000000000ull;
    std::uint64_t code = NAN_BASE + static_cast<std::uint64_t>(offset);
    return std::bit_cast<double>(code);
}

constinit static double NULL_NAN    = get_nan(0);
constinit static double TRUE_NAN    = get_nan(1);
constinit static double FALSE_NAN   = get_nan(2);
constinit static double INVALID_NAN = get_nan(3);

STRICT_FP_END

#endif // __HAKKA_JSON_STRICT_FP_BLOCK_HPP__