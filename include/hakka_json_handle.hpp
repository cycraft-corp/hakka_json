#ifndef __HAKKA_JSON_HANDLE_HPP__
#define __HAKKA_JSON_HANDLE_HPP__
#pragma once

#include <hakka_json_enum.h>

#include <handles/manager.hpp>
#include <handles/manager_registry.hpp>

#include <uniform_compact_pointer.hpp>

namespace hakka {

class JsonHandleCompact
{
    HandleManagerToken data;
    // top 2 bits are types: Scalar(00), String(01), Array(10), Object(11)
    constexpr static auto type_mask = 0xC0000000;

    JsonHandleManagerCompact &get_manager() const
    {
        auto type = static_cast<JsonHandleManagerType>((data & type_mask) >> 30);
        return *JsonHandleManagerRegistryCompact::get_instance().get_manager(type);
    }

public:
    JsonHandleCompact() : data(0)
    {
        // A zero value for the data handle is treated as an INVALID FloatType
        // Please refer to the INVALID_TOKEN in scalar_manager.cpp
    }

    explicit JsonHandleCompact(HandleManagerToken token) : data(token) {}

    JsonHandleCompact(const JsonHandleCompact &other) : data(other.data)
    {
        retain();
    }

    JsonHandleCompact(JsonHandleCompact &&other) noexcept : data(other.data)
    {
        other.data = 0;
    }

    JsonHandleCompact &operator=(const JsonHandleCompact &other)
    {
        if (this != &other)
        {
            release();
            data = other.data;
            retain();
        }
        return *this;
    }

    JsonHandleCompact &operator=(JsonHandleCompact &&other) noexcept
    {
        if (this != &other)
        {
            release();
            data = other.data;
            other.data = 0;
        }
        return *this;
    }

    ~JsonHandleCompact()
    {
        release();
    }

    // Scalar types are: Int, and Floats.
    // We collapse the Null, True, False, Invalid to the Floats NaN (NaN Boxing).
    HakkaJsonType get_type() const
    {
        return get_manager().type(data);
    }

    UniformCompactPointerView get_view() const;

    // get_mut_ptr is for mutable access to the object, which is only available for Array and Object.
    // Please do not use this for other types.
    UniformCompactPointer get_mut_ptr();

    uint32_t retain() const;

    void release()
    {
        if (data)
            get_manager().release(data);
        data = 0;
    }

    bool operator!() const { return data == 0; }
    operator bool() const { return data != 0; }
    bool is_valid() const { return data != 0; }
   
    operator uint64_t() const
    { // For HakkaHandle C API interface
        return static_cast<uint64_t>(data);
    }
};

} // namespace hakka

#endif // __HAKKA_JSON_HANDLE_HPP__
