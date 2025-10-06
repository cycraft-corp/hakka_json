#include <json_deserializer.hpp>
#include <hakka_json_array.hpp>
#include <hakka_json_object.hpp>
#include <hakka_json_string.hpp>
#include <hakka_json_int.hpp>
#include <hakka_json_float.hpp>

#include <nlohmann/json.hpp>

using namespace hakka;

namespace
{
    namespace detail
    {
        JsonHandleCompact to_hakka_json_impl(const nlohmann::json &j)  {
            const auto type = j.type();
            switch (type)
            {
            case nlohmann::json::value_t::null:
                return JsonFloatCompact::create(nullptr);
            case nlohmann::json::value_t::boolean:
                return JsonFloatCompact::create(j.get<bool>());
            case nlohmann::json::value_t::number_unsigned:
                return JsonIntCompact::create(j.get<int64_t>());
            case nlohmann::json::value_t::number_integer:
                return JsonIntCompact::create(j.get<int64_t>());
            case nlohmann::json::value_t::number_float:
                return JsonFloatCompact::create(j.get<double>());
            case nlohmann::json::value_t::string:
                return JsonStringCompact::create(j.get<std::string>());
            case nlohmann::json::value_t::array:
            {
                auto arr_handle = JsonArrayCompact::create();
                auto arr_ptr = std::get<JsonArrayCompact*>(arr_handle.get_mut_ptr());
                if (!arr_ptr)
                    return JsonHandleCompact();

                for (const auto &elem : j)
                    arr_ptr->push_back(to_hakka_json_impl(elem));

                arr_ptr->shrink_to_fit();
                return arr_handle;
            }
            case nlohmann::json::value_t::object:
            {
                auto obj_handle = JsonObjectCompact::create();
                auto obj_ptr = std::get<JsonObjectCompact*>(obj_handle.get_mut_ptr());
                if (!obj_ptr)
                    return JsonHandleCompact();

                for (const auto &[key, value] : j.items())
                    obj_ptr->set(key, to_hakka_json_impl(value));

                obj_ptr->shrink_to_fit();
                return obj_handle;
            }

            default:
                return JsonHandleCompact();
            };
        }
    } // namespace detail
}

struct JsonDeserializerCompact::Impl
{
    nlohmann::json json_data;
};

JsonDeserializerCompact::JsonDeserializerCompact() : impl_(std::make_unique<Impl>()) {}

JsonDeserializerCompact::~JsonDeserializerCompact() = default;

HakkaJsonResultEnum JsonDeserializerCompact::loads(std::string_view json_str, uint32_t max_depth) {
    nlohmann::json::parser_callback_t cb = [max_depth](int depth,
                                                       nlohmann::json::parse_event_t event,
                                                       nlohmann::json &) -> bool
    {
        if (event == nlohmann::json::parse_event_t::object_start || event == nlohmann::json::parse_event_t::array_start)
        {
            if (uint32_t(depth) >= max_depth) [[unlikely]]
                throw std::runtime_error("Recursion depth exceeded");
        }
        return true;
    };
    try {
        impl_->json_data = nlohmann::json::parse(json_str, cb, true, true);
    }
    catch (const std::runtime_error &) {
        return HAKKA_JSON_RECURSION_DEPTH_EXCEEDED;
    }
    catch (const nlohmann::json::parse_error &) {
        return HAKKA_JSON_PARSE_ERROR;
    }
    catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
    return HAKKA_JSON_SUCCESS;
}

JsonHandleCompact JsonDeserializerCompact::to_hakka_json() const {
    // we need this wrapper to eliminate the JsonDeserializer significant overhead
    return detail::to_hakka_json_impl(impl_->json_data); 
}

HakkaJsonType JsonDeserializerCompact::type() const {
    const auto &my_type = impl_->json_data.type();

    switch (my_type) {
    case nlohmann::json::value_t::null:
        return HAKKA_JSON_NULL;
    case nlohmann::json::value_t::boolean:
        return HAKKA_JSON_BOOL;
    case nlohmann::json::value_t::number_unsigned:
        return HAKKA_JSON_INT;
    case nlohmann::json::value_t::number_integer:
        return HAKKA_JSON_INT;
    case nlohmann::json::value_t::number_float:
        return HAKKA_JSON_FLOAT;
    case nlohmann::json::value_t::string:
        return HAKKA_JSON_STRING;
    case nlohmann::json::value_t::array:
        return HAKKA_JSON_ARRAY;
    case nlohmann::json::value_t::object:
        return HAKKA_JSON_OBJECT;
    default:
        return HAKKA_JSON_INVALID;
    }
    return HAKKA_JSON_INVALID;
}
