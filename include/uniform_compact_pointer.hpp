#ifndef __UNIFORM_COMPACT_POINTER_HPP__
#define __UNIFORM_COMPACT_POINTER_HPP__
#pragma once

#include <cstddef>
#include <variant>
#include <memory>
#include <functional>
#include <tl/expected.hpp>
#include <type_traits>
#include <climits>

#include <hakka_json_enum.h>

namespace hakka {

class JsonIntCompact;
class JsonFloatCompact;
class JsonBoolCompact;
class JsonStringCompact;
class JsonArrayCompact;
class JsonObjectCompact;
class JsonNullCompact;
class JsonInvalidCompact;

// This will NOT take the ownership of the object, so the caller must ensure the pointers are valid.
using UniformCompactPointerView = std::variant<std::monostate,
                                           const JsonIntCompact*,
                                           const JsonFloatCompact*,
                                           const JsonBoolCompact*,
                                           const JsonStringCompact*,
                                           const JsonArrayCompact*,
                                           const JsonObjectCompact*,
                                           const JsonNullCompact*,
                                           const JsonInvalidCompact*>;

template <typename T>
concept isNanBoxingType = std::is_same_v<T, const JsonFloatCompact*> ||
                           std::is_same_v<T, const JsonBoolCompact*> ||
                           std::is_same_v<T, const JsonNullCompact*> ||
                           std::is_same_v<T, const JsonInvalidCompact*>;

template <class Invoker, class Ret, class... Args>
auto dispatch(UniformCompactPointerView v, Invoker&& invoke, Args&&... args)
    -> decltype(auto)
{
    return std::visit(
        [&](auto&& arg) -> tl::expected<Ret, HakkaJsonResultEnum> {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::monostate>) {
                return tl::make_unexpected(HAKKA_JSON_INVALID_ARGUMENT);
            } else if constexpr (isNanBoxingType<T>) {
                if (!arg) return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
                return std::invoke(
                    std::forward<Invoker>(invoke),
                    reinterpret_cast<const JsonFloatCompact*>(arg), // YES, it is nan boxing
                    std::forward<Args>(args)...
                );
            } else {
                static_assert(std::is_pointer_v<T>, "Variant is expected to hold pointer alternatives.");
                if (!arg) return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
                return std::invoke(
                    std::forward<Invoker>(invoke),
                    arg,
                    std::forward<Args>(args)...
                );
            }
        },
        v
    );
}
                           
// Due to Scalar types are immutable, we don't need to keep the pointers.
using UniformCompactPointer = std::variant<std::monostate,
                                           JsonArrayCompact*,
                                           JsonObjectCompact*>;

class OwnedUniformCompactPointer {
    // This is a tagged pointer that take the ownership of the object.
    // The bit fields are:
    // high .................................. low
    // | type 4 bits | the rest of pointer bits |
    // | 0000 | for null type    |
    // | 0001 | for string type  |
    // | 0010 | for int type     |
    // | 0011 | for float type   |
    // | 0100 | for bool type    |
    // | 0101 | for object type  |
    // | 0110 | for array type   |
    // | 1111 | for invalid type |


    using PointerType = void*;
    PointerType data = nullptr;
    // -- This order MUST be the same as the HakkaJsonType enum --
    constexpr static auto SHIFT_OFFSET = (sizeof(PointerType) * CHAR_BIT) - 4;
    constexpr static auto INT_MASK = std::size_t(HAKKA_JSON_NULL) << SHIFT_OFFSET;
    constexpr static auto FLOAT_MASK = std::size_t(HAKKA_JSON_FLOAT) << SHIFT_OFFSET;
    constexpr static auto BOOL_MASK = std::size_t(HAKKA_JSON_BOOL) << SHIFT_OFFSET;
    constexpr static auto NULL_MASK = std::size_t(HAKKA_JSON_NULL) << SHIFT_OFFSET;
    constexpr static auto STRING_MASK = std::size_t(HAKKA_JSON_STRING) << SHIFT_OFFSET;
    constexpr static auto ARRAY_MASK = std::size_t(HAKKA_JSON_ARRAY) << SHIFT_OFFSET;
    constexpr static auto OBJECT_MASK = std::size_t(HAKKA_JSON_OBJECT) << SHIFT_OFFSET;
    constexpr static auto INVALID_MASK = std::size_t(HAKKA_JSON_INVALID) << SHIFT_OFFSET;
    // -- This order MUST be the same as the HakkaJsonType enum --
    constexpr static auto TYPE_MASK = (INVALID_MASK | NULL_MASK | STRING_MASK | INT_MASK | FLOAT_MASK | BOOL_MASK | ARRAY_MASK | OBJECT_MASK);

    HakkaJsonType get_type() const {
        return static_cast<HakkaJsonType>((reinterpret_cast<std::size_t>(data) & TYPE_MASK) >> SHIFT_OFFSET);
    }

public:
    OwnedUniformCompactPointer() : data(nullptr) {}
    explicit OwnedUniformCompactPointer(std::unique_ptr<JsonIntCompact> ptr) : data(
        (PointerType)(((std::size_t)ptr.release()) | (INT_MASK))
    ) {}
    explicit OwnedUniformCompactPointer(std::unique_ptr<JsonFloatCompact> ptr) : data(
        (PointerType)(((std::size_t)ptr.release()) | (FLOAT_MASK))
    ) {}
    explicit OwnedUniformCompactPointer(std::unique_ptr<JsonBoolCompact> ptr) : data(
        (PointerType)(((std::size_t)ptr.release()) | (BOOL_MASK))
    ) {}
    explicit OwnedUniformCompactPointer(std::unique_ptr<JsonNullCompact> ptr) : data(
        (PointerType)(((std::size_t)ptr.release()) | (NULL_MASK))
    ) {}
    explicit OwnedUniformCompactPointer(std::unique_ptr<JsonInvalidCompact> ptr) : data(
        (PointerType)(((std::size_t)ptr.release()) | (INVALID_MASK))
    ) {}
    explicit OwnedUniformCompactPointer(std::unique_ptr<JsonStringCompact> ptr) : data(
        (PointerType)(((std::size_t)ptr.release()) | (STRING_MASK))
    ) {}
    explicit OwnedUniformCompactPointer(std::unique_ptr<JsonArrayCompact> ptr) : data(
        (PointerType)(((std::size_t)ptr.release()) | (ARRAY_MASK))
    ) {}
    explicit OwnedUniformCompactPointer(std::unique_ptr<JsonObjectCompact> ptr) : data(
        (PointerType)(((std::size_t)ptr.release()) | (OBJECT_MASK))
    ) {}
    explicit OwnedUniformCompactPointer(std::nullptr_t) : data(nullptr) {} // nullptr constructor for invalid state

    OwnedUniformCompactPointer(OwnedUniformCompactPointer &&other) noexcept : data(other.data) {
        other.data = nullptr;
    }
    OwnedUniformCompactPointer(const OwnedUniformCompactPointer &other) = delete;
    OwnedUniformCompactPointer &operator=(OwnedUniformCompactPointer &&other) noexcept {
        if (this != &other) {
            data = other.data;
            other.data = nullptr;
        }
        return *this;
    }
    OwnedUniformCompactPointer &operator=(const OwnedUniformCompactPointer &other) = delete;

    ~OwnedUniformCompactPointer();

    template <typename T>
    T *get() const {
        return reinterpret_cast<T *>(reinterpret_cast<std::size_t>(data) & ~TYPE_MASK);
    }

    template <typename T>
    void emplace(std::unique_ptr<T> ptr) {
        this->~OwnedUniformCompactPointer();
        new (this) OwnedUniformCompactPointer(std::move(ptr));
    }

    void emplace(std::nullptr_t) {
        this->~OwnedUniformCompactPointer();
        new (this) OwnedUniformCompactPointer(std::nullptr_t{});
    }

    bool operator==(std::nullptr_t) const {
        return data == nullptr;
    }

    bool operator!=(std::nullptr_t) const {
        return data != nullptr;
    }
};

} // namespace hakka

#endif // __UNIFORM_COMPACT_POINTER_HPP__