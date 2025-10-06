#ifndef __HAKKA_JSON_HANDLE_SCALAR_MANAGER_HPP__
#define __HAKKA_JSON_HANDLE_SCALAR_MANAGER_HPP__
#pragma once

#include <handles/manager.hpp>

#include <cstdint>
#include <type_traits>

namespace hakka {

class ScalarManagerCompact : public JsonHandleManagerCompact
{
    // Scalar types are: Int, and Floats.
    // We collapse the Null, True, False, Invalid to the Floats NaN (NaN Boxing).
    ScalarManagerCompact() = default;

public:
    // scalar mask take one more bit to separate int from others
    static constexpr auto scalar_mask = 0xE0000000; // 001 or 000 (1110 0000 0000 0000 0000 0000 0000 0000)
    static ScalarManagerCompact &get_instance();
    ~ScalarManagerCompact() = default;

    HakkaJsonType type(HandleManagerToken token) const override;
    UniformCompactPointerView get_view(HandleManagerToken token) const override;
    void release(HandleManagerToken token) override;

    template <typename T>
        requires std::is_same_v<T, int64_t> ||
                 std::is_same_v<T, double> ||
                 std::is_same_v<T, bool> ||
                 std::is_same_v<T, std::nullptr_t> ||
                 std::is_same_v<T, void *>
    HandleManagerToken create(T &&value);
};

} // namespace hakka

#endif // __HAKKA_JSON_HANDLE_SCALAR_MANAGER_HPP__
