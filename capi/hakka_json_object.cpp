#include <hakka_json_object.h>

#include <hakka_json_string.hpp>
#include <hakka_json_int.hpp>
#include <hakka_json_float.hpp>
#include <hakka_json_array.hpp>
#include <hakka_json_object.hpp>

#include <cstring>
#include <string>
#include <string_view>
#include <uniform_compact_pointer.hpp>

using namespace hakka;

static JsonHandleCompact *to_json_handle_compact(HakkaHandle *handle)
{
    static_assert(sizeof(JsonHandleCompact) <= sizeof(HakkaHandle));
    return reinterpret_cast<JsonHandleCompact *>(handle);
}

static const JsonObjectCompact *to_json_object_compact(HakkaHandle *handle)
{
    JsonHandleCompact *json_handle = to_json_handle_compact(handle);
    if (json_handle->get_type() != HakkaJsonType::HAKKA_JSON_OBJECT)
        return nullptr;
    
    try {
        auto view = json_handle->get_view();
        return std::get<const JsonObjectCompact*>(view);
    } catch (...) {
        return nullptr;
    }
}

static JsonObjectCompact *to_json_object_compact_mut(HakkaHandle *handle)
{
    JsonHandleCompact *json_handle = to_json_handle_compact(handle);
    if (json_handle->get_type() != HakkaJsonType::HAKKA_JSON_OBJECT)
        return nullptr;
    
    try {
        auto mut_ptr = json_handle->get_mut_ptr();
        return std::get<JsonObjectCompact*>(mut_ptr);
    } catch (...) {
        return nullptr;
    }
}

static HakkaHandle move_to_hakka_handle(JsonHandleCompact &&handle)
{
    static_assert(sizeof(JsonHandleCompact) <= sizeof(HakkaHandle));
    auto inc_ref = [](const auto *elem) { return elem->inc_ref_impl(); };
    dispatch<decltype(inc_ref), uint64_t>(handle.get_view(), std::forward<decltype(inc_ref)>(inc_ref));
    return static_cast<HakkaHandle>(handle);
}

// helper that extracts compact types from variant
template <typename T>
static const T *get_compact_ptr(const JsonHandleCompact &handle)
{
    try {
        auto view = handle.get_view();
        return std::get<const T*>(view);
    } catch (...) {
        return nullptr;
    }
}

#define CHECK_INVALID_ARGUMENT(invalid_expr)    \
    do                                          \
    {                                           \
        if (invalid_expr)                       \
        {                                       \
            return HAKKA_JSON_INVALID_ARGUMENT; \
        }                                       \
    } while (0)

#define HANDLE_CHECK_INVALID_ARGUMENT(handle) CHECK_INVALID_ARGUMENT((handle->is_valid() == false))

extern_c HakkaJsonResultEnum CreateHakkaObject(HakkaHandle *handle)
{
    CHECK_INVALID_ARGUMENT(handle == nullptr);

    auto result = JsonObjectCompact::create();
    if (!result.is_valid())
        return HAKKA_JSON_INTERNAL_ERROR;

    *handle = move_to_hakka_handle(std::move(result));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum LoadsHakkaObject(const uint8_t *json_str, uint32_t json_length, HakkaHandle *handle, uint32_t max_depth)
{
    CHECK_INVALID_ARGUMENT(json_str == nullptr || handle == nullptr);

    auto result = JsonObjectCompact::loads(std::string_view(reinterpret_cast<const char *>(json_str), json_length), max_depth);
    if (!result)
        return result.error();

    *handle = move_to_hakka_handle(std::move(result.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum DumpHakkaObject(HakkaHandle object, uint32_t max_depth, uint8_t *buffer, uint64_t *buffer_size)
{
    CHECK_INVALID_ARGUMENT(buffer == nullptr || buffer_size == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    const JsonObjectCompact *json_object = to_json_object_compact(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    auto dump_result = json_object->dump_impl(max_depth);
    if (!dump_result)
        return dump_result.error();

    const std::string &dumped_str = dump_result.value();
    if (dumped_str.size() > *buffer_size)
    {
        *buffer_size = static_cast<uint64_t>(dumped_str.size());
        return HAKKA_JSON_NOT_ENOUGH_MEMORY;
    }

    std::memcpy(buffer, dumped_str.c_str(), dumped_str.size());
    *buffer_size = static_cast<uint64_t>(dumped_str.size());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum SetHakkaObjectInt(HakkaHandle object, const uint8_t *key, uint32_t key_length, int64_t value)
{
    CHECK_INVALID_ARGUMENT(key == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    auto int_handle = JsonIntCompact::create(value);
    if (!int_handle.is_valid())
        return HAKKA_JSON_INTERNAL_ERROR;

    return json_object->set_impl(str_key, int_handle);
}

extern_c HakkaJsonResultEnum SetHakkaObjectFloat(HakkaHandle object, const uint8_t *key, uint32_t key_length, double value)
{
    CHECK_INVALID_ARGUMENT(key == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    auto float_handle = JsonFloatCompact::create(value);
    if (!float_handle.is_valid())
        return HAKKA_JSON_INTERNAL_ERROR;

    return json_object->set_impl(str_key, float_handle);
}

extern_c HakkaJsonResultEnum SetHakkaObjectString(HakkaHandle object, const uint8_t *key, uint32_t key_length, const uint8_t *value, uint32_t value_length)
{
    CHECK_INVALID_ARGUMENT(key == nullptr || value == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    std::string_view str_value(reinterpret_cast<const char *>(value), value_length);
    auto string_handle = JsonStringCompact::create(str_value);
    if (!string_handle.is_valid())
        return HAKKA_JSON_INTERNAL_ERROR;

    return json_object->set_impl(str_key, string_handle);
}

extern_c HakkaJsonResultEnum SetHakkaObjectNull(HakkaHandle object, const uint8_t *key, uint32_t key_length)
{
    CHECK_INVALID_ARGUMENT(key == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    auto null_handle = JsonFloatCompact::create(nullptr);
    if (!null_handle.is_valid())
        return HAKKA_JSON_INTERNAL_ERROR;

    return json_object->set_impl(str_key, null_handle);
}

extern_c HakkaJsonResultEnum GetHakkaObjectInt(HakkaHandle object, const uint8_t *key, uint32_t key_length, int64_t *value)
{
    CHECK_INVALID_ARGUMENT(key == nullptr || value == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    const JsonObjectCompact *json_object = to_json_object_compact(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    auto result = json_object->get_impl(str_key);
    if (!result)
    {
        return result.error();
    }

    if (result.value().get_type() != HakkaJsonType::HAKKA_JSON_INT)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonIntCompact *json_int = get_compact_ptr<JsonIntCompact>(result.value());
    if (!json_int)
        return HAKKA_JSON_INTERNAL_ERROR;

    auto int_result = json_int->get_impl();
    if (!int_result)
        return int_result.error();

    if (!std::holds_alternative<int64_t>(int_result.value()))
        return HAKKA_JSON_INTERNAL_ERROR;

    *value = std::get<int64_t>(int_result.value());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaObjectFloat(HakkaHandle object, const uint8_t *key, uint32_t key_length, double *value)
{
    CHECK_INVALID_ARGUMENT(key == nullptr || value == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    const JsonObjectCompact *json_object = to_json_object_compact(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    auto result = json_object->get_impl(str_key);
    if (!result)
    {
        return result.error();
    }

    if (result.value().get_type() != HakkaJsonType::HAKKA_JSON_FLOAT)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonFloatCompact *json_float = get_compact_ptr<JsonFloatCompact>(result.value());
    if (!json_float)
        return HAKKA_JSON_INTERNAL_ERROR;

    auto float_result = json_float->get_impl();
    if (!float_result)
        return float_result.error();

    if (!std::holds_alternative<double>(float_result.value()))
        return HAKKA_JSON_INTERNAL_ERROR;

    *value = std::get<double>(float_result.value());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaObjectString(HakkaHandle object, const uint8_t *key, uint32_t key_length, uint8_t *buffer, uint32_t *buffer_size)
{
    CHECK_INVALID_ARGUMENT(key == nullptr || buffer == nullptr || buffer_size == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    auto result = json_object->get(str_key);
    if (!result)
    {
        return result.error();
    }

    if (result.value().get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(result.value());
    auto store_string = [&buffer, &buffer_size](const PrimitiveType &input)
    {
        if (std::holds_alternative<std::string>(input) == false)
        {
            return HAKKA_JSON_INTERNAL_ERROR;
        }

        const auto &str = std::get<std::string>(input);
        if (str.size() + 1 > *buffer_size)
        {
            *buffer_size = static_cast<uint32_t>(str.size() + 1);
            return HAKKA_JSON_NOT_ENOUGH_MEMORY;
        }

        std::memcpy(buffer, str.c_str(), str.size());
        buffer[str.size()] = '\0';
        return HAKKA_JSON_SUCCESS;
    };

    json_string->get().map(store_string);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaObjectNull(HakkaHandle object, const uint8_t *key, uint32_t key_length, C_BOOL *result)
{
    CHECK_INVALID_ARGUMENT(key == nullptr || result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    auto null_result = json_object->get(str_key);
    if (!null_result)
    {
        return null_result.error();
    }

    *result = null_result.value().get_type() == HakkaJsonType::HAKKA_JSON_NULL;
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaObjectObject(HakkaHandle object, const uint8_t *key, uint32_t key_length, HakkaHandle *value)
{
    CHECK_INVALID_ARGUMENT(key == nullptr || value == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    auto result = json_object->get(str_key);
    if (!result)
    {
        return result.error();
    }

    *value = move_to_hakka_handle(std::move(result.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum SetHakkaObject(HakkaHandle object, const uint8_t *key, uint32_t key_length, HakkaHandle value)
{
    CHECK_INVALID_ARGUMENT(key == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    return json_object->set_impl(str_key, *to_json_handle_compact(&value));
}

extern_c HakkaJsonResultEnum RemoveHakkaObjectKey(HakkaHandle object, const uint8_t *key, uint32_t key_length)
{
    CHECK_INVALID_ARGUMENT(key == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    return json_object->remove_impl(str_key);
}

extern_c HakkaJsonResultEnum GetHakkaObjectSize(HakkaHandle object, uint32_t *size)
{
    CHECK_INVALID_ARGUMENT(size == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    *size = json_object->length();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum ContainsHakkaObjectKey(HakkaHandle object, const uint8_t *key, uint32_t key_length, C_BOOL *result)
{
    CHECK_INVALID_ARGUMENT(key == nullptr || result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    *result = json_object->contains(str_key);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaObjectKeys(HakkaHandle object, HakkaHandle *keys)
{
    CHECK_INVALID_ARGUMENT(keys == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    *keys = move_to_hakka_handle(json_object->keys_handle());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaObjectValues(HakkaHandle object, HakkaHandle *values)
{
    CHECK_INVALID_ARGUMENT(values == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    *values = move_to_hakka_handle(json_object->values_handle());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum CreateHakkaObjectFromKeys(HakkaHandle string_array_handle,
                                                       HakkaHandle default_value,
                                                       HakkaHandle *result)

{
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&string_array_handle));
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&default_value));
    // check string_array_handle is array type
    if (to_json_handle_compact(&string_array_handle)->get_type() != HakkaJsonType::HAKKA_JSON_ARRAY)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonArrayCompact *string_array = get_compact_ptr<JsonArrayCompact>(*to_json_handle_compact(&string_array_handle));
    if (string_array == nullptr)
        return HAKKA_JSON_INTERNAL_ERROR;

    std::vector<KeyType> keys;
    for (size_t i = 0; i < string_array->length(); ++i)
    {
        auto key_handle = string_array->at(i);
        if (!key_handle || key_handle.value().get_type() != HakkaJsonType::HAKKA_JSON_STRING)
            return HAKKA_JSON_TYPE_ERROR;

        const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(key_handle.value());
        keys.push_back(std::get<std::string>(json_string->get().value()));
    }

    auto result_handle = JsonObjectCompact::fromkeys(keys, *to_json_handle_compact(&default_value));
    if (!result_handle)
        return result_handle.error();

    *result = move_to_hakka_handle(std::move(result_handle.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum PopHakkaObject(HakkaHandle object, const uint8_t *key, uint32_t key_length, HakkaHandle *value)
{
    CHECK_INVALID_ARGUMENT(key == nullptr || value == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    std::string str_key(reinterpret_cast<const char *>(key), key_length);
    auto result = json_object->pop(str_key);
    if (!result)
    {
        return result.error();
    }

    *value = move_to_hakka_handle(std::move(result.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum PopItemHakkaObject(HakkaHandle object, HakkaHandle *key, HakkaHandle *value)
{
    CHECK_INVALID_ARGUMENT(key == nullptr || value == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    auto result = json_object->popitem();
    if (!result)
    {
        return result.error();
    }

    JsonHandleCompact key_handle;
    {
        auto key_content = result.value().first;
        if (std::holds_alternative<std::string>(key_content) == false)
            return HAKKA_JSON_INTERNAL_ERROR;

        std::string key_str = std::get<std::string>(key_content);
        key_handle = JsonStringCompact::create(key_str);
    }

    *key = move_to_hakka_handle(std::move(key_handle));
    *value = move_to_hakka_handle(std::move(result.value().second));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum ClearHakkaObject(HakkaHandle object)
{
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    json_object->clear_impl();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum UpdateHakkaObject(HakkaHandle object, HakkaHandle other)
{
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&other));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    JsonObjectCompact *json_other = to_json_object_compact_mut(&other);
    if (json_object == nullptr || json_other == nullptr)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    return json_object->update(*json_other);
}

class HakkaObjectMoveIter
{
public:
    HakkaObjectMoveIter(JsonObjectIterCompact &&iter) : iter_(std::move(iter)) {}

    HakkaObjectMoveIter &operator++()
    {
        ++iter_;
        return *this;
    }

    HakkaObjectMoveIter operator++(int)
    {
        HakkaObjectMoveIter temp = *this;
        ++(*this);
        return temp;
    }

    bool operator==(const HakkaObjectMoveIter &other) const
    {
        return iter_ == other.iter_;
    }

    bool operator!=(const HakkaObjectMoveIter &other) const
    {
        return !(*this == other);
    }

    std::pair<std::string, JsonHandleCompact> operator*()
    {
        auto cur = iter_.operator->();
        return {std::get<std::string>(cur->first), std::move(cur->second)};
    }

    bool is_end() const
    {
        return iter_.is_end();
    }

private:
    JsonObjectIterCompact iter_;
};

extern_c HakkaJsonResultEnum CreateHakkaObjectIterBegin(HakkaHandle object, HakkaObjectIter *iter)
{
    CHECK_INVALID_ARGUMENT(iter == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&object));

    JsonObjectCompact *json_object = to_json_object_compact_mut(&object);
    if (json_object == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    *iter = reinterpret_cast<HakkaObjectIter>(new (std::nothrow) HakkaObjectMoveIter(json_object->begin()));
    if (*iter == 0)
        return HAKKA_JSON_NOT_ENOUGH_MEMORY;

    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum MoveHakkaObjectIterNext(HakkaObjectIter iter)
{
    CHECK_INVALID_ARGUMENT(iter == 0);

    auto iter_ = reinterpret_cast<HakkaObjectMoveIter *>(iter);
    ++(*iter_);

    if (iter_->is_end())
        return HAKKA_JSON_ITERATOR_END;

    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaObjectIterDeref(HakkaObjectIter iter, HakkaHandle *key, HakkaHandle *value)
{
    CHECK_INVALID_ARGUMENT(iter == 0 || key == nullptr || value == nullptr);

    auto iter_ = reinterpret_cast<HakkaObjectMoveIter *>(iter);
    if (iter_->is_end())
        return HAKKA_JSON_ITERATOR_END;

    auto deref = **iter_;
    *key = move_to_hakka_handle(JsonStringCompact::create(deref.first));
    *value = move_to_hakka_handle(std::move(deref.second));

    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum HakkaObjectIterRelease(HakkaObjectIter *iter)
{
    CHECK_INVALID_ARGUMENT(iter == nullptr || *iter == 0);

    delete reinterpret_cast<HakkaObjectMoveIter *>(*iter);
    *iter = 0;
    return HAKKA_JSON_SUCCESS;
}
