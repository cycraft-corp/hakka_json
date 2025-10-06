#ifndef __HAKKA_JSON_HANDLE_STRING_MANAGER_HPP__
#define __HAKKA_JSON_HANDLE_STRING_MANAGER_HPP__
#pragma once

#include <handles/manager.hpp>

#include <string_view>

namespace hakka {

// Placeholder for StringManagerCompact - to be implemented later
class StringManagerCompact : public JsonHandleManagerCompact
{
    StringManagerCompact() = default;
    uint32_t get_index(HandleManagerToken token) const;

public:
    static constexpr auto string_mask = 0x40000000; // 0100 0000 0000 0000 0000 0000 0000 0000
    static StringManagerCompact &get_instance();
    ~StringManagerCompact() = default;

    HakkaJsonType type(HandleManagerToken token) const override;
    UniformCompactPointerView get_view(HandleManagerToken token) const override;
    void release(HandleManagerToken token) override;

    HandleManagerToken create(std::string_view value);
};

} // namespace hakka

#endif // __HAKKA_JSON_HANDLE_STRING_MANAGER_HPP__