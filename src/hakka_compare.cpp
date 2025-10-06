#include <hakka_compare.hpp>
#include <hakka_json_int.hpp>
#include <hakka_json_float.hpp>
#include <hakka_json_enum.h>
#include <hakka_json_primitive.hpp>

using namespace hakka;

namespace version_2 {

    static int ii(const UniformCompactPointerView &a, const UniformCompactPointerView &b, uint32_t)
    {
        auto a_int = std::get<const JsonIntCompact*>(a);
        auto b_int = std::get<const JsonIntCompact*>(b);
        return static_cast<int>(std::get<JsonIntCompact::ValueType>(a_int->get().value()) - std::get<JsonIntCompact::ValueType>(b_int->get().value()));
    }

    static int ib(const UniformCompactPointerView &a, const UniformCompactPointerView &b, uint32_t)
    {
        auto a_int = std::get<const JsonIntCompact*>(a);
        auto b_bool = reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonBoolCompact*>(b));
        return std::get<JsonIntCompact::ValueType>(a_int->get().value()) - std::get<bool>(b_bool->get().value());
    }

    static int id(const UniformCompactPointerView &a, const UniformCompactPointerView &b, uint32_t)
    {
        auto a_int = std::get<const JsonIntCompact*>(a);
        auto b_float = std::get<const JsonFloatCompact*>(b);
        return std::get<JsonIntCompact::ValueType>(a_int->get().value()) - std::get<JsonFloatCompact::ValueType>(b_float->get().value());
    }

    static int in(const UniformCompactPointerView &a, const UniformCompactPointerView &, uint32_t)
    {
        auto a_int = std::get<const JsonIntCompact*>(a);
        return std::get<JsonIntCompact::ValueType>(a_int->get().value());
    }

    static int bi(const UniformCompactPointerView &a, const UniformCompactPointerView &b, uint32_t)
    {
        auto a_bool = reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonBoolCompact*>(a));
        auto b_int = std::get<const JsonIntCompact*>(b);
        return std::get<bool>(a_bool->get().value()) - std::get<JsonIntCompact::ValueType>(b_int->get().value());
    }

    static int bb(const UniformCompactPointerView &a, const UniformCompactPointerView &b, uint32_t)
    {
        auto a_bool = reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonBoolCompact*>(a));
        auto b_bool = reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonBoolCompact*>(b));
        return std::get<bool>(a_bool->get().value()) - std::get<bool>(b_bool->get().value());
    }

    static int bd(const UniformCompactPointerView &a, const UniformCompactPointerView &b, uint32_t)
    {
        auto a_bool = reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonBoolCompact*>(a));
        auto b_float = std::get<const JsonFloatCompact*>(b);
        return std::get<bool>(a_bool->get().value()) - std::get<JsonFloatCompact::ValueType>(b_float->get().value());
    }

    static int bn(const UniformCompactPointerView &a, const UniformCompactPointerView &, uint32_t)
    {
        auto a_bool = reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonBoolCompact*>(a));
        return std::get<bool>(a_bool->get().value());
    }

    
    static int di(const UniformCompactPointerView &a, const UniformCompactPointerView &b, uint32_t)
    {
        auto a_float = std::get<const JsonFloatCompact*>(a);
        auto b_int = std::get<const JsonIntCompact*>(b);
        return std::get<JsonFloatCompact::ValueType>(a_float->get().value()) - std::get<JsonIntCompact::ValueType>(b_int->get().value());
    }

    static int db(const UniformCompactPointerView &a, const UniformCompactPointerView &b, uint32_t)
    {
        auto a_float = std::get<const JsonFloatCompact*>(a);
        auto b_bool = reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonBoolCompact*>(b));
        return std::get<JsonFloatCompact::ValueType>(a_float->get().value()) - std::get<bool>(b_bool->get().value());
    }

    static int dd(const UniformCompactPointerView &a, const UniformCompactPointerView &b, uint32_t)
    {
        auto a_float = std::get<const JsonFloatCompact*>(a);
        auto b_float = std::get<const JsonFloatCompact*>(b);
        return std::get<JsonFloatCompact::ValueType>(a_float->get().value()) - std::get<JsonFloatCompact::ValueType>(b_float->get().value());
    }

    static int dn(const UniformCompactPointerView &a, const UniformCompactPointerView &, uint32_t)
    {
        auto a_float = std::get<const JsonFloatCompact*>(a);
        return std::get<JsonFloatCompact::ValueType>(a_float->get().value());
    }

    static int ni(const UniformCompactPointerView &, const UniformCompactPointerView &b, uint32_t)
    {
        auto b_int = std::get<const JsonIntCompact*>(b);
        return std::get<JsonIntCompact::ValueType>(b_int->get().value()) * -1;
    }

    static int nb(const UniformCompactPointerView &, const UniformCompactPointerView &b, uint32_t)
    {
        auto b_bool = reinterpret_cast<const JsonFloatCompact*>(std::get<const JsonBoolCompact*>(b));
        return std::get<bool>(b_bool->get().value()) * -1;
    }

    static int nd(const UniformCompactPointerView &, const UniformCompactPointerView &b, uint32_t)
    {
        auto b_float = std::get<const JsonFloatCompact*>(b);
        return static_cast<int>(std::get<JsonFloatCompact::ValueType>(b_float->get().value()) * -1);
    }

    static int nn(const UniformCompactPointerView &, const UniformCompactPointerView &, uint32_t)
    {
        return 0;
    }

    template<typename T>
    consteval auto get_index_of_uniform_compact_pointer() {
        return UniformCompactPointerView{T{}}.index();
    }

    using CompareFunction = int (*)(const UniformCompactPointerView &, const UniformCompactPointerView &, uint32_t);
    struct EnumPairHash
    {
        std::size_t operator()(const std::pair<HakkaJsonType, HakkaJsonType> &p) const
        {
            return std::hash<int>()(static_cast<int>(p.first)) ^ std::hash<int>()(static_cast<int>(p.second));
        }
    };

    const static std::unordered_map<std::pair<HakkaJsonType, HakkaJsonType>, CompareFunction, EnumPairHash> compare_table = {
        {{HakkaJsonType::HAKKA_JSON_INT, HakkaJsonType::HAKKA_JSON_INT}, ii},
        {{HakkaJsonType::HAKKA_JSON_INT, HakkaJsonType::HAKKA_JSON_BOOL}, ib},
        {{HakkaJsonType::HAKKA_JSON_INT, HakkaJsonType::HAKKA_JSON_FLOAT}, id},
        {{HakkaJsonType::HAKKA_JSON_INT, HakkaJsonType::HAKKA_JSON_NULL}, in},

        {{HakkaJsonType::HAKKA_JSON_BOOL, HakkaJsonType::HAKKA_JSON_INT}, bi},
        {{HakkaJsonType::HAKKA_JSON_BOOL, HakkaJsonType::HAKKA_JSON_BOOL}, bb},
        {{HakkaJsonType::HAKKA_JSON_BOOL, HakkaJsonType::HAKKA_JSON_FLOAT}, bd},
        {{HakkaJsonType::HAKKA_JSON_BOOL, HakkaJsonType::HAKKA_JSON_NULL}, bn},

        {{HakkaJsonType::HAKKA_JSON_FLOAT, HakkaJsonType::HAKKA_JSON_INT}, di},
        {{HakkaJsonType::HAKKA_JSON_FLOAT, HakkaJsonType::HAKKA_JSON_BOOL}, db},
        {{HakkaJsonType::HAKKA_JSON_FLOAT, HakkaJsonType::HAKKA_JSON_FLOAT}, dd},
        {{HakkaJsonType::HAKKA_JSON_FLOAT, HakkaJsonType::HAKKA_JSON_NULL}, dn},

        {{HakkaJsonType::HAKKA_JSON_NULL, HakkaJsonType::HAKKA_JSON_INT}, ni},
        {{HakkaJsonType::HAKKA_JSON_NULL, HakkaJsonType::HAKKA_JSON_BOOL}, nb},
        {{HakkaJsonType::HAKKA_JSON_NULL, HakkaJsonType::HAKKA_JSON_FLOAT}, nd},
        {{HakkaJsonType::HAKKA_JSON_NULL, HakkaJsonType::HAKKA_JSON_NULL}, nn},
    };
} // namespace version_2

namespace hakka {

tl::expected<int, HakkaJsonResultEnum> compare(const UniformCompactPointerView &a, const UniformCompactPointerView &b, uint32_t max_depth)
{
    using IndexType = decltype(UniformCompactPointerView{std::monostate{}}.index());

    const static std::unordered_map<IndexType, HakkaJsonType> index_to_type = {
        {version_2::get_index_of_uniform_compact_pointer<JsonIntCompact*>(), HakkaJsonType::HAKKA_JSON_INT},
        {version_2::get_index_of_uniform_compact_pointer<JsonFloatCompact*>(), HakkaJsonType::HAKKA_JSON_FLOAT},
        {version_2::get_index_of_uniform_compact_pointer<JsonBoolCompact*>(), HakkaJsonType::HAKKA_JSON_BOOL},
        {version_2::get_index_of_uniform_compact_pointer<JsonNullCompact*>(), HakkaJsonType::HAKKA_JSON_NULL},
        {version_2::get_index_of_uniform_compact_pointer<JsonInvalidCompact*>(), HakkaJsonType::HAKKA_JSON_INVALID},
        {version_2::get_index_of_uniform_compact_pointer<JsonStringCompact*>(), HakkaJsonType::HAKKA_JSON_STRING},
        {version_2::get_index_of_uniform_compact_pointer<JsonArrayCompact*>(), HakkaJsonType::HAKKA_JSON_ARRAY},
        {version_2::get_index_of_uniform_compact_pointer<JsonObjectCompact*>(), HakkaJsonType::HAKKA_JSON_OBJECT},
    };

    auto it_a = index_to_type.find(a.index());
    if (it_a == index_to_type.end()) [[unlikely]]
        return tl::make_unexpected(HakkaJsonResultEnum::HAKKA_JSON_TYPE_ERROR);

    auto it_b = index_to_type.find(b.index());
    if (it_b == index_to_type.end()) [[unlikely]]
        return tl::make_unexpected(HakkaJsonResultEnum::HAKKA_JSON_TYPE_ERROR);

    auto it = version_2::compare_table.find({it_a->second, it_b->second});
    if (it == version_2::compare_table.end()) [[unlikely]]
        return tl::make_unexpected(HakkaJsonResultEnum::HAKKA_JSON_TYPE_ERROR);

    return it->second(a, b, max_depth);
}

} // namespace hakka

