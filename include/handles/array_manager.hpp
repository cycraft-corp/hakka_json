#ifndef __HAKKA_JSON_HANDLE_ARRAY_MANAGER_HPP__
#define __HAKKA_JSON_HANDLE_ARRAY_MANAGER_HPP__
#pragma once

#include <handles/manager.hpp>

namespace hakka {

class ArrayManagerCompact : public JsonHandleManagerCompact
{
    ArrayManagerCompact() = default;
    uint32_t get_index(HandleManagerToken token) const;

public:
    static constexpr auto array_mask = 0x80000000;
    static ArrayManagerCompact &get_instance();
    ~ArrayManagerCompact() = default;

    HakkaJsonType type(HandleManagerToken token) const override;
    UniformCompactPointerView get_view(HandleManagerToken token) const override;
    UniformCompactPointer get_mut_ptr(HandleManagerToken token) const override;
    void release(HandleManagerToken token) override;

    HandleManagerToken create();
    // @note: The hash_to_index_map_ is not used in this manager.
    // The core purpose of the hash is aiming for reuse the same object.
    // However, the object is shared by multiple handles, so it's not necessary to reuse the object.
};

} // namespace hakka

#endif // __HAKKA_JSON_HANDLE_ARRAY_MANAGER_HPP__
