#include <handles/array_manager.hpp>
#include <handles/manager_registry.hpp>
#include <hakka_json_array.hpp>

#include <algorithm>

using namespace hakka;

namespace {
    // Track reentrancy to prevent shrinking during nested releases
    thread_local int release_depth = 0;
}

uint32_t ArrayManagerCompact::get_index(HandleManagerToken token) const
{
    return token & ~JsonHandleManagerCompact::type_mask;
}

ArrayManagerCompact &ArrayManagerCompact::get_instance()
{
    static ArrayManagerCompact instance;
    return instance;
}

HakkaJsonType ArrayManagerCompact::type([[maybe_unused]] HandleManagerToken token) const
{
    assert((token & JsonHandleManagerCompact::type_mask) == array_mask);
    return HakkaJsonType::HAKKA_JSON_ARRAY;
}

UniformCompactPointerView ArrayManagerCompact::get_view(HandleManagerToken token) const
{
    assert((token & JsonHandleManagerCompact::type_mask) == array_mask);
    const auto &index = get_index(token);
    std::lock_guard lock(mutex_);
    return UniformCompactPointerView(handles_[index].get<JsonArrayCompact>());
}

UniformCompactPointer ArrayManagerCompact::get_mut_ptr(HandleManagerToken token) const
{
    assert((token & JsonHandleManagerCompact::type_mask) == array_mask);
    const auto &index = get_index(token);
    std::lock_guard lock(mutex_);
    return UniformCompactPointer(handles_[index].get<JsonArrayCompact>());
}

void ArrayManagerCompact::release(HandleManagerToken token) {
    assert((token & JsonHandleManagerCompact::type_mask) == array_mask);
    const auto &index = get_index(token);
    std::lock_guard lock(mutex_);

    ++release_depth;

    const auto &target = handles_[index];
    if (target.get<JsonArrayCompact>()->dec_ref() != 0) {
        --release_depth;
        return;
    }

    handles_[index].emplace(nullptr);
    freelist_.push_back(index);
    std::push_heap(freelist_.begin(), freelist_.end(), std::greater<size_t>());

    // Only shrink at the outermost release to avoid reentrancy issues
    if (--release_depth != 0 || JsonHandleManagerCompact::should_skip_shrinking(handles_, freelist_)) 
        return;

    auto last_active = std::find_if(handles_.rbegin(), handles_.rend(), [](const OwnedUniformCompactPointer &ptr)
                                    { return ptr != nullptr; });

    if (last_active == handles_.rbegin())
        return;

    if (last_active == handles_.rend()) {
        handles_.clear();
        freelist_.clear();
        return;
    }

    const auto last_active_index = static_cast<uint32_t>(std::distance(handles_.begin(), last_active.base()));
    handles_.erase(handles_.begin() + last_active_index, handles_.end());
    freelist_.erase(std::remove_if(freelist_.begin(), freelist_.end(), [last_active_index](size_t index)
                                    { return index >= last_active_index; }),
                    freelist_.end());
    std::make_heap(freelist_.begin(), freelist_.end(), std::greater<size_t>());

    handles_.shrink_to_fit();
    freelist_.shrink_to_fit();
}

HandleManagerToken ArrayManagerCompact::create()
{
    std::lock_guard lock(mutex_);
    size_t index;
    if (freelist_.empty())
    {
        index = handles_.size();
        handles_.emplace_back(JsonArrayCompact::create_unique());
    }
    else
    {
        std::pop_heap(freelist_.begin(), freelist_.end(), std::greater<size_t>());
        index = freelist_.back();
        freelist_.pop_back();
        handles_[index].emplace(JsonArrayCompact::create_unique());
    }
    return index | array_mask;
}

const static auto init_registry_compact = []() -> bool
{
    JsonHandleManagerRegistryCompact::get_instance().register_manager(JsonHandleManagerType::Array,
                                                                       &ArrayManagerCompact::get_instance());
    return true;
}();
