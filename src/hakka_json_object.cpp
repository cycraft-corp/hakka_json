#include <hakka_json_object.hpp>
#include <hakka_json_array.hpp>
#include <handles/object_manager.hpp>
#include <json_deserializer.hpp>
#include <hakka_json_string.hpp>

#include <hakka_json_int.hpp>
#include <hakka_json_float.hpp>
#include <hakka_json_string.hpp>

#include <sstream>
#include <cstring>
#include <algorithm>
#include <utility>

using namespace hakka;

JsonObjectIterCompact::JsonObjectIterCompact(const JsonObjectCompact *obj, 
                                            HakkaIterTraits<JsonObjectIterCompact>::difference_type pos) 
    : obj_(obj), pos_(pos) {}

JsonObjectIterCompact::~JsonObjectIterCompact() = default;

JsonObjectIterCompact &JsonObjectIterCompact::operator++()
{
    ++pos_;
    return *this;
}

JsonObjectIterCompact JsonObjectIterCompact::operator++(int)
{
    JsonObjectIterCompact temp = *this;
    ++(*this);
    return temp;
}

JsonObjectIterCompact &JsonObjectIterCompact::operator--()
{
    --pos_;
    return *this;
}

JsonObjectIterCompact JsonObjectIterCompact::operator--(int)
{
    JsonObjectIterCompact temp = *this;
    --(*this);
    return temp;
}

JsonObjectIterCompact::pointer JsonObjectIterCompact::operator->() const
{
    auto key_string_handle = obj_->keys().at(static_cast<uint32_t>(pos_)).value(); // it is string handle
    auto key_string = std::get<const JsonStringCompact*>(key_string_handle.get_view())->get();
    if (!key_string)
        return nullptr;

    auto value_handle = obj_->values().at(static_cast<uint32_t>(pos_)).value(); // it is value handle
    if (!value_handle)
        return nullptr;

    return std::make_unique<value_type>(std::make_pair(
        std::get<std::string>(key_string.value()),
        value_handle));
}

bool JsonObjectIterCompact::operator==(const JsonObjectIterCompact &other) const
{
    return obj_ == other.obj_ && pos_ == other.pos_;
}

bool JsonObjectIterCompact::operator!=(const JsonObjectIterCompact &other) const
{
    return !(*this == other);
}

bool JsonObjectIterCompact::is_end() const
{
    return pos_ < 0 || static_cast<std::size_t>(pos_) >= obj_->length();
}

JsonObjectCompact::ObjectType_::ObjectType_()
    : keys(JsonArrayCompact::create()),
      values(JsonArrayCompact::create())
{
}

// Constructor
JsonObjectCompact::JsonObjectCompact() : JsonStructuredCompact(), elements_()
{
}

JsonObjectCompact::~JsonObjectCompact() = default;

JsonHandleCompact JsonObjectCompact::create()
{
    return JsonHandleCompact(ObjectManagerCompact::get_instance().create());
}

std::unique_ptr<JsonObjectCompact> JsonObjectCompact::create_unique()
{
    return std::unique_ptr<JsonObjectCompact>(new JsonObjectCompact());
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonObjectCompact::loads(const std::string &json_str, uint32_t max_depth)
{
    return loads(std::string_view(json_str), max_depth);
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonObjectCompact::loads(std::string_view json_str, uint32_t max_depth)
{
    JsonDeserializerCompact dj;
    auto res = dj.loads(json_str, max_depth);
    if (res != HAKKA_JSON_SUCCESS)
        return tl::make_unexpected(res);

    if (dj.type() != HAKKA_JSON_OBJECT)
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);

    return dj.to_hakka_json();
}

// Core CRTP implementations
uint64_t JsonObjectCompact::inc_ref_impl() const
{
    return ref_count.fetch_add(1, std::memory_order_relaxed) + 1;
}

uint64_t JsonObjectCompact::dec_ref_impl() const
{
    return ref_count.fetch_sub(1, std::memory_order_relaxed) - 1;
}

tl::expected<std::string, HakkaJsonResultEnum> JsonObjectCompact::dump_impl(uint32_t max_depth) const
{
    auto invoker = [](const auto *elem, uint32_t max_depth) -> decltype(auto) {
        return elem->dump(max_depth);
    };

    if (max_depth-- == 0)
        return tl::make_unexpected(HAKKA_JSON_RECURSION_DEPTH_EXCEEDED);

    try {
        std::ostringstream ss;
        ss << "{";

        auto keys_view = elements_.keys.get_view();
        auto values_view = elements_.values.get_view();
        const auto *keys_array = std::get<const JsonArrayCompact*>(keys_view);
        const auto *values_array = std::get<const JsonArrayCompact*>(values_view);

        if (!keys_array || !values_array)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        size_t num_keys = keys_array->length();
        size_t num_values = values_array->length();

        if (num_keys != num_values)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        for (size_t i = 0; i < num_keys; ++i)
        {
            if (i > 0)
                ss << ", ";

            auto key_handle = keys_array->at_impl(i);
            auto value_handle = values_array->at_impl(i);

            if (!key_handle || !value_handle)
                return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

            if (key_handle.value().get_type() != HakkaJsonType::HAKKA_JSON_STRING)
                return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);

            auto key_view = key_handle.value().get_view();
            const auto *key_str = std::get<const JsonStringCompact*>(key_view);
            if (!key_str)
                return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

            auto key_value = key_str->get_impl();
            if (!key_value)
                return tl::make_unexpected(key_value.error());

            ss << "\"" << std::get<std::string>(key_value.value()) << "\": ";

            auto dump_result = dispatch<decltype(invoker), std::string>(
                value_handle.value().get_view(), 
                std::forward<decltype(invoker)>(invoker),
                max_depth
            );

            if (!dump_result)
                return tl::make_unexpected(dump_result.error());

            ss << dump_result.value();
        }

        ss << "}";
        return ss.str();
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

HakkaJsonResultEnum JsonObjectCompact::to_bytes_impl(char *buffer, uint32_t *buffer_size) const
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
        buffer[serialized.size()] = '\0'; // Ensure null termination
        *buffer_size = static_cast<uint32_t>(serialized.size());
        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

HakkaJsonType JsonObjectCompact::type_impl() const
{
    return HakkaJsonType::HAKKA_JSON_OBJECT;
}

tl::expected<int, HakkaJsonResultEnum> JsonObjectCompact::compare_impl(const JsonHandleCompact &other) const
{
    auto invoker = [](const auto *elem, const JsonHandleCompact &other_elem) -> decltype(auto) {
        return elem->compare(other_elem);
    };

    if (other.get_type() != HakkaJsonType::HAKKA_JSON_OBJECT)
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);

    try {
        auto other_view = other.get_view();
        const auto *other_obj = std::get<const JsonObjectCompact*>(other_view);
        if (!other_obj)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        auto my_keys_view = elements_.keys.get_view();
        const auto *my_keys_array = std::get<const JsonArrayCompact*>(my_keys_view);
        if (!my_keys_array)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        size_t num_keys = my_keys_array->length();
        size_t other_num_keys = other_obj->length();

        size_t min_keys = std::min(num_keys, other_num_keys);

        for (size_t i = 0; i < min_keys; ++i)
        {
            auto my_key = my_keys_array->at_impl(i);
            if (!my_key || my_key.value().get_type() != HakkaJsonType::HAKKA_JSON_STRING)
                return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

            auto key_view = my_key.value().get_view();
            const auto *key_str = std::get<const JsonStringCompact*>(key_view);
            if (!key_str)
                return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

            auto key_value = key_str->get_impl();
            if (!key_value)
                return tl::make_unexpected(key_value.error());

            std::string my_key_str = std::get<std::string>(key_value.value());
            auto other_value = other_obj->get_impl(my_key_str);
            if (!other_value && other_value.error() == HAKKA_JSON_KEY_NOT_FOUND)
                return static_cast<int>(num_keys) - static_cast<int>(other_num_keys);
            else if (!other_value)
                return tl::make_unexpected(other_value.error());

            auto my_values_view = elements_.values.get_view();
            const auto *my_values_array = std::get<const JsonArrayCompact*>(my_values_view);
            if (!my_values_array)
                return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

            auto my_elem = my_values_array->at_impl(i);
            if (!my_elem)
                return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

            const auto &other_elem = other_value.value();

            if (my_elem.value().get_type() != other_elem.get_type())
                return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);

            auto cmp_result = dispatch<decltype(invoker), int>(
                my_elem.value().get_view(), 
                std::forward<decltype(invoker)>(invoker), 
                other_elem
            );
            if (!cmp_result)
                return tl::make_unexpected(cmp_result.error());

            if (*cmp_result != 0)
                return *cmp_result;
        }

        return static_cast<int>(num_keys) - static_cast<int>(other_num_keys);
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

uint64_t JsonObjectCompact::hash_impl() const
{
    return keys().hash_impl() ^ values().hash_impl();
}

uint64_t JsonObjectCompact::dump_size_impl() const
{
    auto invoker = [](const auto *elem) -> decltype(auto) {
        return elem->dump_size();
    };

    constexpr auto braces_size = 2;   // {}
    constexpr auto sep_size = 2;      // ", " comma separator
    constexpr auto key_value_sep = 2; // ": " colon separator

    auto overhead = braces_size + std::max<std::ptrdiff_t>(static_cast<std::ptrdiff_t>((length() - 1) * sep_size), 0l);
    auto size = overhead;

    try {
        auto keys_view = elements_.keys.get_view();
        auto values_view = elements_.values.get_view();
        const auto *keys_array = std::get<const JsonArrayCompact*>(keys_view);
        const auto *values_array = std::get<const JsonArrayCompact*>(values_view);

        if (!keys_array || !values_array)
            return 0;

        for (size_t i = 0; i < length(); ++i)
        {
            auto key_handle = keys_array->at_impl(i);
            auto value_handle = values_array->at_impl(i);

            if (!key_handle || key_handle.value().get_type() != HakkaJsonType::HAKKA_JSON_STRING)
                continue; // Skip invalid keys

            size += dispatch<decltype(invoker), uint64_t>(
                key_handle.value().get_view(), 
                std::forward<decltype(invoker)>(invoker)
            ).value_or(0);
            size += dispatch<decltype(invoker), uint64_t>(
                value_handle.value().get_view(), 
                std::forward<decltype(invoker)>(invoker)
            ).value_or(0);
            size += key_value_sep;                    // include colon separator
        }

        return size;
    } catch (...) {
        return 0;
    }
}

// JsonStructuredCompact implementation
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonObjectCompact::get_impl(KeyType key) const
{
    if (std::holds_alternative<int64_t>(key))
        return at_impl(std::get<int64_t>(key));

    if (!std::holds_alternative<std::string>(key))
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);

    const std::string &search_key = std::get<std::string>(key);
    auto find_index = find(search_key);

    if (find_index != -1)
    {
        try {
            auto values_view = elements_.values.get_view();
            const auto *values_array = std::get<const JsonArrayCompact*>(values_view);
            if (!values_array)
                return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

            return values_array->at_impl(find_index).value();
        } catch (...) {
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
        }
    }

    return tl::make_unexpected(HAKKA_JSON_KEY_NOT_FOUND);
}

HakkaJsonResultEnum JsonObjectCompact::set_impl(KeyType key, JsonHandleCompact value) const
{
    if (!std::holds_alternative<std::string>(key))
        return HAKKA_JSON_TYPE_ERROR;

    const std::string &set_key = std::get<std::string>(key);
    auto find_index = find(set_key);

    try {
        auto keys_view = elements_.keys.get_mut_ptr();
        auto values_view = elements_.values.get_mut_ptr();
        auto *keys_array = std::get<JsonArrayCompact*>(keys_view);
        auto *values_array = std::get<JsonArrayCompact*>(values_view);

        if (!keys_array || !values_array)
            return HAKKA_JSON_INTERNAL_ERROR;

        if (find_index != -1)
        {
            // Key exists, update the value
            return values_array->set_impl(find_index, value);
        }
        else
        {
            // Insert new key-value pair
            auto new_key = JsonStringCompact::create(set_key);
            if (!new_key.is_valid())
                return HAKKA_JSON_NOT_ENOUGH_MEMORY;

            auto res_keys = keys_array->push_back(new_key);
            if (res_keys != HAKKA_JSON_SUCCESS)
                return res_keys;

            auto res_values = values_array->push_back(value);
            if (res_values != HAKKA_JSON_SUCCESS)
                return res_values;
        }

        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

HakkaJsonResultEnum JsonObjectCompact::remove_impl(KeyType key) const
{
    if (!std::holds_alternative<std::string>(key))
        return HAKKA_JSON_TYPE_ERROR;

    const std::string &remove_key = std::get<std::string>(key);
    auto find_index = find(remove_key);
    if (find_index == -1)
        return HAKKA_JSON_KEY_NOT_FOUND;

    try {
        auto keys_view = elements_.keys.get_mut_ptr();
        auto values_view = elements_.values.get_mut_ptr();
        auto *keys_array = std::get<JsonArrayCompact*>(keys_view);
        auto *values_array = std::get<JsonArrayCompact*>(values_view);

        if (!keys_array || !values_array)
            return HAKKA_JSON_INTERNAL_ERROR;

        // remove key and value
        auto res_keys = keys_array->remove_impl(find_index);
        if (res_keys != HAKKA_JSON_SUCCESS)
            return res_keys;

        auto res_values = values_array->remove_impl(find_index);
        if (res_values != HAKKA_JSON_SUCCESS)
            return res_values;

        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonObjectCompact::at_impl(uint32_t index) const
{
    try {
        auto keys_view = elements_.keys.get_view();
        auto values_view = elements_.values.get_view();
        const auto *keys_array = std::get<const JsonArrayCompact*>(keys_view);
        const auto *values_array = std::get<const JsonArrayCompact*>(values_view);

        if (!keys_array || !values_array)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        size_t num_keys = keys_array->length();

        if (index >= num_keys)
            return tl::make_unexpected(HAKKA_JSON_INDEX_OUT_OF_BOUNDS);

        auto key_handle = keys_array->at_impl(index);
        auto value_handle = values_array->at_impl(index);

        if (!key_handle || !value_handle)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        if (key_handle.value().get_type() != HakkaJsonType::HAKKA_JSON_STRING)
            return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);

        auto key_view = key_handle.value().get_view();
        const auto *key_str = std::get<const JsonStringCompact*>(key_view);
        if (!key_str)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        auto key_value = key_str->get_impl();
        if (!key_value)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        // Create a JsonArrayCompact [key, value]
        auto key_json = JsonStringCompact::create(std::get<std::string>(key_value.value()));
        if (!key_json.is_valid())
            return tl::make_unexpected(HAKKA_JSON_NOT_ENOUGH_MEMORY);

        auto key_value_array = JsonArrayCompact::create();
        if (!key_value_array.is_valid())
            return tl::make_unexpected(HAKKA_JSON_NOT_ENOUGH_MEMORY);

        auto array_view = key_value_array.get_mut_ptr();
        auto *array_ptr = std::get<JsonArrayCompact*>(array_view);
        if (!array_ptr)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        auto res_push_key = array_ptr->push_back(key_json);
        if (res_push_key != HAKKA_JSON_SUCCESS)
            return tl::make_unexpected(res_push_key);

        auto res_push_value = array_ptr->push_back(value_handle.value());
        if (res_push_value != HAKKA_JSON_SUCCESS)
            return tl::make_unexpected(res_push_value);

        return key_value_array;
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

HakkaJsonResultEnum JsonObjectCompact::insert_impl(KeyType key, JsonHandleCompact value) const
{
    if (!std::holds_alternative<std::string>(key))
        return HAKKA_JSON_TYPE_ERROR;

    const std::string &insert_key = std::get<std::string>(key);

    try {
        auto new_key = JsonStringCompact::create(insert_key);
        if (!new_key.is_valid())
            return HAKKA_JSON_NOT_ENOUGH_MEMORY;

        auto keys_view = elements_.keys.get_mut_ptr();
        auto values_view = elements_.values.get_mut_ptr();
        auto *keys_array = std::get<JsonArrayCompact*>(keys_view);
        auto *values_array = std::get<JsonArrayCompact*>(values_view);

        if (!keys_array || !values_array)
            return HAKKA_JSON_INTERNAL_ERROR;

        auto res_keys = keys_array->push_back(new_key);
        if (res_keys != HAKKA_JSON_SUCCESS)
            return res_keys;

        auto res_values = values_array->push_back(value);
        if (res_values != HAKKA_JSON_SUCCESS)
            return res_values;

        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

HakkaJsonResultEnum JsonObjectCompact::erase_impl(KeyType key) const
{
    return remove_impl(key);
}

HakkaJsonResultEnum JsonObjectCompact::clear_impl() const
{
    try {
        auto keys_view = elements_.keys.get_mut_ptr();
        auto values_view = elements_.values.get_mut_ptr();
        auto *keys_array = std::get<JsonArrayCompact*>(keys_view);
        auto *values_array = std::get<JsonArrayCompact*>(values_view);

        if (!keys_array || !values_array)
            return HAKKA_JSON_INTERNAL_ERROR;

        auto res_keys = keys_array->clear_impl();
        if (res_keys != HAKKA_JSON_SUCCESS)
            return res_keys;

        auto res_values = values_array->clear_impl();
        if (res_values != HAKKA_JSON_SUCCESS)
            return res_values;

        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

void JsonObjectCompact::shrink_to_fit_impl() const
{
    try {
        auto keys_view = elements_.keys.get_mut_ptr();
        auto values_view = elements_.values.get_mut_ptr();
        auto *keys_array = std::get<JsonArrayCompact*>(keys_view);
        auto *values_array = std::get<JsonArrayCompact*>(values_view);

        if (keys_array && values_array)
        {
            keys_array->shrink_to_fit_impl();
            values_array->shrink_to_fit_impl();
        }
    } catch (...) {
        // Ignore errors in shrink_to_fit as it's an optimization
    }
}

// Additional methods for JsonObjectCompact
std::size_t JsonObjectCompact::length() const
{
    try {
        auto keys_view = elements_.keys.get_view();
        const auto *keys_array = std::get<const JsonArrayCompact*>(keys_view);
        return keys_array ? keys_array->length() : 0;
    } catch (...) {
        return 0;
    }
}

bool JsonObjectCompact::contains(KeyType key) const
{
    if (!std::holds_alternative<std::string>(key))
        return false;

    const std::string &search_key = std::get<std::string>(key);
    return find(search_key) != -1;
}

const JsonArrayCompact &JsonObjectCompact::keys() const
{
    auto keys_view = elements_.keys.get_view();
    return *std::get<const JsonArrayCompact*>(keys_view);
}

JsonHandleCompact JsonObjectCompact::keys_handle() const
{
    return elements_.keys;
}

const JsonArrayCompact &JsonObjectCompact::values() const
{
    auto values_view = elements_.values.get_view();
    return *std::get<const JsonArrayCompact*>(values_view);
}

JsonHandleCompact JsonObjectCompact::values_handle() const
{
    return elements_.values;
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonObjectCompact::fromkeys(const std::vector<KeyType> &keys, JsonHandleCompact value)
{
    auto new_obj_handle = JsonObjectCompact::create();
    if (!new_obj_handle.is_valid())
        return tl::make_unexpected(HAKKA_JSON_NOT_ENOUGH_MEMORY);

    try {
        auto obj_view = new_obj_handle.get_mut_ptr();
        auto *new_obj = std::get<JsonObjectCompact*>(obj_view);
        if (!new_obj)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        for (const auto &key_variant : keys)
        {
            if (!std::holds_alternative<std::string>(key_variant))
                continue; // Skip invalid keys

            const std::string &key = std::get<std::string>(key_variant);
            auto res = new_obj->set_impl(key, value);
            if (res != HAKKA_JSON_SUCCESS)
                return tl::make_unexpected(res);
        }

        return new_obj_handle;
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonObjectCompact::pop(const KeyType &key)
{
    if (!std::holds_alternative<std::string>(key))
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);

    const std::string &pop_key = std::get<std::string>(key);
    auto find_index = find(pop_key);

    if (find_index == -1)
        return tl::make_unexpected(HAKKA_JSON_KEY_NOT_FOUND);

    try {
        auto values_view = elements_.values.get_view();
        const auto *values_array = std::get<const JsonArrayCompact*>(values_view);
        if (!values_array)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        auto value_handle = values_array->at_impl(find_index);
        if (!value_handle)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        // Store the value before removing
        JsonHandleCompact result = value_handle.value();

        // remove key and value
        auto keys_mut_view = elements_.keys.get_mut_ptr();
        auto values_mut_view = elements_.values.get_mut_ptr();
        auto *keys_array = std::get<JsonArrayCompact*>(keys_mut_view);
        auto *values_array_mut = std::get<JsonArrayCompact*>(values_mut_view);

        if (!keys_array || !values_array_mut)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        auto res_keys = keys_array->remove_impl(find_index);
        if (res_keys != HAKKA_JSON_SUCCESS)
            return tl::make_unexpected(res_keys);

        auto res_values = values_array_mut->remove_impl(find_index);
        if (res_values != HAKKA_JSON_SUCCESS)
            return tl::make_unexpected(res_values);

        return result;
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

tl::expected<std::pair<KeyType, JsonHandleCompact>, HakkaJsonResultEnum> JsonObjectCompact::popitem()
{
    try {
        auto keys_view = elements_.keys.get_view();
        const auto *keys_array = std::get<const JsonArrayCompact*>(keys_view);
        if (!keys_array)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        size_t num_keys = keys_array->length();

        if (num_keys == 0)
            return tl::make_unexpected(HAKKA_JSON_KEY_NOT_FOUND);

        // Remove the last item
        size_t last_index = num_keys - 1;
        auto key_handle = keys_array->at_impl(last_index);

        auto values_view = elements_.values.get_view();
        const auto *values_array = std::get<const JsonArrayCompact*>(values_view);
        if (!values_array)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        auto value_handle = values_array->at_impl(last_index);

        if (!key_handle || !value_handle)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        auto key_view = key_handle.value().get_view();
        const auto *key_str = std::get<const JsonStringCompact*>(key_view);
        if (!key_str)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        auto key_value = key_str->get_impl();
        if (!key_value)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        // Store results before removing
        std::string key_str_result = std::get<std::string>(key_value.value());
        JsonHandleCompact value_result = value_handle.value();

        auto keys_mut_view = elements_.keys.get_mut_ptr();
        auto values_mut_view = elements_.values.get_mut_ptr();
        auto *keys_array_mut = std::get<JsonArrayCompact*>(keys_mut_view);
        auto *values_array_mut = std::get<JsonArrayCompact*>(values_mut_view);

        if (!keys_array_mut || !values_array_mut)
            return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);

        auto res_keys = keys_array_mut->remove_impl(static_cast<int64_t>(last_index));
        if (res_keys != HAKKA_JSON_SUCCESS)
            return tl::make_unexpected(res_keys);

        auto res_values = values_array_mut->remove_impl(static_cast<int64_t>(last_index));
        if (res_values != HAKKA_JSON_SUCCESS)
            return tl::make_unexpected(res_values);

        return std::make_pair(KeyType(key_str_result), value_result);
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

tl::expected<JsonHandleCompact, HakkaJsonResultEnum> JsonObjectCompact::setdefault(const KeyType &key, JsonHandleCompact default_value)
{
    if (!std::holds_alternative<std::string>(key))
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);

    const std::string &def_key = std::get<std::string>(key);
    auto existing = get_impl(def_key);
    if (existing)
        return existing.value();

    // Set to default value
    auto res = set_impl(def_key, default_value);
    if (res != HAKKA_JSON_SUCCESS)
        return tl::make_unexpected(res);

    return default_value;
}

HakkaJsonResultEnum JsonObjectCompact::update(const JsonObjectCompact &other)
{
    try {
        auto other_keys_view = other.elements_.keys.get_view();
        auto other_values_view = other.elements_.values.get_view();
        const auto *other_keys_array = std::get<const JsonArrayCompact*>(other_keys_view);
        const auto *other_values_array = std::get<const JsonArrayCompact*>(other_values_view);

        if (!other_keys_array || !other_values_array)
            return HAKKA_JSON_INTERNAL_ERROR;

        size_t num_keys = other_keys_array->length();

        for (size_t i = 0; i < num_keys; ++i)
        {
            auto key_handle = other_keys_array->at_impl(static_cast<uint32_t>(i));
            auto value_handle = other_values_array->at_impl(static_cast<uint32_t>(i));

            if (!key_handle || !value_handle)
                continue;

            if (key_handle.value().get_type() != HakkaJsonType::HAKKA_JSON_STRING)
                continue;

            auto key_view = key_handle.value().get_view();
            const auto *key_str = std::get<const JsonStringCompact*>(key_view);
            if (!key_str)
                continue;

            auto key_value = key_str->get_impl();
            if (!key_value)
                continue;

            auto res = set_impl(std::get<std::string>(key_value.value()), value_handle.value());
            if (res != HAKKA_JSON_SUCCESS)
                return res;
        }

        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

int64_t JsonObjectCompact::find(const std::string &key) const
{
    try {
        auto keys_view = elements_.keys.get_view();
        const auto *keys_array = std::get<const JsonArrayCompact*>(keys_view);
        if (!keys_array)
            return -1;

        size_t num_keys = keys_array->length();

        for (size_t i = 0; i < num_keys; ++i)
        {
            auto key_handle = keys_array->at_impl(static_cast<uint32_t>(i));
            if (!key_handle || key_handle.value().get_type() != HakkaJsonType::HAKKA_JSON_STRING)
                continue;

            auto key_view = key_handle.value().get_view();
            const auto *key_str = std::get<const JsonStringCompact*>(key_view);
            if (!key_str)
                continue;

            auto key_value = key_str->get_impl();
            if (!key_value)
                continue;

            if (std::get<std::string>(key_value.value()) == key)
                return static_cast<int64_t>(i);
        }

        return -1;
    } catch (...) {
        return -1;
    }
}

JsonObjectIterCompact JsonObjectCompact::begin() const
{
    return JsonObjectIterCompact(this, 0);
}

JsonObjectIterCompact JsonObjectCompact::end() const
{
    return JsonObjectIterCompact(this, static_cast<HakkaIterTraits<JsonObjectIterCompact>::difference_type>(length()));
}