#ifndef __HAKKA_JSON_PRIMITIVE_HPP__
#define __HAKKA_JSON_PRIMITIVE_HPP__
#pragma once

#include <hakka_json_base.hpp>
#include <hakka_json_handle.hpp>

#include <cstdint>
#include <variant>
#include <atomic>
#include <tl/expected.hpp>

namespace hakka {

// Int, Float, String, Null, Invalid types
using PrimitiveType = std::variant<int64_t, bool, double, std::string, std::nullptr_t, void *>;

template <typename Derived, typename T>
class JsonPrimitiveCompact : public JsonBaseCompact<Derived>
{
public:
    using value_type = T;
    explicit JsonPrimitiveCompact(T v) : value_(v) {}
    ~JsonPrimitiveCompact() = default;

    tl::expected<PrimitiveType, HakkaJsonResultEnum> get() const {
        return static_cast<const Derived *>(this)->get_impl();
    };
    bool is_valid() const { return true; }

protected:
    T value_;
    mutable std::atomic<uint64_t> ref_count = 1;
};

} // namespace hakka

#endif // __HAKKA_JSON_PRIMITIVE_HPP__