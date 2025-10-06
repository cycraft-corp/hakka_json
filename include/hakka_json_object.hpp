#ifndef __HAKKA_JSON_OBJECT_HPP__
#define __HAKKA_JSON_OBJECT_HPP__
#pragma once

#include <hakka_json_structured.hpp>
#include <hakka_json_string.hpp>
#include <hakka_json_array.hpp>
#include <hakka_iter_base.hpp>

namespace hakka {

class JsonObjectIterCompact;
class JsonObjectCompact;

template <>
struct HakkaIterTraits<JsonObjectIterCompact>
{
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::pair<KeyType, JsonHandleCompact>;
    using difference_type = std::ptrdiff_t;
    using pointer = std::unique_ptr<value_type>;
    using reference = const value_type &;
};

class JsonObjectIterCompact : public HakkaIterBase<JsonObjectIterCompact>
{
public:
    JsonObjectIterCompact(const JsonObjectCompact *obj, HakkaIterTraits<JsonObjectIterCompact>::difference_type pos);
    ~JsonObjectIterCompact();

    JsonObjectIterCompact &operator++();
    JsonObjectIterCompact operator++(int);
    JsonObjectIterCompact &operator--();
    JsonObjectIterCompact operator--(int);
    pointer operator->() const;
    bool operator==(const JsonObjectIterCompact &other) const;
    bool operator!=(const JsonObjectIterCompact &other) const;

    bool is_end() const;

private:
    const JsonObjectCompact *const obj_;
    std::ptrdiff_t pos_;
};

class JsonObjectCompact : public JsonStructuredCompact<JsonObjectCompact>
{
    struct ObjectType_
    {
        ObjectType_();
        ~ObjectType_() = default;
        JsonHandleCompact keys;   // handle to JsonArray, but the JsonHandle is a JsonString type
        JsonHandleCompact values; // handle to JsonArray
    };

public:
    using ObjectType = ObjectType_;

    virtual ~JsonObjectCompact();

    // Factory method
    [[nodiscard]] static JsonHandleCompact create();
    [[nodiscard]] static std::unique_ptr<JsonObjectCompact> create_unique();

    static tl::expected<JsonHandleCompact, HakkaJsonResultEnum> loads(const std::string &json_str, uint32_t max_depth = 2048);
    static tl::expected<JsonHandleCompact, HakkaJsonResultEnum> loads(std::string_view json_str, uint32_t max_depth = 2048);

    uint64_t inc_ref_impl() const;
    uint64_t dec_ref_impl() const;

    // Overrides from JsonBase
    tl::expected<std::string, HakkaJsonResultEnum> dump_impl(uint32_t max_depth = 0) const;
    HakkaJsonResultEnum to_bytes_impl(char *buffer, uint32_t *buffer_size) const;
    HakkaJsonType type_impl() const;
    tl::expected<int, HakkaJsonResultEnum> compare_impl(const JsonHandleCompact &other) const;
    uint64_t hash_impl() const;
    uint64_t dump_size_impl() const;

    // Overrides from JsonStructured
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> get_impl(KeyType key) const;
    HakkaJsonResultEnum set_impl(KeyType key, JsonHandleCompact value) const;
    HakkaJsonResultEnum remove_impl(KeyType key) const;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> at_impl(uint32_t index) const;
    HakkaJsonResultEnum insert_impl(KeyType index, JsonHandleCompact value) const;
    HakkaJsonResultEnum erase_impl(KeyType key) const;
    HakkaJsonResultEnum clear_impl() const;
    void shrink_to_fit_impl() const;

    // Additional methods to support C API operations with Python dict-like behavior
    std::size_t length() const;
    bool contains(KeyType key) const;
    const JsonArrayCompact &keys() const;
    JsonHandleCompact keys_handle() const;
    const JsonArrayCompact &values() const;
    JsonHandleCompact values_handle() const;
    static tl::expected<JsonHandleCompact, HakkaJsonResultEnum> fromkeys(const std::vector<KeyType> &keys, JsonHandleCompact value);
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> pop(const KeyType &key);
    tl::expected<std::pair<KeyType, JsonHandleCompact>, HakkaJsonResultEnum> popitem();
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> setdefault(const KeyType &key, JsonHandleCompact default_value = JsonHandleCompact());
    HakkaJsonResultEnum update(const JsonObjectCompact &other);

    // Additional methods to support C API operations with Python dict-like behavior
    // begin() and end iterator support
    JsonObjectIterCompact begin() const;
    JsonObjectIterCompact end() const;

private:
    int64_t find(const std::string &key) const; // Returns -1 if not found, otherwise returns the index
    mutable ObjectType elements_;

    // Private constructor to enforce usage of factory method
    JsonObjectCompact();
};

} // namespace hakka

#endif // __HAKKA_JSON_OBJECT_HPP__