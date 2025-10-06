#ifndef __HAJKA_JSON_STRUCTURED_HPP__
#define __HAJKA_JSON_STRUCTURED_HPP__
#pragma once

#include <hakka_json_base.hpp>
#include <hakka_json_structured_key.hpp>
#include <hakka_json_handle.hpp>

#include <tl/expected.hpp>
#include <atomic>

namespace hakka {

template <typename Derived>
class JsonStructuredCompact : public JsonBaseCompact<Derived>
{
public:
    ~JsonStructuredCompact() = default;

    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> get(KeyType key) const { return static_cast<const Derived *>(this)->get_impl(key); }
    HakkaJsonResultEnum set(KeyType key, JsonHandleCompact value) { return static_cast<Derived *>(this)->set_impl(key, value); }
    HakkaJsonResultEnum remove(KeyType key) { return static_cast<Derived *>(this)->remove_impl(key); }
    tl::expected<JsonHandleCompact, HakkaJsonResultEnum> at(uint32_t index) const { return static_cast<const Derived *>(this)->at_impl(index); }
    HakkaJsonResultEnum insert(KeyType index, JsonHandleCompact value) { return static_cast<Derived *>(this)->insert_impl(index, value); }
    HakkaJsonResultEnum erase(KeyType key) { return static_cast<Derived *>(this)->erase_impl(key); }
    HakkaJsonResultEnum clear() { return static_cast<Derived *>(this)->clear_impl(); }
    bool is_valid() const { return true; }
    void shrink_to_fit() { return static_cast<Derived *>(this)->shrink_to_fit_impl(); }

protected:
    mutable std::atomic<uint64_t> ref_count = 1;
    // We left the derived to create their member variables, due to the ObjectClass has "two" arrays.
};

} // namespace hakka

#endif // __HAJKA_JSON_STRUCTURED_HPP__