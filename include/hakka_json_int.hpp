#ifndef __HAKKA_JSON_INT_HPP__
#define __HAKKA_JSON_INT_HPP__
#include <cstdint>
#pragma once

#include <hakka_json_primitive.hpp>

namespace hakka {

class JsonIntCompact final : public JsonPrimitiveCompact<JsonIntCompact, int64_t> {
public:
    using ValueType = int64_t;

    ~JsonIntCompact() { dec_ref(); }
    [[nodiscard]] static JsonHandleCompact create(ValueType value);
    [[nodiscard]] static std::unique_ptr<JsonIntCompact> create_unique(ValueType value);

    // From JsonBaseCompact
    uint64_t inc_ref_impl() const;
    uint64_t dec_ref_impl() const;
    tl::expected<std::string, HakkaJsonResultEnum> dump_impl([[maybe_unused]] uint32_t max_depth = 0) const;
    HakkaJsonResultEnum to_bytes_impl(char *buffer, uint32_t *buffer_size) const;
    HakkaJsonType type_impl() const;
    tl::expected<int, HakkaJsonResultEnum> compare_impl(const JsonHandleCompact &other) const;
    uint64_t hash_impl() const;
    uint64_t dump_size_impl() const;

    // From JsonPrimitiveCompact
    tl::expected<PrimitiveType, HakkaJsonResultEnum> get_impl() const;
    using JsonPrimitiveCompact::is_valid;             // inherit is_valid

private:
    explicit JsonIntCompact(ValueType value);
};

} // namespace hakka

#endif