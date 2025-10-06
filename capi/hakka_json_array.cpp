#include <hakka_json_array.h>

#include <hakka_json_string.hpp>
#include <hakka_json_int.hpp>
#include <hakka_json_float.hpp>
#include <hakka_json_array.hpp>
#include <hakka_json_object.hpp>

#include <variant>
#include <uniform_compact_pointer.hpp>

using namespace hakka;

static JsonHandleCompact *to_json_handle_compact(HakkaHandle *handle)
{
    static_assert(sizeof(JsonHandleCompact) <= sizeof(HakkaHandle));
    return reinterpret_cast<JsonHandleCompact *>(handle);
}

static const JsonArrayCompact *to_json_array_compact(HakkaHandle *handle)
{
    JsonHandleCompact *json_handle = to_json_handle_compact(handle);
    if (json_handle->get_type() != HakkaJsonType::HAKKA_JSON_ARRAY)
        return nullptr;
    
    try {
        auto view = json_handle->get_view();
        return std::get<const JsonArrayCompact*>(view);
    } catch (...) {
        return nullptr;
    }
}

static JsonArrayCompact *to_json_array_compact_mut(HakkaHandle *handle)
{
    JsonHandleCompact *json_handle = to_json_handle_compact(handle);
    if (json_handle->get_type() != HakkaJsonType::HAKKA_JSON_ARRAY)
        return nullptr;
    
    try {
        auto mut_ptr = json_handle->get_mut_ptr();
        return std::get<JsonArrayCompact*>(mut_ptr);
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

extern_c HakkaJsonResultEnum CreateHakkaArray(HakkaHandle *handle)
{
    CHECK_INVALID_ARGUMENT(handle == nullptr);

    auto result = JsonArrayCompact::create();
    if (!result.is_valid())
        return HAKKA_JSON_INTERNAL_ERROR;

    *handle = move_to_hakka_handle(std::move(result));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum LoadsHakkaArray(const uint8_t *json_str, uint32_t json_length, HakkaHandle *handle, uint32_t max_depth)
{
    CHECK_INVALID_ARGUMENT(json_str == nullptr || handle == nullptr);

    auto result = JsonArrayCompact::loads(std::string_view(reinterpret_cast<const char *>(json_str), json_length), max_depth);
    if (!result)
        return result.error();

    *handle = move_to_hakka_handle(std::move(result.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum DumpHakkaArray(HakkaHandle array, uint32_t max_depth, uint8_t *buffer, uint64_t *buffer_size)
{
    CHECK_INVALID_ARGUMENT(buffer == nullptr || buffer_size == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));

    const JsonArrayCompact *json_array = to_json_array_compact(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    auto result = json_array->dump_impl(max_depth);
    if (!result)
        return result.error();

    if (result.value().size() > *buffer_size)
    {
        *buffer_size = static_cast<uint64_t>(result.value().size());
        return HAKKA_JSON_NOT_ENOUGH_MEMORY;
    }

    *buffer_size = static_cast<uint64_t>(result.value().size());
    std::memcpy(buffer, result.value().c_str(), result.value().size());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaArrayObject(HakkaHandle array, uint32_t index, HakkaHandle *value)
{
    CHECK_INVALID_ARGUMENT(value == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));

    const JsonArrayCompact *json_array = to_json_array_compact(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    auto result = json_array->at_impl(index);
    if (!result)
        return result.error();

    *value = move_to_hakka_handle(std::move(result.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum SetHakkaArray(HakkaHandle array, uint32_t index, HakkaHandle value)
{
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));

    JsonArrayCompact *json_array = to_json_array_compact_mut(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    return json_array->set_impl(index, *to_json_handle_compact(&value));
}

extern_c HakkaJsonResultEnum GetHakkaArraySlice(HakkaHandle array, int64_t start, int64_t end, int64_t step, HakkaHandle *result)
{
    CHECK_INVALID_ARGUMENT(result == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));

    const JsonArrayCompact *json_array = to_json_array_compact(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    auto slice_result = json_array->get_slice(start, end, step);
    if (!slice_result)
        return slice_result.error();

    *result = move_to_hakka_handle(std::move(slice_result.value()));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum SetHakkaArraySlice(HakkaHandle dst, int64_t start, int64_t end, int64_t step, HakkaHandle src)
{
    CHECK_INVALID_ARGUMENT(src == 0 || step == 0 || dst == 0);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&dst));
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&src));

    JsonArrayCompact *json_array_dst = to_json_array_compact_mut(&dst);
    if (json_array_dst == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    return json_array_dst->set_slice(start, end, step, *to_json_handle_compact(&src));
}

extern_c HakkaJsonResultEnum RemoveHakkaArrayIndex(HakkaHandle array, uint32_t index)
{
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));

    JsonArrayCompact *json_array = to_json_array_compact_mut(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    return json_array->remove_impl(index);
}

extern_c HakkaJsonResultEnum ClearHakkaArray(HakkaHandle array)
{
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));

    JsonArrayCompact *json_array = to_json_array_compact_mut(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    return json_array->clear_impl();
}

extern_c HakkaJsonResultEnum InsertHakkaArray(HakkaHandle array, uint32_t index, HakkaHandle value)
{
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&value));

    JsonArrayCompact *json_array = to_json_array_compact_mut(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    return json_array->insert_impl(index, *to_json_handle_compact(&value));
}

extern_c HakkaJsonResultEnum MultiplyHakkaArray(HakkaHandle array, int64_t times)
{
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));

    JsonArrayCompact *json_array = to_json_array_compact_mut(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    return json_array->multiply(times);
}

extern_c HakkaJsonResultEnum GetHakkaArraySize(HakkaHandle array, uint32_t *size)
{
    CHECK_INVALID_ARGUMENT(size == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));

    const JsonArrayCompact *json_array = to_json_array_compact(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    *size = static_cast<uint32_t>(json_array->length());
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum CountHakkaArray(HakkaHandle array, HakkaHandle value, uint32_t *count)
{
    CHECK_INVALID_ARGUMENT(count == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&value));

    const JsonArrayCompact *json_array = to_json_array_compact(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    return json_array->count(*to_json_handle_compact(&value), count);
}

extern_c HakkaJsonResultEnum ExtendHakkaArrayArray(HakkaHandle array, HakkaHandle value)
{
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&value));

    JsonArrayCompact *json_array = to_json_array_compact_mut(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    return json_array->extend(*to_json_handle_compact(&value));
}

extern_c HakkaJsonResultEnum FindFirstHakkaArray(HakkaHandle array, HakkaHandle other, uint32_t start, uint32_t stop, uint32_t *index)
{
    CHECK_INVALID_ARGUMENT(index == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&other));

    const JsonArrayCompact *json_array = to_json_array_compact(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    return json_array->index(*to_json_handle_compact(&other), start, stop, index);
}

extern_c HakkaJsonResultEnum PushBackHakkaArray(HakkaHandle array, HakkaHandle value)
{
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&value));

    JsonArrayCompact *json_array = to_json_array_compact_mut(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    return json_array->push_back(*to_json_handle_compact(&value));
}

extern_c HakkaJsonResultEnum PopHakkaArray(HakkaHandle array, uint32_t index, HakkaHandle *value)
{
    CHECK_INVALID_ARGUMENT(value == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));

    JsonArrayCompact *json_array = to_json_array_compact_mut(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    JsonHandleCompact pop_outed;
    auto result = json_array->pop(index, &pop_outed);
    if (result != HAKKA_JSON_SUCCESS)
        return result;

    *value = move_to_hakka_handle(std::move(pop_outed));
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum RemoveValueHakkaArray(HakkaHandle array, HakkaHandle value)
{
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&value));

    JsonArrayCompact *json_array = to_json_array_compact_mut(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    return json_array->remove_value(*to_json_handle_compact(&value));
}

extern_c HakkaJsonResultEnum ReverseHakkaArray(HakkaHandle array)
{
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));

    JsonArrayCompact *json_array = to_json_array_compact_mut(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    return json_array->reverse();
}

class HakkaArrayMoveIterCompact
{
public:
    // Constructors for forward and reverse iterators
    HakkaArrayMoveIterCompact(JsonArrayIterCompact<IterDirection::FORWARD> iter)
        : iter_(std::move(iter)), direction_(IterDirection::FORWARD) {}

    HakkaArrayMoveIterCompact(JsonArrayIterCompact<IterDirection::REVERSE> iter)
        : iter_(std::move(iter)), direction_(IterDirection::REVERSE) {}

    // Increment operators
    HakkaArrayMoveIterCompact &operator++()
    {
        std::visit([](auto &iter)
                   { ++iter; }, iter_);
        return *this;
    }

    // Decrement operators
    HakkaArrayMoveIterCompact &operator--()
    {
        std::visit([](auto &iter)
                   { --iter; }, iter_);
        return *this;
    }

    // Dereference operators
    JsonHandleCompact *operator->()
    {
        return std::visit([](auto &iter) -> JsonHandleCompact *
                          { return iter.operator->(); }, iter_);
    }

    JsonHandleCompact &operator*()
    {
        return std::visit([](auto &iter) -> JsonHandleCompact &
                          { return iter.operator*(); }, iter_);
    }

    // Comparison operators
    bool operator==(const HakkaArrayMoveIterCompact &other) const
    {
        if (direction_ != other.direction_)
            return false;

        return std::visit([&other](auto &iter) -> bool
                          {
        using IterType = std::decay_t<decltype(iter)>;
        if constexpr (std::is_same_v<IterType, JsonArrayIterCompact<IterDirection::FORWARD>>)
        {
            auto* other_iter = std::get_if<JsonArrayIterCompact<IterDirection::FORWARD>>(&other.iter_);
            if (other_iter)
                return iter == *other_iter;
            return false;
        }
        else // REVERSE
        {
            auto* other_iter = std::get_if<JsonArrayIterCompact<IterDirection::REVERSE>>(&other.iter_);
            if (other_iter)
                return iter == *other_iter;
            return false;
        } }, iter_);
    }

    bool operator!=(const HakkaArrayMoveIterCompact &other) const
    {
        return !(*this == other);
    }

    // Random access operators
    HakkaArrayMoveIterCompact &operator+=(int64_t n)
    {
        std::visit([n](auto &iter)
                   { iter += n; }, iter_);
        return *this;
    }

    HakkaArrayMoveIterCompact &operator-=(int64_t n)
    {
        std::visit([n](auto &iter)
                   { iter -= n; }, iter_);
        return *this;
    }

    // Check if iterator is at the end
    bool is_end() const
    {
        return std::visit([](const auto &iter) -> bool
                          { return iter.is_end(); }, iter_);
    }

private:
    std::variant<JsonArrayIterCompact<IterDirection::FORWARD>, JsonArrayIterCompact<IterDirection::REVERSE>> iter_;
    IterDirection direction_;
};

extern_c HakkaJsonResultEnum CreateHakkaArrayIterBegin(HakkaHandle array, HakkaArrayIter *iter)
{
    CHECK_INVALID_ARGUMENT(iter == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));

    const JsonArrayCompact *json_array = to_json_array_compact(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    JsonArrayIterCompact<IterDirection::FORWARD> forward_iter(json_array, 0);
    HakkaArrayMoveIterCompact *move_iter = new (std::nothrow) HakkaArrayMoveIterCompact(std::move(forward_iter));
    if (move_iter == nullptr)
        return HAKKA_JSON_NOT_ENOUGH_MEMORY;

    *iter = reinterpret_cast<HakkaArrayIter>(move_iter);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum CreateHakkaArrayIterRBegin(HakkaHandle array, HakkaArrayIter *iter)
{
    CHECK_INVALID_ARGUMENT(iter == nullptr);
    HANDLE_CHECK_INVALID_ARGUMENT(to_json_handle_compact(&array));

    const JsonArrayCompact *json_array = to_json_array_compact(&array);
    if (json_array == nullptr)
        return HAKKA_JSON_TYPE_ERROR;

    std::ptrdiff_t last_pos = static_cast<std::ptrdiff_t>(json_array->length()) - 1;
    if (last_pos < 0)
        last_pos = 0; // Handle empty array gracefully

    JsonArrayIterCompact<IterDirection::REVERSE> reverse_iter(json_array, last_pos);
    HakkaArrayMoveIterCompact *move_iter = new (std::nothrow) HakkaArrayMoveIterCompact(std::move(reverse_iter));
    if (move_iter == nullptr)
        return HAKKA_JSON_NOT_ENOUGH_MEMORY;

    *iter = reinterpret_cast<HakkaArrayIter>(move_iter);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum MoveHakkaArrayIterNext(HakkaArrayIter iter)
{
    CHECK_INVALID_ARGUMENT(iter == 0);

    HakkaArrayMoveIterCompact *move_iter = reinterpret_cast<HakkaArrayMoveIterCompact *>(iter);
    ++(*move_iter);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum MoveHakkaArrayIterPrev(HakkaArrayIter iter)
{
    CHECK_INVALID_ARGUMENT(iter == 0);

    HakkaArrayMoveIterCompact *move_iter = reinterpret_cast<HakkaArrayMoveIterCompact *>(iter);
    --(*move_iter);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum GetHakkaArrayIterDeref(HakkaArrayIter iter, HakkaHandle *value)
{
    CHECK_INVALID_ARGUMENT(iter == 0 || value == nullptr);
    auto inc_ref = [](const auto *elem) { return elem->inc_ref_impl(); };
    
    HakkaArrayMoveIterCompact *move_iter = reinterpret_cast<HakkaArrayMoveIterCompact *>(iter);
    if (move_iter->is_end())
        return HAKKA_JSON_ITERATOR_END;

    JsonHandleCompact &handle = move_iter->operator*();
    dispatch<decltype(inc_ref), uint64_t>(handle.get_view(), std::forward<decltype(inc_ref)>(inc_ref));
    *value = static_cast<HakkaHandle>(handle);
    return HAKKA_JSON_SUCCESS;
}

extern_c HakkaJsonResultEnum HakkaArrayIterRelease(HakkaArrayIter *iter)
{
    CHECK_INVALID_ARGUMENT(iter == nullptr || *iter == 0);

    delete reinterpret_cast<HakkaArrayMoveIterCompact *>(*iter);
    *iter = 0;
    return HAKKA_JSON_SUCCESS;
}
