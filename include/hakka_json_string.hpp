#ifndef __HAKKA_JSON_STRING_HPP__
#define __HAKKA_JSON_STRING_HPP__
#pragma once

#include <hakka_json_primitive.hpp>
#include <hakka_iter_base.hpp>
#include <pico_string.hpp>

#include <string_view>
#include <memory>

namespace hakka {

class JsonStringCompactIter;

template <>
struct HakkaIterTraits<JsonStringCompactIter>
{
    using value_type = char32_t;
    using reference = char32_t;
    using pointer = const char32_t *;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;
};

class JsonStringCompactIter : public HakkaIterBase<JsonStringCompactIter>
{
    JsonStringCompactIter(std::nullptr_t);

public:
    JsonStringCompactIter() = default;
    ~JsonStringCompactIter();

    JsonStringCompactIter(std::string_view str, bool end = false);
    JsonStringCompactIter(JsonStringCompactIter &&other) noexcept;
    JsonStringCompactIter &operator=(JsonStringCompactIter &&other) noexcept;

    reference operator*() const;
    pointer operator->() const = delete;
    JsonStringCompactIter &operator++();
    JsonStringCompactIter &operator--();
    bool operator==(const JsonStringCompactIter &other) const;
    bool operator!=(const JsonStringCompactIter &other) const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

class JsonStringCompact : public JsonPrimitiveCompact<JsonStringCompact, scc::PicoStringProxy> {
public:
    using ValueType = scc::PicoStringProxy;
    
    ~JsonStringCompact() { dec_ref(); }
    [[nodiscard]] static JsonHandleCompact create(const ValueType &value);
    [[nodiscard]] static JsonHandleCompact create(std::string_view value);
    [[nodiscard]] static JsonHandleCompact create(const std::string &value)
    {
        return create(std::string_view(value));
    }
    template <size_t N>
    [[nodiscard]] static JsonHandleCompact create(const char (&value)[N])
    {
        return create(std::string_view(value, N - 1));
    }
    [[nodiscard]] static std::unique_ptr<JsonStringCompact> create_unique(const ValueType &value);
    [[nodiscard]] static std::unique_ptr<JsonStringCompact> create_unique(std::string_view value);
    [[nodiscard]] static std::unique_ptr<JsonStringCompact> create_unique(const std::string &value)
    {
        return create_unique(std::string_view(value));
    }
    template <size_t N>
    [[nodiscard]] static std::unique_ptr<JsonStringCompact> create_unique(const char (&value)[N])
    {
        return create_unique(std::string_view(value, N - 1));
    }

    // From JsonBaseCompact
    uint64_t inc_ref_impl() const;
    uint64_t dec_ref_impl() const;
    tl::expected<std::string, HakkaJsonResultEnum> dump_impl([[maybe_unused]] uint32_t max_depth = 0) const;
    HakkaJsonResultEnum to_bytes_impl(char *buffer, uint32_t *buffer_size) const;
    HakkaJsonType type_impl() const;
    tl::expected<int, HakkaJsonResultEnum> compare_impl(const JsonHandleCompact &other) const;
    uint64_t hash_impl() const;
    uint64_t dump_size_impl() const;

    // From JsonPrimitiveCompact
    tl::expected<PrimitiveType, HakkaJsonResultEnum> get_impl() const;
    using JsonPrimitiveCompact::is_valid;             // inherit is_valid

    // String Manipulation Functions, they are not from CRTP
    tl::expected<int64_t, HakkaJsonResultEnum> length() const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> capitalize() const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> casefold() const;
    tl::expected<int64_t, HakkaJsonResultEnum> count(const std::string &substring) const;
    tl::expected<int64_t, HakkaJsonResultEnum> count(std::string_view substring) const;
    tl::expected<bool, HakkaJsonResultEnum> endswith(const std::string &suffix) const;
    tl::expected<bool, HakkaJsonResultEnum> endswith(std::string_view suffix) const;
    tl::expected<int64_t, HakkaJsonResultEnum> find(const std::string &substring) const;
    tl::expected<int64_t, HakkaJsonResultEnum> find(std::string_view substring) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> concatenate(const std::string &other) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> concatenate(std::string_view other) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> multiply(int64_t times) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> slice(int64_t start, int64_t end, int64_t step = 1) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> lower() const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> removeprefix(const std::string &prefix) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> removeprefix(std::string_view prefix) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> removesuffix(const std::string &suffix) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> removesuffix(std::string_view suffix) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> replace(const std::string &old_substr, const std::string &new_substr) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> replace(std::string_view old_substr, std::string_view new_substr) const;
    tl::expected<int64_t, HakkaJsonResultEnum> rfind(const std::string &substring) const;
    tl::expected<int64_t, HakkaJsonResultEnum> rfind(std::string_view substring) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> rsplit(const std::string &separator, int64_t maxsplit = -1) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> rsplit(std::string_view separator, int64_t maxsplit = -1) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> split(const std::string &separator, int64_t maxsplit = -1) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> split(std::string_view separator, int64_t maxsplit = -1) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> splitlines(bool keepends = false) const;
    tl::expected<bool, HakkaJsonResultEnum> startswith(const std::string &prefix) const;
    tl::expected<bool, HakkaJsonResultEnum> startswith(std::string_view prefix) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> upper() const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> swapcase() const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> title() const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> zfill(int64_t width) const;

    // Helper for python get the string, because the UTF-8 needing capability is not equal length
    // For example: "繁體中文" is 4 characters, but 12 bytes
    tl::expected<uint64_t, HakkaJsonResultEnum> utf8_length() const;

    // String Testing Functions
    tl::expected<bool, HakkaJsonResultEnum> isalnum() const;
    tl::expected<bool, HakkaJsonResultEnum> isalpha() const;
    tl::expected<bool, HakkaJsonResultEnum> isascii() const;
    tl::expected<bool, HakkaJsonResultEnum> isdecimal() const;
    tl::expected<bool, HakkaJsonResultEnum> isdigit() const;
    tl::expected<bool, HakkaJsonResultEnum> isidentifier() const;
    tl::expected<bool, HakkaJsonResultEnum> islower() const;
    tl::expected<bool, HakkaJsonResultEnum> isnumeric() const;
    tl::expected<bool, HakkaJsonResultEnum> isprintable() const;
    tl::expected<bool, HakkaJsonResultEnum> isspace() const;
    tl::expected<bool, HakkaJsonResultEnum> istitle() const;
    tl::expected<bool, HakkaJsonResultEnum> isupper() const;

    JsonStringCompactIter begin() const;
    JsonStringCompactIter end() const;

private:
    explicit JsonStringCompact(const ValueType &value);
    explicit JsonStringCompact(std::string_view value);
};

} // namespace hakka

#endif
