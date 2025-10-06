#ifndef __HAKKA_JSON_DESERIALIZER_HPP__
#define __HAKKA_JSON_DESERIALIZER_HPP__
#pragma once

#include <hakka_json_enum.h>
#include <hakka_json_handle.hpp>

#include <string_view>
#include <memory>

namespace hakka {

class JsonDeserializerCompact
{
    struct Impl;
    std::unique_ptr<Impl> impl_;

public:
    HakkaJsonResultEnum loads(std::string_view json_str, uint32_t max_depth);
    JsonHandleCompact to_hakka_json() const;
    HakkaJsonType type() const;

    JsonDeserializerCompact();
    ~JsonDeserializerCompact();
};

} // namespace hakka

#endif // __HAKKA_JSON_DESERIALIZER_HPP__