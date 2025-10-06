#include <hakka_json_primitive.h>
#include <hakka_json_string.hpp>
#include <hakka_json_int.hpp>
#include <hakka_json_float.hpp>
#include <hakka_json_array.hpp>

#include <hakka_json_object.hpp>
#include <hakka_json_array.hpp>

#include <cstring>
#include <string>
#include <string_view>

using namespace hakka;

namespace JsonBaseCompactCapi{
    auto inc_ref = [](const auto* p) { return p->inc_ref_impl(); };
    auto dec_ref = [](const auto* p) { return p->dec_ref_impl(); };
    auto dump = [](const auto* p, uint32_t max_depth) { return p->dump_impl(max_depth); };
    auto to_bytes = [](const auto* p, char *buffer, uint32_t *buffer_size) { return p->to_bytes_impl(buffer, buffer_size); };
    auto compare = [](const auto* p, const JsonHandleCompact &other) { return p->compare_impl(other); };
    auto hash = [](const auto* p) { return p->hash_impl(); };
    auto dump_size = [](const auto* p) { return p->dump_size_impl(); };
}

// Conversion functions between HakkaHandle and JsonHandleCompact
// Ensure that the lifetime of JsonHandleCompact is managed correctly, releasing the object when JsonHandleCompact is released
static JsonHandleCompact *to_json_handle_compact(HakkaHandle *handle)
{
    static_assert(sizeof(JsonHandleCompact) <= sizeof(HakkaHandle));
    return reinterpret_cast<JsonHandleCompact *>(handle);
}

static HakkaHandle move_to_hakka_handle(JsonHandleCompact &&handle)
{
    static_assert(sizeof(JsonHandleCompact) <= sizeof(HakkaHandle));
    dispatch<decltype(JsonBaseCompactCapi::inc_ref), uint64_t>(handle.get_view(), 
        std::forward<decltype(JsonBaseCompactCapi::inc_ref)>(JsonBaseCompactCapi::inc_ref));
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

// Helper to call methods through dispatch pattern
template <typename RetType, typename Func, typename... Args>
static tl::expected<RetType, HakkaJsonResultEnum> call_compact_method(const JsonHandleCompact &handle, Func&& func, Args&&... args)
{
    auto invoker = [&func](const auto *elem, Args&&... args) -> decltype(auto) {
        return std::invoke(func, elem, std::forward<Args>(args)...);
    };
    
    return dispatch<decltype(invoker), RetType>(
        handle.get_view(), 
        std::forward<decltype(invoker)>(invoker),
        std::forward<Args>(args)...
    );
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

extern_c void HakkaRelease(HakkaHandle *handle)
{
    if (handle == nullptr || *handle == 0)
        return;

    JsonHandleCompact self_handle = *to_json_handle_compact(handle); // RAII will handle the release
    *handle = HakkaHandle{0};

    // The self_handle holds a reference count, and the caller also holds a reference count.
    // The reference count will be decremented twice: once for self_handle and once for the caller.
    dispatch<decltype(JsonBaseCompactCapi::dec_ref), uint64_t>(self_handle.get_view(), 
        std::forward<decltype(JsonBaseCompactCapi::dec_ref)>(JsonBaseCompactCapi::dec_ref));
}

extern_c HakkaJsonResultEnum HakkaDump(HakkaHandle handle, uint32_t max_depth, uint8_t *buffer, uint64_t *buffer_size)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(buffer == nullptr || buffer_size == nullptr);
    // HANDLE_CHECK_INVALID_ARGUMENT(self_handle);, ONLY dump can be called on invalid objects

    // NOTE: if self_handle is zero, it means that the object is JsonInvalid
    auto result = dispatch<decltype(JsonBaseCompactCapi::dump), std::string>(self_handle->get_view(), 
        std::forward<decltype(JsonBaseCompactCapi::dump)>(JsonBaseCompactCapi::dump), max_depth);
    if (!result)
        return result.error();

    const auto &dumped = result.value();
    if (dumped.size() > *buffer_size)
    {
        *buffer_size = static_cast<uint64_t>(dumped.size());
        return HAKKA_JSON_NOT_ENOUGH_MEMORY;
    }

    std::memcpy(buffer, dumped.c_str(), dumped.size());
    *buffer_size = static_cast<uint64_t>(dumped.size());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum HakkaToBytes(HakkaHandle handle, uint8_t *buffer, uint32_t *buffer_size)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(buffer == nullptr || buffer_size == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    auto result = dispatch<decltype(JsonBaseCompactCapi::to_bytes), HakkaJsonResultEnum>(self_handle->get_view(), 
        std::forward<decltype(JsonBaseCompactCapi::to_bytes)>(JsonBaseCompactCapi::to_bytes), 
        reinterpret_cast<char *>(buffer), buffer_size);
    if (!result)
        return result.error();

    return result.value();
}

extern_c HakkaJsonResultEnum HakkaIsValid(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);

    *result = self_handle->is_valid();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum HakkaType(HakkaHandle handle, HakkaJsonType *type)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(type == nullptr);
    // HANDLE_CHECK_INVALID_ARGUMENT(self_handle);, ONLY dump can be called on invalid objects

    // NOTE: if self_handle is zero, it means that the object is JsonInvalid
    *type = self_handle->get_type();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum HakkaCompare(HakkaHandle handle, HakkaHandle other, int32_t *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    JsonHandleCompact *other_handle = to_json_handle_compact(&other);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);
    HANDLE_CHECK_INVALID_ARGUMENT(other_handle);

    auto cmp_result = dispatch<decltype(JsonBaseCompactCapi::compare), int>(self_handle->get_view(), 
        std::forward<decltype(JsonBaseCompactCapi::compare)>(JsonBaseCompactCapi::compare), *other_handle);
    if (!cmp_result)
        return cmp_result.error();

    *result = cmp_result.value();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum HakkaHash(HakkaHandle handle, uint64_t *hash)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(hash == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    auto hash_result = dispatch<decltype(JsonBaseCompactCapi::hash), uint64_t>(self_handle->get_view(), 
        std::forward<decltype(JsonBaseCompactCapi::hash)>(JsonBaseCompactCapi::hash));
    if (!hash_result)
        return hash_result.error();

    *hash = hash_result.value();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum HakkaDumpSize(HakkaHandle handle, uint64_t *capacity)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(capacity == nullptr);
    // HANDLE_CHECK_INVALID_ARGUMENT(self_handle);, ONLY dump can be called on invalid objects

    // NOTE: if self_handle is zero, it means that the object is JsonInvalid
    auto dump_size_result = dispatch<decltype(JsonBaseCompactCapi::dump_size), uint64_t>(self_handle->get_view(), 
        std::forward<decltype(JsonBaseCompactCapi::dump_size)>(JsonBaseCompactCapi::dump_size));
    if (!dump_size_result)
        return dump_size_result.error();

    *capacity = dump_size_result.value();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum HakkaReclaim(HakkaHandle handle)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    dispatch<decltype(JsonBaseCompactCapi::inc_ref), uint64_t>(self_handle->get_view(), 
        std::forward<decltype(JsonBaseCompactCapi::inc_ref)>(JsonBaseCompactCapi::inc_ref));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum CreateHakkaInt(HakkaHandle *handle, int64_t value)
{
    CHECK_INVALID_ARGUMENT(handle == nullptr);

    auto result = JsonIntCompact::create(value);
    if (!result.is_valid())
        return HAKKA_JSON_INTERNAL_ERROR;

    *handle = move_to_hakka_handle(std::move(result));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum CreateHakkaFloat(HakkaHandle *handle, double value)
{
    CHECK_INVALID_ARGUMENT(handle == nullptr);

    auto result = JsonFloatCompact::create(value);
    if (!result.is_valid())
        return HAKKA_JSON_INTERNAL_ERROR;

    *handle = move_to_hakka_handle(std::move(result));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum CreateHakkaNull(HakkaHandle *handle)
{
    CHECK_INVALID_ARGUMENT(handle == nullptr);

    auto result = JsonFloatCompact::create(nullptr);
    if (!result.is_valid())
        return HAKKA_JSON_INTERNAL_ERROR;

    *handle = move_to_hakka_handle(std::move(result));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum CreateHakkaBool(HakkaHandle *handle, C_BOOL value)
{
    CHECK_INVALID_ARGUMENT(handle == nullptr);

    auto result = JsonFloatCompact::create(static_cast<bool>(value));
    if (!result.is_valid())
        return HAKKA_JSON_INTERNAL_ERROR;

    *handle = move_to_hakka_handle(std::move(result));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum CreateHakkaInvalid(HakkaHandle *handle)
{
    CHECK_INVALID_ARGUMENT(handle == nullptr);

    // Invalid handle is represented as 0
    *handle = HakkaHandle{0};
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaInt(HakkaHandle handle, int64_t *value)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(value == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_INT)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonIntCompact *json_int = get_compact_ptr<JsonIntCompact>(*self_handle);
    if (!json_int)
        return HAKKA_JSON_INTERNAL_ERROR;

    auto result = json_int->get_impl();
    if (!result)
        return result.error();

    if (!std::holds_alternative<int64_t>(result.value()))
        return HAKKA_JSON_INTERNAL_ERROR;

    *value = std::get<int64_t>(result.value());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaFloat(HakkaHandle handle, double *value)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(value == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_FLOAT)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonFloatCompact *json_float = get_compact_ptr<JsonFloatCompact>(*self_handle);
    if (!json_float)
        return HAKKA_JSON_INTERNAL_ERROR;

    auto result = json_float->get_impl();
    if (!result)
        return result.error();

    if (!std::holds_alternative<double>(result.value()))
        return HAKKA_JSON_INTERNAL_ERROR;

    *value = std::get<double>(result.value());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaBool(HakkaHandle handle, C_BOOL *value)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(value == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_BOOL)
        return HAKKA_JSON_TYPE_ERROR;

    // JsonBool is NaN-boxed in JsonFloatCompact
    const JsonFloatCompact *json_float = reinterpret_cast<const JsonFloatCompact*>(get_compact_ptr<const JsonBoolCompact>(*self_handle));
    if (!json_float)
        return HAKKA_JSON_INTERNAL_ERROR;

    auto result = json_float->get_impl();
    if (!result)
        return result.error();

    if (!std::holds_alternative<bool>(result.value()))
        return HAKKA_JSON_INTERNAL_ERROR;

    *value = std::get<bool>(result.value());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum CreateHakkaString(HakkaHandle *handle, const uint8_t *value, uint32_t length)
{
    CHECK_INVALID_ARGUMENT(handle == nullptr || value == nullptr);

    auto result = JsonStringCompact::create(std::string_view(reinterpret_cast<const char *>(value), length));
    if (!result.is_valid())
        return HAKKA_JSON_INTERNAL_ERROR;

    *handle = move_to_hakka_handle(std::move(result));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaString(HakkaHandle handle, uint8_t *buffer, uint32_t *buffer_size)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(buffer == nullptr || buffer_size == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;

    auto result = json_string->get_impl();
    if (!result)
        return result.error();

    if (!std::holds_alternative<std::string>(result.value()))
        return HAKKA_JSON_INTERNAL_ERROR;

    const auto &str = std::get<std::string>(result.value());
    if (str.size() + 1 > *buffer_size)
    {
        *buffer_size = static_cast<uint32_t>(str.size() + 1);
        return HAKKA_JSON_NOT_ENOUGH_MEMORY;
    }

    std::memcpy(buffer, str.c_str(), str.size());
    buffer[str.size()] = '\0';
    *buffer_size = static_cast<uint32_t>(str.size());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringLength(HakkaHandle handle, uint32_t *length)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(length == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;

    auto result = json_string->length();
    if (!result)
        return result.error();

    *length = static_cast<uint32_t>(result.value());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringCapitalize(HakkaHandle handle, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto capitalized = json_string->capitalize();
    if (!capitalized)
        return capitalized.error();

    *result = move_to_hakka_handle(std::move(capitalized.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringCasefold(HakkaHandle handle, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto casefolded = json_string->casefold();
    if (!casefolded)
        return casefolded.error();

    *result = move_to_hakka_handle(std::move(casefolded.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringCount(HakkaHandle handle, const uint8_t *substring, uint32_t substring_length, int64_t *count)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(substring == nullptr || count == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto counted = json_string->count(std::string_view(reinterpret_cast<const char *>(substring), substring_length));
    if (!counted)
        return counted.error();

    *count = counted.value();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringEndswith(HakkaHandle handle, const uint8_t *suffix, uint32_t suffix_length, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(suffix == nullptr || result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto ends_with = json_string->endswith(std::string_view(reinterpret_cast<const char *>(suffix), suffix_length));
    if (!ends_with)
        return ends_with.error();

    *result = ends_with.value();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringFind(HakkaHandle handle, const uint8_t *substring, uint32_t substring_length, int64_t *position)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(substring == nullptr || position == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto found = json_string->find(std::string_view(reinterpret_cast<const char *>(substring), substring_length));
    if (!found)
        return found.error();

    *position = found.value();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringConcatenate(HakkaHandle handle, const uint8_t *other, uint32_t other_length, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(other == nullptr || result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto concatenated = json_string->concatenate(std::string_view(reinterpret_cast<const char *>(other), other_length));
    if (!concatenated)
        return concatenated.error();

    *result = move_to_hakka_handle(std::move(concatenated.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringMultiply(HakkaHandle handle, int64_t times, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto multiplied = json_string->multiply(times);
    if (!multiplied)
        return multiplied.error();

    *result = move_to_hakka_handle(std::move(multiplied.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringSlice(HakkaHandle handle, int64_t start, int64_t end, int64_t step, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto sliced = json_string->slice(start, end, step);
    if (!sliced)
        return sliced.error();

    *result = move_to_hakka_handle(std::move(sliced.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringLower(HakkaHandle handle, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto lowered = json_string->lower();
    if (!lowered)
        return lowered.error();

    *result = move_to_hakka_handle(std::move(lowered.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringRemoveprefix(HakkaHandle handle, const uint8_t *prefix, uint32_t prefix_length, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(prefix == nullptr || result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto removed = json_string->removeprefix(std::string_view(reinterpret_cast<const char *>(prefix), prefix_length));
    if (!removed)
        return removed.error();

    *result = move_to_hakka_handle(std::move(removed.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringRemovesuffix(HakkaHandle handle, const uint8_t *suffix, uint32_t suffix_length, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(suffix == nullptr || result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto removed = json_string->removesuffix(std::string_view(reinterpret_cast<const char *>(suffix), suffix_length));
    if (!removed)
        return removed.error();

    *result = move_to_hakka_handle(std::move(removed.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringReplace(HakkaHandle handle, const uint8_t *old_substr, uint32_t old_substr_length, const uint8_t *new_substr, uint32_t new_substr_length, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(old_substr == nullptr || new_substr == nullptr || result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto replaced = json_string->replace(std::string_view(reinterpret_cast<const char *>(old_substr), old_substr_length),
                                         std::string_view(reinterpret_cast<const char *>(new_substr), new_substr_length));
    if (!replaced)
        return replaced.error();

    *result = move_to_hakka_handle(std::move(replaced.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringRfind(HakkaHandle handle, const uint8_t *substring, uint32_t substring_length, int64_t *position)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(substring == nullptr || position == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto found = json_string->rfind(std::string_view(reinterpret_cast<const char *>(substring), substring_length));
    if (!found)
        return found.error();

    *position = found.value();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringRsplit(HakkaHandle handle, const uint8_t *separator, uint32_t separator_length, int64_t maxsplit, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(separator == nullptr || result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto rsplit = json_string->rsplit(std::string_view(reinterpret_cast<const char *>(separator), separator_length), maxsplit);
    if (!rsplit)
        return rsplit.error();

    *result = move_to_hakka_handle(std::move(rsplit.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringSplit(HakkaHandle handle, const uint8_t *separator, uint32_t separator_length, int64_t maxsplit, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(separator == nullptr || result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto split = json_string->split(std::string_view(reinterpret_cast<const char *>(separator), separator_length), maxsplit);
    if (!split)
        return split.error();

    *result = move_to_hakka_handle(std::move(split.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringSplitlines(HakkaHandle handle, C_BOOL keepends, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto splitlines = json_string->splitlines(keepends);
    if (!splitlines)
        return splitlines.error();

    *result = move_to_hakka_handle(std::move(splitlines.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringStartswith(HakkaHandle handle, const uint8_t *prefix, uint32_t prefix_length, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(prefix == nullptr || result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto starts_with = json_string->startswith(std::string_view(reinterpret_cast<const char *>(prefix), prefix_length));
    if (!starts_with)
        return starts_with.error();

    *result = starts_with.value();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringUpper(HakkaHandle handle, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto uppered = json_string->upper();
    if (!uppered)
        return uppered.error();

    *result = move_to_hakka_handle(std::move(uppered.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringSwapcase(HakkaHandle handle, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto swapped = json_string->swapcase();
    if (!swapped)
        return swapped.error();

    *result = move_to_hakka_handle(std::move(swapped.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringTitle(HakkaHandle handle, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto titled = json_string->title();
    if (!titled)
        return titled.error();

    *result = move_to_hakka_handle(std::move(titled.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringZfill(HakkaHandle handle, int64_t width, HakkaHandle *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto zfilled = json_string->zfill(width);
    if (!zfilled)
        return zfilled.error();

    *result = move_to_hakka_handle(std::move(zfilled.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringUTF8Length(HakkaHandle handle, uint64_t *length)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(length == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    auto utf8_length = json_string->utf8_length();
    if (!utf8_length)
        return utf8_length.error();

    *length = utf8_length.value();
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringIsalnum(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *result = json_string->isalnum().value_or(false);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringIsalpha(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *result = json_string->isalpha().value_or(false);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringIsascii(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *result = json_string->isascii().value_or(false);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringIsdecimal(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *result = json_string->isdecimal().value_or(false);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringIsdigit(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *result = json_string->isdigit().value_or(false);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringIsidentifier(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *result = json_string->isidentifier().value_or(false);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringIslower(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *result = json_string->islower().value_or(false);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringIsnumeric(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *result = json_string->isnumeric().value_or(false);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringIsprintable(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *result = json_string->isprintable().value_or(false);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringIsspace(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *result = json_string->isspace().value_or(false);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringIstitle(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *result = json_string->istitle().value_or(false);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringIsupper(HakkaHandle handle, C_BOOL *result)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&handle);
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
    {
        return HAKKA_JSON_TYPE_ERROR;
    }

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *result = json_string->isupper().value_or(false);
    return HAKKA_JSON_SUCCESS;
}

// string iterator
class HakkaStringMoveIterCompact
{
public:
    HakkaStringMoveIterCompact(JsonStringCompactIter &&begin_, JsonStringCompactIter &&end_)
        : iter(std::move(begin_)), end(std::move(end_)) {}
    ~HakkaStringMoveIterCompact() = default;
    HakkaStringMoveIterCompact(const HakkaStringMoveIterCompact &) = delete;
    HakkaStringMoveIterCompact &operator=(const HakkaStringMoveIterCompact &) = delete;
    HakkaStringMoveIterCompact(HakkaStringMoveIterCompact &&) = default;
    HakkaStringMoveIterCompact &operator=(HakkaStringMoveIterCompact &&) = default;

    JsonStringCompactIter::reference operator*() { return *iter; }
    HakkaStringMoveIterCompact &operator++()
    {
        if (iter != end)
            ++iter;
        return *this;
    }

    bool operator==(const HakkaStringMoveIterCompact &other) const { return iter == other.iter; }
    bool operator!=(const HakkaStringMoveIterCompact &other) const { return iter != other.iter; }

    bool is_end() const { return iter == end; }

private:
    JsonStringCompactIter iter;
    JsonStringCompactIter end;
};

extern_c HakkaJsonResultEnum CreateHakkaStringBegin(HakkaHandle str_handle, HakkaStringIter *iter)
{
    JsonHandleCompact *self_handle = to_json_handle_compact(&str_handle);
    CHECK_INVALID_ARGUMENT(iter == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(self_handle);

    if (self_handle->get_type() != HakkaJsonType::HAKKA_JSON_STRING)
        return HAKKA_JSON_TYPE_ERROR;

    const JsonStringCompact *json_string = get_compact_ptr<JsonStringCompact>(*self_handle);
    if (!json_string)
        return HAKKA_JSON_INTERNAL_ERROR;
    *iter = reinterpret_cast<HakkaStringIter>(new (std::nothrow) HakkaStringMoveIterCompact(json_string->begin(),
                                                                                           json_string->end()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum MoveHakkaStringNext(HakkaStringIter iter)
{
    CHECK_INVALID_ARGUMENT(iter == 0);

    auto iter_ = reinterpret_cast<HakkaStringMoveIterCompact *>(iter);
    ++(*iter_);

    if (iter_->is_end())
        return HAKKA_JSON_ITERATOR_END;
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaStringDeref(HakkaStringIter iter, uint32_t *utf32)
{
    CHECK_INVALID_ARGUMENT(iter == 0 || utf32 == nullptr);

    auto iter_ = reinterpret_cast<HakkaStringMoveIterCompact *>(iter);
    if (iter_->is_end())
        return HAKKA_JSON_ITERATOR_END;

    *utf32 = static_cast<uint32_t>(**iter_);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum HakkaStringIterRelease(HakkaStringIter *iter)
{
    CHECK_INVALID_ARGUMENT(iter == nullptr || *iter == 0);

    delete reinterpret_cast<HakkaStringMoveIterCompact *>(*iter);
    *iter = 0;
    return HAKKA_JSON_SUCCESS;
}
