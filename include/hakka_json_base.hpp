#ifndef __HAKKA_JSON_BASE_HPP__
#define __HAKKA_JSON_BASE_HPP__
#pragma once

#include <string>
#include <cstdint>

#include <tl/expected.hpp>

#include <hakka_json_enum.h>
#include <hakka_json_handle.hpp>

namespace hakka {

template <typename Derived>
class JsonBaseCompact {
public:
    JsonBaseCompact() = default;
    ~JsonBaseCompact() = default;

    uint64_t inc_ref() const { return static_cast<const Derived *>(this)->inc_ref_impl(); }
    uint64_t dec_ref() const { return static_cast<const Derived *>(this)->dec_ref_impl(); }

    tl::expected<std::string, HakkaJsonResultEnum> dump(uint32_t max_depth) const { return static_cast<const Derived *>(this)->dump_impl(max_depth); }
    HakkaJsonResultEnum to_bytes(char *buffer, uint32_t *buffer_size) const { return static_cast<const Derived *>(this)->to_bytes_impl(buffer, buffer_size); }
    bool is_valid() const { return static_cast<const Derived *>(this)->is_valid_impl(); }
    HakkaJsonType type() const { return static_cast<const Derived *>(this)->type_impl(); }
    tl::expected<int, HakkaJsonResultEnum> compare(const JsonHandleCompact &other) const { return static_cast<const Derived *>(this)->compare_impl(other); }
    uint64_t hash() const { return static_cast<const Derived *>(this)->hash_impl(); }
    uint64_t dump_size() const { return static_cast<const Derived *>(this)->dump_size_impl(); }

    // capi needs aligned method for vistitor calling
    tl::expected<uint64_t, HakkaJsonResultEnum> inc_ref_capi() const { return static_cast<const Derived *>(this)->inc_ref_impl(); }
    tl::expected<uint64_t, HakkaJsonResultEnum> dec_ref_capi() const { return static_cast<const Derived *>(this)->dec_ref_impl(); }
    tl::expected<std::string, HakkaJsonResultEnum> dump_capi(uint32_t max_depth) const { return static_cast<const Derived *>(this)->dump_impl(max_depth); }
    tl::expected<void, HakkaJsonResultEnum> to_bytes_capi(char *buffer, uint32_t *buffer_size) const { 
        auto result = static_cast<const Derived *>(this)->to_bytes_impl(buffer, buffer_size); 
        if (result == HAKKA_JSON_SUCCESS) return {};
        return tl::make_unexpected(result);
    }
    tl::expected<bool, HakkaJsonResultEnum> is_valid_capi() const { return static_cast<const Derived *>(this)->is_valid_impl(); }
    tl::expected<HakkaJsonType, HakkaJsonResultEnum> type_capi() const { return static_cast<const Derived *>(this)->type_impl(); }
    tl::expected<int, HakkaJsonResultEnum> compare_capi(const JsonHandleCompact &other) const { return static_cast<const Derived *>(this)->compare_impl(other); }
    tl::expected<uint64_t, HakkaJsonResultEnum> hash_capi() const { return static_cast<const Derived *>(this)->hash_impl(); }
    tl::expected<uint64_t, HakkaJsonResultEnum> dump_size_capi() const { return static_cast<const Derived *>(this)->dump_size_impl(); }
};

} // namespace hakka

#endif // __HAKKA_JSON_BASE_HPP__