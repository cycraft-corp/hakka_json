#include <uniform_compact_pointer.hpp>
#include <hakka_json_int.hpp>
#include <hakka_compare.hpp>

#include <handles/scalar_manager.hpp>

#include <cstring>
#include <string>

using namespace hakka;

JsonIntCompact::JsonIntCompact(ValueType value) : JsonPrimitiveCompact(value)
{
}

JsonHandleCompact JsonIntCompact::create(ValueType value) {
    return JsonHandleCompact(ScalarManagerCompact::get_instance().create(std::move(value)));
}

std::unique_ptr<JsonIntCompact> JsonIntCompact::create_unique(ValueType value) {
    return std::unique_ptr<JsonIntCompact>(new (std::nothrow) JsonIntCompact(value));
}

uint64_t JsonIntCompact::inc_ref_impl() const {
    return ref_count.fetch_add(1, std::memory_order_relaxed) + 1;
}
    
uint64_t JsonIntCompact::dec_ref_impl() const {
    return ref_count.fetch_sub(1, std::memory_order_relaxed) - 1;
}

tl::expected<std::string, HakkaJsonResultEnum> JsonIntCompact::dump_impl([[maybe_unused]] uint32_t /*max_depth*/) const {
    try {
        return std::to_string(value_);
    } catch (...) {
        return tl::make_unexpected(HAKKA_JSON_INTERNAL_ERROR);
    }
}

HakkaJsonResultEnum JsonIntCompact::to_bytes_impl(char *buffer, uint32_t *buffer_size) const {
    try {
        std::string int_str = dump(1).value();
        uint32_t required_size = static_cast<uint32_t>(int_str.size()) + 1; // +1 for null terminator

        if (*buffer_size < required_size) {
            *buffer_size = required_size;
            return HAKKA_JSON_NOT_ENOUGH_MEMORY;
        }

        std::memcpy(buffer, int_str.c_str(), required_size);
        buffer[int_str.size()] = '\0';
        *buffer_size = static_cast<uint32_t>(int_str.size());
        return HAKKA_JSON_SUCCESS;
    } catch (...) {
        return HAKKA_JSON_INTERNAL_ERROR;
    }
}

HakkaJsonType JsonIntCompact::type_impl() const {
    return HakkaJsonType::HAKKA_JSON_INT;
}

tl::expected<int, HakkaJsonResultEnum> JsonIntCompact::compare_impl(const JsonHandleCompact &other) const {
    if (other.get_type() != HakkaJsonType::HAKKA_JSON_INT &&
        other.get_type() != HakkaJsonType::HAKKA_JSON_FLOAT &&
        other.get_type() != HakkaJsonType::HAKKA_JSON_BOOL)
    {
        return tl::make_unexpected(HAKKA_JSON_TYPE_ERROR);
    }

    return hakka::compare(UniformCompactPointerView(const_cast<JsonIntCompact*>(this)), other.get_view(), 0);
}

uint64_t JsonIntCompact::hash_impl() const {
    return std::hash<ValueType>{}(value_);
}

uint64_t JsonIntCompact::dump_size_impl() const {
 
    return dump_impl(0).value().size();
}

tl::expected<PrimitiveType, HakkaJsonResultEnum> JsonIntCompact::get_impl() const {
    return value_;
}
