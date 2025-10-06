#include <hakka_json_handle.hpp>
#include <hakka_json_base.hpp>

#include <hakka_json_int.hpp>
#include <hakka_json_float.hpp>
#include <hakka_json_string.hpp>
#include <hakka_json_array.hpp>
#include <hakka_json_object.hpp>

#include <variant>

using namespace hakka;

OwnedUniformCompactPointer::~OwnedUniformCompactPointer() {
    if (data == nullptr)
        return;

    auto type = get_type();
    auto ptr = reinterpret_cast<PointerType>((reinterpret_cast<std::size_t>(data) & ~TYPE_MASK));

    if (type == HAKKA_JSON_INT) {
        // cast it back to JsonIntCompact* and for unique_ptr release
        std::unique_ptr<JsonIntCompact> int_ptr(reinterpret_cast<JsonIntCompact*>(ptr));
    } else if (type == HAKKA_JSON_FLOAT) {
        std::unique_ptr<JsonFloatCompact> float_ptr(reinterpret_cast<JsonFloatCompact*>(ptr));
    } else if (type == HAKKA_JSON_BOOL) {
        std::unique_ptr<JsonFloatCompact> bool_ptr(reinterpret_cast<JsonFloatCompact*>(ptr)); // YES, it is nan boxing
    } else if (type == HAKKA_JSON_STRING) {
        std::unique_ptr<JsonStringCompact> string_ptr(reinterpret_cast<JsonStringCompact*>(ptr));
    } else if (type == HAKKA_JSON_ARRAY) {
        std::unique_ptr<JsonArrayCompact> array_ptr(reinterpret_cast<JsonArrayCompact*>(ptr));
    } else if (type == HAKKA_JSON_OBJECT) {
        std::unique_ptr<JsonObjectCompact> object_ptr(reinterpret_cast<JsonObjectCompact*>(ptr));
    } else if (type == HAKKA_JSON_NULL) {
        std::unique_ptr<JsonFloatCompact> null_ptr(reinterpret_cast<JsonFloatCompact*>(ptr)); // YES, it is nan boxing
    } else if (type == HAKKA_JSON_INVALID) {
        std::unique_ptr<JsonFloatCompact> invalid_ptr(reinterpret_cast<JsonFloatCompact*>(ptr)); // YES, it is nan boxing
    }

    data = nullptr; // prevent reentrancy
}

UniformCompactPointerView JsonHandleCompact::get_view() const
{
    auto type = get_type();
    const auto &manager = get_manager();
    if (type == HAKKA_JSON_INT)
        return UniformCompactPointerView(std::get<const JsonIntCompact*>(manager.get_view(data)));
    else if (type == HAKKA_JSON_FLOAT)
        return UniformCompactPointerView(std::get<const JsonFloatCompact*>(manager.get_view(data)));
    else if (type == HAKKA_JSON_BOOL)
        return UniformCompactPointerView(reinterpret_cast<const JsonBoolCompact*>(std::get<const JsonFloatCompact*>(manager.get_view(data))));
    else if (type == HAKKA_JSON_STRING)
        return UniformCompactPointerView(std::get<const JsonStringCompact*>(manager.get_view(data)));
    else if (type == HAKKA_JSON_ARRAY)
        return UniformCompactPointerView(std::get<const JsonArrayCompact*>(manager.get_view(data)));
    else if (type == HAKKA_JSON_OBJECT)
        return UniformCompactPointerView(std::get<const JsonObjectCompact*>(manager.get_view(data)));
    else if (type == HAKKA_JSON_NULL)
        return UniformCompactPointerView(reinterpret_cast<const JsonNullCompact*>(std::get<const JsonFloatCompact*>(manager.get_view(data))));
    else if (type == HAKKA_JSON_INVALID)
        return UniformCompactPointerView(reinterpret_cast<const JsonInvalidCompact*>(std::get<const JsonFloatCompact*>(manager.get_view(data))));
    else
        return UniformCompactPointerView(std::monostate{});
}

UniformCompactPointer JsonHandleCompact::get_mut_ptr()
{
    auto type = get_type();
    const auto &manager = get_manager();
    if (type == HAKKA_JSON_ARRAY)
        return UniformCompactPointer(std::get<JsonArrayCompact*>(manager.get_mut_ptr(data)));
    else if (type == HAKKA_JSON_OBJECT)
        return UniformCompactPointer(std::get<JsonObjectCompact*>(manager.get_mut_ptr(data)));
    else
        return UniformCompactPointer(std::monostate{});
}


uint32_t JsonHandleCompact::retain() const {
    auto compacted_pointer = get_view();
    if (std::holds_alternative<std::monostate>(compacted_pointer))
        return 0;
    else if (std::holds_alternative<const JsonIntCompact*>(compacted_pointer))
        return std::get<const JsonIntCompact*>(compacted_pointer)->inc_ref();
    else if (std::holds_alternative<const JsonFloatCompact*>(compacted_pointer))
        return std::get<const JsonFloatCompact*>(compacted_pointer)->inc_ref();
    else if (std::holds_alternative<const JsonBoolCompact*>(compacted_pointer))
        return reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonBoolCompact*>(compacted_pointer))->inc_ref(); // YES, it is nan boxing
    else if (std::holds_alternative<const JsonStringCompact*>(compacted_pointer))
        return std::get<const JsonStringCompact*>(compacted_pointer)->inc_ref();
    else if (std::holds_alternative<const JsonArrayCompact*>(compacted_pointer))
        return std::get<const JsonArrayCompact*>(compacted_pointer)->inc_ref();
    else if (std::holds_alternative<const JsonObjectCompact*>(compacted_pointer))
        return std::get<const JsonObjectCompact*>(compacted_pointer)->inc_ref();
    else if (std::holds_alternative<const JsonNullCompact*>(compacted_pointer))
        return reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonNullCompact*>(compacted_pointer))->inc_ref(); // YES, it is nan boxing
    else if (std::holds_alternative<const JsonInvalidCompact*>(compacted_pointer))
        return reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonInvalidCompact*>(compacted_pointer))->inc_ref(); // YES, it is nan boxing
    else
        return 0;
}
