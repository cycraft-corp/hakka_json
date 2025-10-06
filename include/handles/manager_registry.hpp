#ifndef __HAKKA_JSON_HANDLE_MANAGER_REGISTERY_HPP__
#define __HAKKA_JSON_HANDLE_MANAGER_REGISTERY_HPP__
#pragma once

#include <handles/manager.hpp>

#include <cstdint>
#include <cassert>

namespace hakka {

class JsonHandleManagerRegistryCompact
{
    JsonHandleManagerCompact *managers_[4];
    JsonHandleManagerRegistryCompact() = default;

public:
    static JsonHandleManagerRegistryCompact &get_instance()
    {
        static JsonHandleManagerRegistryCompact instance;
        return instance;
    }
    
    void register_manager(JsonHandleManagerType type, JsonHandleManagerCompact *manager)
    {
        assert(type >= JsonHandleManagerType::Scalar && type <= JsonHandleManagerType::Object);
        assert(manager != nullptr);
        assert(managers_[static_cast<uint8_t>(type)] == nullptr);
        managers_[static_cast<uint8_t>(type)] = manager;
    }
    
    JsonHandleManagerCompact *get_manager(JsonHandleManagerType type)
    {
        assert(type >= JsonHandleManagerType::Scalar && type <= JsonHandleManagerType::Object);
        return managers_[static_cast<uint8_t>(type)];
    }
};

} // namespace hakka

#endif // __HAKKA_JSON_HANDLE_MANAGER_REGISTERY_HPP__