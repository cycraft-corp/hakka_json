#include <hakka_json_array.hpp>
#include <hakka_json_handle.hpp>
#include <handles/array_manager.hpp>
#include <json_deserializer.hpp>

#include <hakka_json_int.hpp>
#include <hakka_json_float.hpp>
#include <hakka_json_string.hpp>
#include <hakka_json_object.hpp>

#include <sstream>
#include <cstring>
#include <algorithm>

#include <tl/expected.hpp>
#include <nlohmann/json.hpp>

using namespace hakka;

namespace
{
    namespace detail
    {
        // Compact version helpers
        HakkaJsonResultEnum try_reserve_compact(JsonArrayCompact::ArrayType &array, size_t size)
        {
            try
            {
                array.reserve(size);
            }
            catch (const std::bad_alloc &)
            {
                return HAKKA_JSON_NOT_ENOUGH_MEMORY;
            }
            catch (...)
            {
                return HAKKA_JSON_INTERNAL_ERROR;
            }
            return HAKKA_JSON_SUCCESS;
        }

        HakkaJsonResultEnum
        set_slice_single_compact(JsonArrayCompact::ArrayType &array,
                                 int start,
                                 int stop,
                                 const JsonArrayCompact::ArrayType &value)
        {
            // if value is empty, we just erase the range
            if (value.empty())
            {
                array.erase(array.begin() + start, array.begin() + stop);
                return HAKKA_JSON_SUCCESS;
            }

            std::vector<JsonHandleCompact> tmp;
            auto tmp_reserve = try_reserve_compact(tmp, array.size() + value.size() - (stop - start));
            if (tmp_reserve != HAKKA_JSON_SUCCESS)
                return tmp_reserve;

            // tmp = [begin:start] + value + [stop:end]
            std::move(array.begin(), array.begin() + start, std::back_inserter(tmp));
            std::move(value.begin(), value.end(), std::back_inserter(tmp));
            std::move(array.begin() + stop, array.end(), std::back_inserter(tmp));

            array.swap(tmp);
            return HAKKA_JSON_SUCCESS;
        }
    }
}

JsonArrayCompact::JsonArrayCompact() : JsonStructuredCompact(), elements_()
{
}

JsonArrayCompact::~JsonArrayCompact() = default;

JsonHandleCompact JsonArrayCompact::create()
{
    return JsonHandleCompact(ArrayManagerCompact::get_instance().create());
}

std::unique_ptr<JsonArrayCompact> JsonArrayCompact::create_unique()
{
    return std::unique_ptr<JsonArrayCompact>(new JsonArrayCompact());
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonArrayCompact::loads(const std::string &json_str, uint32_t max_depth)
{
    return loads(std::string_view(json_str), max_depth);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonArrayCompact::loads(std::string_view json_str, uint32_t max_depth)
{
    JsonDeserializerCompact dj;
    auto res = dj.loads(json_str, max_depth);
    if (res != HAKKA_JSON_SUCCESS)
        return tl::make_unexpected(res);

    if (dj.type() != HAKKA_JSON_ARRAY)
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);

    return dj.to_hakka_json();
}

tl::expected<std::string, HakkaJsonResultEnum> JsonArrayCompact::dump_impl(uint32_t max_depth) const
{
    auto invoker = [](const auto *elem, uint32_t max_depth) -> decltype(auto) {
        return elem->dump(max_depth);
    };

    if (max_depth-- == 0)
        return tl::make_unexpected(HAKKA_JSON_RECURSION_DEPTH_EXCEEDED);
    if (elements_.empty())
        return "[]";

    try {
        std::ostringstream ss;
        ss << "[";

        for (size_t i = 0; i < elements_.size(); ++i)
        {
            if (i > 0)
                ss << ", ";

            auto dump_result = dispatch<decltype(invoker), std::string>(
                elements_[i].get_view(), 
                std::forward<decltype(invoker)>(invoker),
                max_depth
            );

            if (!dump_result)
                return tl::make_unexpected(dump_result.error());

            ss << dump_result.value();
        }

        ss << "]";
        return ss.str();
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

uint64_t JsonArrayCompact::inc_ref_impl() const
{
    return ref_count.fetch_add(1, std::memory_order_relaxed) + 1;
}

uint64_t JsonArrayCompact::dec_ref_impl() const
{
    return ref_count.fetch_sub(1, std::memory_order_relaxed) - 1;
}

HakkaJsonResultEnum JsonArrayCompact::to_bytes_impl(char *buffer, uint32_t *buffer_size) const
{
    try {
        auto dump_result = dump_impl(512);
        if (!dump_result)
            return dump_result.error();
        std::string serialized = dump_result.value();
        uint32_t required_size = static_cast<uint32_t>(serialized.size()) + 1; // +1 for null terminator

        if (*buffer_size < required_size)
        {
            *buffer_size = required_size;
            return HAKKA_JSON_NOT_ENOUGH_MEMORY;
        }

        std::memcpy(buffer, serialized.c_str(), required_size);
        buffer[serialized.size()] = '\0';
        *buffer_size = static_cast<uint32_t>(serialized.size());
        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

HakkaJsonType JsonArrayCompact::type_impl() const
{
    return HakkaJsonType::HAKKA_JSON_ARRAY;
}

tl::expected<int, HakkaJsonResultEnum> JsonArrayCompact::compare_impl(const JsonHandleCompact &other) const
{
    if (other.get_type() != HakkaJsonType::HAKKA_JSON_ARRAY)
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);

    auto invoker = [](const auto *elem, const JsonHandleCompact &other_elem) -> decltype(auto) {
        return elem->compare(other_elem);
    };

    try {
        auto other_view = other.get_view();
        const auto *other_array = std::get<const JsonArrayCompact*>(other_view);
        if (!other_array)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        size_t min_size = std::min(elements_.size(), other_array->elements_.size());
        for (size_t i = 0; i < min_size; ++i)
        {
            auto cmp_result = dispatch<decltype(invoker), int>(
                elements_[i].get_view(), 
                std::forward<decltype(invoker)>(invoker), 
                other_array->elements_[i]
            );
            if (!cmp_result)
                return tl::make_unexpected(cmp_result.error());

            if (cmp_result.value() != 0)
                return cmp_result.value();
        }

        if (elements_.size() < other_array->elements_.size()) return -1;
        if (elements_.size() > other_array->elements_.size()) return 1;
        return 0;
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

uint64_t JsonArrayCompact::hash_impl() const
{
    auto invoker = [](const auto *elem) -> decltype(auto) {
        return elem->hash();
    };

    uint64_t hash = 0;
    for (const auto &elem : elements_)
        hash ^= dispatch<decltype(invoker), uint64_t>(
            elem.get_view(), 
            std::forward<decltype(invoker)>(invoker)
        ).value_or(0);
    return hash;
}

uint64_t JsonArrayCompact::dump_size_impl() const
{
    auto invoker = [](const auto *elem) -> decltype(auto) {
        return elem->dump_size();
    };

    // recursive size + separator + brackets
    constexpr auto brackets_size = 2; // []
    constexpr auto sep_size = 2;      // ", "
    auto overhead = brackets_size + std::max<std::ptrdiff_t>(static_cast<std::ptrdiff_t>((length() - 1) * sep_size), 0l);

    uint64_t size = overhead;
    for (const auto &elem : elements_)
        size += dispatch<decltype(invoker), uint64_t>(
            elem.get_view(), 
            std::forward<decltype(invoker)>(invoker)
        ).value_or(0);

    return size;
}

// JsonStructuredCompact implementation
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonArrayCompact::get_impl(KeyType key) const
{
    if (!std::holds_alternative<int64_t>(key))
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);

    int64_t index = std::get<int64_t>(key);
    if (index < 0 || static_cast<size_t>(index) >= elements_.size())
        return tl::make_unexpected(HAKKA_JSON_INDEX_OUT_OF_BOUNDS);

    return elements_[index];
}

HakkaJsonResultEnum JsonArrayCompact::set_impl(KeyType key, JsonHandleCompact value)
{
    if (!std::holds_alternative<int64_t>(key))
        return HAKKA_JSON_TYPE_ERROR;

    int64_t index = std::get<int64_t>(key);
    if (index < 0 || static_cast<size_t>(index) >= elements_.size())
        return HAKKA_JSON_INDEX_OUT_OF_BOUNDS;

    elements_[index] = value;
    return HAKKA_JSON_SUCCESS;
}

HakkaJsonResultEnum JsonArrayCompact::remove_impl(KeyType key)
{
    if (!std::holds_alternative<int64_t>(key))
        return HAKKA_JSON_TYPE_ERROR;

    int64_t index = std::get<int64_t>(key);
    if (index < 0 || static_cast<size_t>(index) >= elements_.size())
        return HAKKA_JSON_INDEX_OUT_OF_BOUNDS;

    elements_.erase(elements_.begin() + index);
    return HAKKA_JSON_SUCCESS;
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonArrayCompact::at_impl(uint32_t index) const
{
    if (index >= elements_.size())
        return tl::make_unexpected(HAKKA_JSON_INDEX_OUT_OF_BOUNDS);

    return elements_[index];
}

HakkaJsonResultEnum JsonArrayCompact::insert_impl(KeyType key, JsonHandleCompact value)
{
    if (!std::holds_alternative<int64_t>(key))
        return HAKKA_JSON_TYPE_ERROR;

    int64_t index = std::get<int64_t>(key);
    if (index < 0 || static_cast<size_t>(index) > elements_.size())
        return HAKKA_JSON_INDEX_OUT_OF_BOUNDS;

    elements_.insert(elements_.begin() + index, std::move(value));
    return HAKKA_JSON_SUCCESS;
}

HakkaJsonResultEnum JsonArrayCompact::erase_impl(KeyType key)
{
    return remove_impl(key);
}

HakkaJsonResultEnum JsonArrayCompact::clear_impl()
{
    elements_.clear();
    elements_.shrink_to_fit();
    return HAKKA_JSON_SUCCESS;
}

void JsonArrayCompact::shrink_to_fit_impl()
{
    elements_.resize(elements_.size());
    elements_.shrink_to_fit();
}

// multiply method: Repeats the array 'times' times
HakkaJsonResultEnum JsonArrayCompact::multiply(uint32_t times)
{
    if (times == 0)
    {
        elements_.clear();
        return HAKKA_JSON_SUCCESS;
    }

    if (times == 1)
        return HAKKA_JSON_SUCCESS;

    auto res = detail::try_reserve_compact(elements_, elements_.size() * times);
    if (res != HAKKA_JSON_SUCCESS)
        return res;

    const auto &[start, stop] = std::make_pair(elements_.begin(), elements_.end() - 1);
    for (uint32_t i = 1; i < times; ++i)
    {
        elements_.insert(elements_.end(), start, stop);
        elements_.push_back(*stop);
    }

    return HAKKA_JSON_SUCCESS;
}

// Additional methods for JsonArrayCompact
HakkaJsonResultEnum JsonArrayCompact::count(const JsonHandleCompact &value, uint32_t *out_count) const
{
    auto invoker = [](const auto *elem, const JsonHandleCompact &other_elem) -> decltype(auto) {
        return elem->compare(other_elem);
    };

    if (!out_count)
        return HAKKA_JSON_INVALID_ARGUMENT;

    *out_count = 0;
    for (const auto &elem : elements_)
    {
        auto cmp_result = dispatch<decltype(invoker), int>(
            elem.get_view(), 
            std::forward<decltype(invoker)>(invoker), 
            value
        );
        if (!cmp_result && cmp_result.error() != HAKKA_JSON_TYPE_ERROR)
            return cmp_result.error();
        else if (!cmp_result)
            continue;

        if (*cmp_result == 0)
            ++(*out_count);
    }
    return HAKKA_JSON_SUCCESS;
}

HakkaJsonResultEnum JsonArrayCompact::extend(const JsonHandleCompact &other)
{
    if (other.get_type() != HakkaJsonType::HAKKA_JSON_ARRAY)
        return HAKKA_JSON_TYPE_ERROR;

    try {
        auto other_view = other.get_view();
        const auto *other_array = std::get<const JsonArrayCompact*>(other_view);
        if (!other_array)
            return HAKKA_JSON_INTERNAL_ERROR;

        elements_.insert(elements_.end(), other_array->elements_.begin(), other_array->elements_.end());
        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

HakkaJsonResultEnum JsonArrayCompact::index(const JsonHandleCompact &value, uint32_t start, uint32_t stop, uint32_t *out_index) const
{
    auto invoker = [](const auto *elem, const JsonHandleCompact &other_elem) -> decltype(auto) {
        return elem->compare(other_elem);
    };

    if (!out_index)
        return HAKKA_JSON_INVALID_ARGUMENT;

    if (start >= elements_.size())
        return HAKKA_JSON_INDEX_OUT_OF_BOUNDS;

    stop = std::min(stop, static_cast<uint32_t>(elements_.size()));

    for (uint32_t i = start; i < stop; ++i)
    {
        auto cmp_result = dispatch<decltype(invoker), int>(
            elements_[i].get_view(), 
            std::forward<decltype(invoker)>(invoker), 
            value
        );
        if (!cmp_result && cmp_result.error() == HAKKA_JSON_TYPE_ERROR)
            continue;
        else if (!cmp_result)
            return cmp_result.error();

        if (*cmp_result == 0)
        {
            *out_index = i;
            return HAKKA_JSON_SUCCESS;
        }
    }

    return HAKKA_JSON_KEY_NOT_FOUND;
}

HakkaJsonResultEnum JsonArrayCompact::push_back(JsonHandleCompact value)
{
    if (!value.is_valid())
        return HAKKA_JSON_INVALID_ARGUMENT;

    try {
        elements_.emplace_back(std::move(value));
        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

HakkaJsonResultEnum JsonArrayCompact::pop(uint32_t index, JsonHandleCompact *pop_outed)
{
    if (!pop_outed)
        return HAKKA_JSON_INVALID_ARGUMENT;

    if (index >= elements_.size())
        return HAKKA_JSON_INDEX_OUT_OF_BOUNDS;

    try {
        *pop_outed = std::move(elements_[index]);
        elements_.erase(elements_.begin() + index);
        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

// remove_value method: Removes the first occurrence of a value
HakkaJsonResultEnum JsonArrayCompact::remove_value(const JsonHandleCompact &value)
{
    auto invoker = [](const auto *elem, const JsonHandleCompact &other_elem) -> decltype(auto) {
        return elem->compare(other_elem);
    };

    for (auto it = elements_.begin(); it != elements_.end(); ++it)
    {
        auto cmp_result = dispatch<decltype(invoker), int>(
            (*it).get_view(), 
            std::forward<decltype(invoker)>(invoker), 
            value
        );
        if (!cmp_result && cmp_result.error() == HAKKA_JSON_TYPE_ERROR)
            continue;
        else if (!cmp_result)
            return cmp_result.error();

        if (*cmp_result == 0)
        {
            try {
                elements_.erase(it);
                return HAKKA_JSON_SUCCESS;
            } catch (...) {
                return HAKKA_JSON_INTERNAL_ERROR;
            }
        }
    }
    return HAKKA_JSON_KEY_NOT_FOUND;
}

HakkaJsonResultEnum JsonArrayCompact::reverse()
{
    try {
        std::reverse(elements_.begin(), elements_.end());
        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonArrayCompact::get_slice(int start, int end, int step) const
{
    if (step == 0)
        return tl::make_unexpected(HAKKA_JSON_INVALID_ARGUMENT);

    if (start < 0)
        start += static_cast<int>(elements_.size());
    if (end < 0)
        end += static_cast<int>(elements_.size());

    auto slice_array_handle = JsonArrayCompact::create();
    if (!slice_array_handle.is_valid())
        return tl::make_unexpected(HAKKA_JSON_NOT_ENOUGH_MEMORY);

    try {
        auto slice_ptr = slice_array_handle.get_mut_ptr();
        auto *slice_array = std::get<JsonArrayCompact*>(slice_ptr);
        if (!slice_array)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        // resize the slice array to the expected size
        auto res = detail::try_reserve_compact(slice_array->elements_, (end - start) / step);
        if (res != HAKKA_JSON_SUCCESS)
            return tl::make_unexpected(res);

        for (int i = start; i < end && i >= 0; i += step)
        {
            slice_array->push_back(elements_[i]);
        }

        return slice_array_handle;
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

// set_slice method: Sets a slice of the array
// Similar to Python's list slicing, this method sets a slice of the array with a new value
HakkaJsonResultEnum JsonArrayCompact::set_slice(int start, int end, int step, JsonHandleCompact value)
{
    // TODO: add more types of value
    if (step == 0 || value.get_type() != HakkaJsonType::HAKKA_JSON_ARRAY)
        return HAKKA_JSON_INVALID_ARGUMENT;

    if (start < 0)
        start += static_cast<int>(elements_.size());
    if (end < 0)
        end += static_cast<int>(elements_.size());

    // if start is still less than 0, it means the slice is invalid
    if (start < 0 || end < 0)
        return HAKKA_JSON_INVALID_ARGUMENT;

    try {
        auto value_ptr = value.get_view();
        const auto *value_array = std::get<const JsonArrayCompact*>(value_ptr);
        if (!value_array)
            return HAKKA_JSON_INTERNAL_ERROR;

        const std::size_t &original_slice_size = (end - start) / step;

        // It depends on the step, if the step is 1, it is special case
        // Otherwise, the original_slice_size and value_array->length() should be equal
        if (step == 1)
            return detail::set_slice_single_compact(elements_, start, end, value_array->elements_);
        else if (original_slice_size != value_array->length())
            return HAKKA_JSON_INVALID_ARGUMENT;

        for (int i = start, j = 0; i < end && i >= 0; i += step, ++j)
        {
            elements_[i] = value_array->elements_[j];
        }

        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

HakkaJsonResultEnum JsonArrayCompact::reserve(size_t size) noexcept
{
    return detail::try_reserve_compact(elements_, size);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonArrayCompact::pop_back() noexcept
{
    if (elements_.empty())
        return tl::make_unexpected(HAKKA_JSON_INDEX_OUT_OF_BOUNDS);

    try {
        JsonHandleCompact popped = std::move(elements_.back());
        elements_.pop_back();
        return popped;
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

JsonArrayIterCompact<IterDirection::FORWARD> JsonArrayCompact::begin()
{
    return JsonArrayIterCompact<IterDirection::FORWARD>(this, 0);
}

JsonArrayIterCompact<IterDirection::FORWARD> JsonArrayCompact::end() const
{
    return JsonArrayIterCompact<IterDirection::FORWARD>(this, elements_.size());
}

JsonArrayIterCompact<IterDirection::REVERSE> JsonArrayCompact::rbegin()
{
    return JsonArrayIterCompact<IterDirection::REVERSE>(this, elements_.size() - 1);
}

JsonArrayIterCompact<IterDirection::REVERSE> JsonArrayCompact::rend() const
{
    return JsonArrayIterCompact<IterDirection::REVERSE>(this, -1);
}