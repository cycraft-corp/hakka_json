#include <handles/string_manager.hpp>
#include <handles/manager_registry.hpp>
#include <hakka_json_string.hpp>

#include <algorithm>

using namespace hakka;

uint32_t StringManagerCompact::get_index(HandleManagerToken token) const
{
    return static_cast<uint32_t>(token & ~JsonHandleManagerCompact::type_mask);
}

StringManagerCompact &StringManagerCompact::get_instance()
{
    static StringManagerCompact instance;
    return instance;
}

HakkaJsonType StringManagerCompact::type([[maybe_unused]] HandleManagerToken token) const
{
    assert((token & JsonHandleManagerCompact::type_mask) == string_mask); // String top 2 bits are 01
    return HakkaJsonType::HAKKA_JSON_STRING;
}

UniformCompactPointerView StringManagerCompact::get_view(HandleManagerToken token) const
{
    assert((token & JsonHandleManagerCompact::type_mask) == string_mask); // String top 2 bits are 01
    const auto &index = get_index(token);
    std::lock_guard lock(mutex_);
    return UniformCompactPointerView(handles_[index].get<JsonStringCompact>());
}

void StringManagerCompact::release(HandleManagerToken token)
{
    assert((token & JsonHandleManagerCompact::type_mask) == string_mask); // String top 2 bits are 01
    const auto &index = get_index(token);
    
    std::lock_guard lock(mutex_);
    const auto &target = handles_[index];
    if (target.get<JsonStringCompact>()->dec_ref() != 0)
        return; // No need to shrink the vector if nothing is released

    // Release the object - remove from hash map
    auto hash_value = target.get<JsonStringCompact>()->hash();
    auto it = hash_to_index_map_.find(hash_value);
    if (it != hash_to_index_map_.end())
    {
        auto &indices = it->second;
        // Remove this index from the vector
        indices.erase(std::remove(indices.begin(), indices.end(), static_cast<size_t>(index)), indices.end());
        // If the vector is now empty, remove the hash entry entirely
        if (indices.empty())
        {
            hash_to_index_map_.erase(it);
        }
    }
    
    freelist_.push_back(index);
    std::push_heap(freelist_.begin(), freelist_.end(), std::greater<size_t>());
    handles_[index].emplace(nullptr);

    if (JsonHandleManagerCompact::should_skip_shrinking(handles_, freelist_)) [[likely]] {
        return;
    }

    // Check if we need to shrink the vector
    auto last_active = std::find_if(handles_.rbegin(), handles_.rend(), [](const OwnedUniformCompactPointer &ptr)
                                    { return ptr != nullptr; });
    if (last_active == handles_.rbegin()) // No need to shrink
        return;
    
    // Erase elements from (last_active_index, rbegin), they are all nullptr
    const auto last_active_index = static_cast<uint32_t>(std::distance(handles_.begin(), last_active.base()));
    handles_.erase(handles_.begin() + last_active_index, handles_.end());
    freelist_.erase(std::remove_if(freelist_.begin(), freelist_.end(), [last_active_index](size_t index)
                                   { return index >= last_active_index; }),
                    freelist_.end());
    // Rebuild the heap
    std::make_heap(freelist_.begin(), freelist_.end(), std::greater<size_t>());

    // Shrink to fit
    handles_.shrink_to_fit();
    freelist_.shrink_to_fit();
    // hash_to_index_map_ is already updated when we release the object
}

HandleManagerToken StringManagerCompact::create(std::string_view value)
{
    auto hash_value = std::hash<std::string_view>{}(value);
    std::lock_guard lock(mutex_);
    auto it = hash_to_index_map_.find(hash_value);
    if (it != hash_to_index_map_.end())
    {
        // Check all indices with this hash for an exact match
        const auto &indices = it->second;
        for (const auto &token : indices)
        {
            const auto *existing = handles_[token].get<JsonStringCompact>();
            if (existing->value_.to_string_view() == value)
            {
                existing->inc_ref();
                return static_cast<HandleManagerToken>(token) | string_mask;
            }
        }
    }

    // No exact match found, create new entry
    size_t index = static_cast<size_t>(-1);
    if (!freelist_.empty())
    {
        std::pop_heap(freelist_.begin(), freelist_.end(), std::greater<size_t>());
        index = freelist_.back();
        freelist_.pop_back();
        handles_[index].emplace(JsonStringCompact::create_unique(value));
    }
    else
    {
        index = handles_.size();
        handles_.emplace_back(JsonStringCompact::create_unique(value));
    }

    hash_to_index_map_[hash_value].push_back(index);
    return static_cast<HandleManagerToken>(index) | string_mask;
}

static const auto init_registry_compact = []()
{
    JsonHandleManagerRegistryCompact::get_instance().register_manager(JsonHandleManagerType::String,
                                                                       &StringManagerCompact::get_instance());
    return true;
}();
