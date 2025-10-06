#ifndef __HAKKA_JSON_ARRAY_HPP__
#define __HAKKA_JSON_ARRAY_HPP__
#pragma once

#include <hakka_json_structured.hpp>
#include <hakka_iter_base.hpp>

#include <vector>
#include <memory>
#include <string>
#include <cstddef>
#include <iterator>

namespace hakka {

class JsonArrayCompact;
enum class IterDirection
{
    FORWARD = 1,
    REVERSE = -1,
};

template <IterDirection Direction>
class JsonArrayIterCompact;

template <IterDirection Direction>
struct HakkaIterTraits<JsonArrayIterCompact<Direction>>
{
    using iterator_category = std::random_access_iterator_tag;
    using value_type = JsonHandleCompact;
    using difference_type = std::ptrdiff_t;
    using pointer = JsonHandleCompact *;
    using reference = JsonHandleCompact &;
};

class JsonArrayCompact : public JsonStructuredCompact<JsonArrayCompact>
{
    friend class JsonArrayIterCompact<IterDirection::FORWARD>;
    friend class JsonArrayIterCompact<IterDirection::REVERSE>;

public:
    using ArrayType = std::vector<JsonHandleCompact>;

    ~JsonArrayCompact();

    // Factory method
    [[nodiscard]] static JsonHandleCompact create();
    [[nodiscard]] static std::unique_ptr<JsonArrayCompact> create_unique();
    static tl::expected<JsonHandleCompact, HakkaJsonResultEnum> loads(const std::string &json_str, uint32_t max_depth = 2048);
    static tl::expected<JsonHandleCompact, HakkaJsonResultEnum> loads(std::string_view json_str, uint32_t max_depth = 2048);

    // Overrides from JsonBaseCompact
    uint64_t inc_ref_impl() const;
    uint64_t dec_ref_impl() const;
    tl::expected<std::string, HakkaJsonResultEnum> dump_impl([[maybe_unused]] uint32_t max_depth = 0) const;
    HakkaJsonResultEnum to_bytes_impl(char *buffer, uint32_t *buffer_size) const;
    HakkaJsonType type_impl() const;
    tl::expected<int, HakkaJsonResultEnum> compare_impl(const JsonHandleCompact &other) const;
    uint64_t hash_impl() const;
    uint64_t dump_size_impl() const;

    HakkaJsonResultEnum multiply(uint32_t times);

    // Overrides from JsonStructuredCompact
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> get_impl(KeyType key) const;
    HakkaJsonResultEnum set_impl(KeyType key, JsonHandleCompact value);
    HakkaJsonResultEnum remove_impl(KeyType key);
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> at_impl(uint32_t index) const;
    HakkaJsonResultEnum insert_impl(KeyType index, JsonHandleCompact value);
    HakkaJsonResultEnum erase_impl(KeyType key);
    HakkaJsonResultEnum clear_impl();
    void shrink_to_fit_impl();

    // Additional methods to support C API operations
    std::size_t length() const { return elements_.size(); }
    HakkaJsonResultEnum count(const JsonHandleCompact &value, uint32_t *out_count) const;
    HakkaJsonResultEnum extend(const JsonHandleCompact &other);
    HakkaJsonResultEnum index(const JsonHandleCompact &value, uint32_t start, uint32_t stop, uint32_t *out_index) const;
    HakkaJsonResultEnum push_back(JsonHandleCompact value);
    HakkaJsonResultEnum pop(uint32_t index, JsonHandleCompact *pop_outed);
    HakkaJsonResultEnum remove_value(const JsonHandleCompact &value);
    HakkaJsonResultEnum reverse();
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> get_slice(int start, int end, int step) const;
    HakkaJsonResultEnum set_slice(int start, int end, int step, JsonHandleCompact value);

    HakkaJsonResultEnum reserve(size_t size) noexcept;
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> pop_back() noexcept;

    JsonArrayIterCompact<IterDirection::FORWARD> begin();
    JsonArrayIterCompact<IterDirection::FORWARD> end() const;

    JsonArrayIterCompact<IterDirection::REVERSE> rbegin();
    JsonArrayIterCompact<IterDirection::REVERSE> rend() const;

private:
    ArrayType elements_;

    // Private constructor to enforce usage of factory method
    JsonArrayCompact();
};

template <IterDirection Direction>
class JsonArrayIterCompact : public HakkaIterBase<JsonArrayIterCompact<Direction>>
{
public:
    using difference_type = typename HakkaIterTraits<JsonArrayIterCompact<Direction>>::difference_type;
    using pointer = typename HakkaIterTraits<JsonArrayIterCompact<Direction>>::pointer;
    using reference = typename HakkaIterTraits<JsonArrayIterCompact<Direction>>::reference;

    JsonArrayIterCompact(const JsonArrayCompact *array, difference_type pos = 0) : array_(array), pos_(pos)
    {
        // Clamp the initial position within valid bounds
        if (pos_ < 0)
            pos_ = 0;
        if (pos_ > static_cast<difference_type>(array_->length()))
            pos_ = static_cast<difference_type>(array_->length());
    }

    ~JsonArrayIterCompact() = default;
    JsonArrayIterCompact(const JsonArrayIterCompact &) = default;
    JsonArrayIterCompact &operator=(const JsonArrayIterCompact &) = default;
    JsonArrayIterCompact(JsonArrayIterCompact &&) = default;
    JsonArrayIterCompact &operator=(JsonArrayIterCompact &&) = default;
    
    // Increment and Decrement operators
    JsonArrayIterCompact &operator++()
    {
        if constexpr (Direction == IterDirection::FORWARD)
        {
            if (pos_ < static_cast<difference_type>(array_->length()))
                ++pos_;
        }
        else // REVERSE
        {
            if (pos_ >= 0)
                --pos_;
        }
        return *this;
    }

    JsonArrayIterCompact operator++(int)
    {
        JsonArrayIterCompact temp = *this;
        ++(*this);
        return temp;
    }

    JsonArrayIterCompact &operator--()
    {
        if constexpr (Direction == IterDirection::FORWARD)
        {
            if (pos_ > 0)
                --pos_;
        }
        else // REVERSE
        {
            if (pos_ < static_cast<difference_type>(array_->length() - 1))
                ++pos_;
        }
        return *this;
    }

    JsonArrayIterCompact operator--(int)
    {
        JsonArrayIterCompact temp = *this;
        --(*this);
        return temp;
    }

    // Dereference operators
    pointer operator->() const
    {
        return const_cast<pointer>(&array_->elements_[pos_]);
    }

    reference operator*() const
    {
        return const_cast<reference>(array_->elements_[pos_]);
    }
    
    // Comparison operators
    bool operator==(const JsonArrayIterCompact &other) const
    {
        return array_ == other.array_ && pos_ == other.pos_;
    }
    
    bool operator!=(const JsonArrayIterCompact &other) const { return !(*this == other); }

    // Random access operators
    JsonArrayIterCompact &operator+=(difference_type n)
    {
        if constexpr (Direction == IterDirection::FORWARD)
        {
            pos_ += n;
        }
        else // REVERSE
        {
            pos_ -= n;
        }

        // Clamp the position within valid bounds
        pos_ = std::max(static_cast<difference_type>(0), pos_);
        pos_ = std::min(pos_, static_cast<difference_type>(array_->length()));

        return *this;
    }

    JsonArrayIterCompact operator+(difference_type n) const
    {
        JsonArrayIterCompact temp = *this;
        temp += n;
        return temp;
    }

    JsonArrayIterCompact &operator-=(difference_type n)
    {
        if constexpr (Direction == IterDirection::FORWARD)
        {
            pos_ -= n;
        }
        else // REVERSE
        {
            pos_ += n;
        }

        // Clamp the position within valid bounds
        pos_ = std::max(static_cast<difference_type>(0), pos_);
        pos_ = std::min(pos_, static_cast<difference_type>(array_->length()));

        return *this;
    }

    JsonArrayIterCompact operator-(difference_type n) const
    {
        JsonArrayIterCompact temp = *this;
        temp -= n;
        return temp;
    }

    // Utility function to check if iterator is at the end
    bool is_end() const
    {
        if constexpr (Direction == IterDirection::FORWARD)
        {
            return pos_ >= static_cast<difference_type>(array_->length()) || pos_ < 0;
        }
        else // REVERSE
        {
            return pos_ < 0 || pos_ >= static_cast<difference_type>(array_->length());
        }
    }

private:
    const JsonArrayCompact *const array_;
    difference_type pos_;
};

} // namespace hakka

#endif // __HAKKA_JSON_ARRAY_HPP__