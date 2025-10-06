#include <handles/scalar_manager.hpp>
#include <handles/manager_registry.hpp>
#include <handles/strict_fp_block.hpp>
#include <hakka_json_float.hpp>
#include <hakka_json_int.hpp>

#include <cassert>
#include <cstring>
#include <algorithm>

using namespace hakka;

#if defined(__GNUC__) || defined(__clang__)
#define UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define UNREACHABLE() __assume(0)
#else
#define UNREACHABLE() assert(false)
#endif

namespace
{
    namespace detail
    {
        // We combine the int and float handle managers locally and only expose the scalar manager.
        // Due to NaN boxing, we can't have separate managers for int and float.
        // Memory efficiency is a core concern; exposing these two managers separately
        // would require extra bits for the type, which is not feasible.
        class JsonHandleManagerIntCompact : public JsonHandleManagerCompact
        {
            JsonHandleManagerIntCompact() = default;

        public:
            static JsonHandleManagerIntCompact &get_instance()
            {
                // Internifyer is NOT used for Compact Manager
                static JsonHandleManagerIntCompact instance;
                return instance;
            }

            HandleManagerToken create(int64_t &&value);
            virtual HakkaJsonType type(HandleManagerToken token) const override;
            virtual UniformCompactPointerView get_view(HandleManagerToken token) const override;
            virtual void release(HandleManagerToken token) override;
            // HAKKA_JSON_INT top 3 bits are 001, the rest are the index
            static constexpr auto int_mask = 0x20000000;
            uint32_t get_index(HandleManagerToken token) const
            {
                return token & ~ScalarManagerCompact::scalar_mask;
            }
        };

        class JsonHandleManagerFloatCompact : public JsonHandleManagerCompact
        {
            JsonHandleManagerFloatCompact() = default;

        public:
            static JsonHandleManagerFloatCompact &get_instance()
            {
                static JsonHandleManagerFloatCompact instance;
                return instance;
            }
            
            HandleManagerToken create(double &&value);
            virtual HakkaJsonType type(HandleManagerToken token) const override;
            virtual UniformCompactPointerView get_view(HandleManagerToken token) const override;
            virtual void release(HandleManagerToken token) override;
            // HAKKA_JSON_FLOAT top 3 bits are 000, the rest are the index
            static constexpr auto float_mask = 0x00000000;
            uint32_t get_index(HandleManagerToken token) const
            {
                return token & ~ScalarManagerCompact::scalar_mask;
            }
        };
    }
}

// The create stuff for the compact manager, they are the same as the normal manager
template <>
HandleManagerToken ScalarManagerCompact::create(int64_t &&value)
{
    return detail::JsonHandleManagerIntCompact::get_instance().create(std::move(value));
}

template <>
HandleManagerToken ScalarManagerCompact::create(double &&value)
{
    return detail::JsonHandleManagerFloatCompact::get_instance().create(std::move(value));
}

template <>
HandleManagerToken ScalarManagerCompact::create(bool &&value)
{
    static const auto TRUE_COMPACT_TOKEN = detail::JsonHandleManagerFloatCompact::get_instance().create(double(TRUE_NAN));
    static const auto FALSE_COMPACT_TOKEN = detail::JsonHandleManagerFloatCompact::get_instance().create(double(FALSE_NAN));

    static const auto TF_ARRAY = std::array<HandleManagerToken, 2>{FALSE_COMPACT_TOKEN, TRUE_COMPACT_TOKEN};
    return TF_ARRAY[static_cast<size_t>(value)];
}

template <>
HandleManagerToken ScalarManagerCompact::create(std::nullptr_t &&)
{
    static const auto NULL_COMPACT_TOKEN = detail::JsonHandleManagerFloatCompact::get_instance().create(double(NULL_NAN));
    return NULL_COMPACT_TOKEN;
}

template <>
HandleManagerToken ScalarManagerCompact::create(void *&&)
{
    static const auto INVALID_COMPACT_TOKEN = detail::JsonHandleManagerFloatCompact::get_instance().create(double(INVALID_NAN));
    return INVALID_COMPACT_TOKEN;
}

namespace
{
    namespace detail_compact
    {
        // Just declare the static inline variables here, and define them at the end of the file.
        // They need the ScalarManagerCompact to be defined first.
        // ---- DO NOT CHANGE THE ORDER: KEEP INVALID AS THE FIRST ----
        static inline const HandleManagerToken INVALID_COMPACT_TOKEN = ScalarManagerCompact::get_instance().create((void*)(0));
        // ---- DO NOT CHANGE THE ORDER: KEEP INVALID AS THE FIRST ----
        static inline const HandleManagerToken NULL_COMPACT_TOKEN = ScalarManagerCompact::get_instance().create(nullptr);
        static inline const HandleManagerToken TRUE_COMPACT_TOKEN = ScalarManagerCompact::get_instance().create(true);
        static inline const HandleManagerToken FALSE_COMPACT_TOKEN = ScalarManagerCompact::get_instance().create(false);
    }
}

ScalarManagerCompact &ScalarManagerCompact::get_instance()
{
    static ScalarManagerCompact instance;
    return instance;
}

HakkaJsonType ScalarManagerCompact::type(HandleManagerToken token) const
{
    assert((token & JsonHandleManagerCompact::type_mask) == 0); // Scalar top 2 bits are zero

    if ((token & ScalarManagerCompact::scalar_mask) == detail::JsonHandleManagerIntCompact::int_mask)
        return HakkaJsonType::HAKKA_JSON_INT;
    else if (token == detail_compact::INVALID_COMPACT_TOKEN)
        return HakkaJsonType::HAKKA_JSON_INVALID;
    else if (token == detail_compact::NULL_COMPACT_TOKEN)
        return HakkaJsonType::HAKKA_JSON_NULL;
    else if (token == detail_compact::TRUE_COMPACT_TOKEN)
        return HakkaJsonType::HAKKA_JSON_BOOL;
    else if (token == detail_compact::FALSE_COMPACT_TOKEN)
        return HakkaJsonType::HAKKA_JSON_BOOL;
    else
        return HakkaJsonType::HAKKA_JSON_FLOAT;
}

UniformCompactPointerView ScalarManagerCompact::get_view(HandleManagerToken token) const
{
    assert((token & JsonHandleManagerCompact::type_mask) == 0); // Scalar top 2 bits are zero

    auto target_type = type(token);
    if (target_type == HakkaJsonType::HAKKA_JSON_INT)
        return UniformCompactPointerView(detail::JsonHandleManagerIntCompact::get_instance().get_view(token));
    else if (target_type == HakkaJsonType::HAKKA_JSON_FLOAT)
        return UniformCompactPointerView(detail::JsonHandleManagerFloatCompact::get_instance().get_view(token));
    else if (target_type == HakkaJsonType::HAKKA_JSON_NULL)
        return UniformCompactPointerView(detail::JsonHandleManagerFloatCompact::get_instance().get_view(token));
    else if (target_type == HakkaJsonType::HAKKA_JSON_INVALID)
        return UniformCompactPointerView(detail::JsonHandleManagerFloatCompact::get_instance().get_view(token));
    else if (target_type == HakkaJsonType::HAKKA_JSON_BOOL)
        return UniformCompactPointerView(detail::JsonHandleManagerFloatCompact::get_instance().get_view(token));
    else
        return UniformCompactPointerView(std::monostate{});
}

void ScalarManagerCompact::release(HandleManagerToken token)
{
    assert((token & JsonHandleManagerCompact::type_mask) == 0); // Scalar top 2 bits are zero
    auto target_type = type(token);
    if (target_type == HakkaJsonType::HAKKA_JSON_INT)
        detail::JsonHandleManagerIntCompact::get_instance().release(token);
    else if (target_type == HakkaJsonType::HAKKA_JSON_FLOAT)
        detail::JsonHandleManagerFloatCompact::get_instance().release(token);
    else
        (void)token; // do nothing, because NULL, TRUE, FALSE, INVALID are immortals
}

HandleManagerToken detail::JsonHandleManagerIntCompact::create(int64_t &&value)
{
    auto hash_value = std::hash<int64_t>{}(value);
    std::lock_guard lock(mutex_);
    auto it = hash_to_index_map_.find(hash_value);
    if (it != hash_to_index_map_.end())
    {
        const auto &index_of_active = it->second;
        handles_[index_of_active].get<JsonIntCompact>()->inc_ref();
        return index_of_active | int_mask;
    }
    
    // try to get a free index
    size_t index = -1;
    if (!freelist_.empty())
    {
        std::pop_heap(freelist_.begin(), freelist_.end(), std::greater<size_t>());
        index = freelist_.back();
        freelist_.pop_back();
        handles_[index].emplace(JsonIntCompact::create_unique(std::move(value)));
    }
    else
    {
        index = handles_.size();
        handles_.emplace_back(JsonIntCompact::create_unique(std::move(value)));
    }
    
    hash_to_index_map_[hash_value] = index;
    return index | int_mask;
}

HakkaJsonType detail::JsonHandleManagerIntCompact::type([[maybe_unused]] HandleManagerToken token) const
{
    assert((token & JsonHandleManagerCompact::type_mask) == 0); // Scalar top 2 bits are zero
    return HakkaJsonType::HAKKA_JSON_INT;
}

UniformCompactPointerView detail::JsonHandleManagerIntCompact::get_view(HandleManagerToken token) const
{
    assert((token & JsonHandleManagerCompact::type_mask) == 0); // Scalar top 2 bits are zero
    
    uint32_t index = get_index(token);
    std::lock_guard lock(mutex_);
    return UniformCompactPointerView(handles_[index].get<JsonIntCompact>());
}

void detail::JsonHandleManagerIntCompact::release(HandleManagerToken token)
{
    assert((token & JsonHandleManagerCompact::type_mask) == 0); // Scalar top 2 bits are zero
    uint32_t index = get_index(token);
    
    std::lock_guard lock(mutex_);
    const auto &target = handles_[index];
    if (target.get<JsonIntCompact>()->dec_ref() != 0)
        return; // No need to shrink the vector if nothing is released

    // Release the object
    hash_to_index_map_.erase(target.get<JsonIntCompact>()->hash());
    freelist_.push_back(index);
    std::push_heap(freelist_.begin(), freelist_.end(), std::greater<size_t>());
    handles_[index].emplace(nullptr); // unique_ptr will delete the object

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

HandleManagerToken detail::JsonHandleManagerFloatCompact::create(double &&value)
{
    auto hash_value = JsonFloatCompact::free_hash(value);
    std::lock_guard lock(mutex_);
    auto it = hash_to_index_map_.find(hash_value);
    if (it != hash_to_index_map_.end())
    {
        const auto &index_of_active = it->second;
        handles_[index_of_active].get<JsonFloatCompact>()->inc_ref();
        return index_of_active | float_mask;
    }
    
    // try to get a free index
    size_t index = -1;
    if (!freelist_.empty())
    {
        std::pop_heap(freelist_.begin(), freelist_.end(), std::greater<size_t>());
        index = freelist_.back();
        freelist_.pop_back();
        handles_[index].emplace(JsonFloatCompact::create_unique(std::move(value)));
    }
    else
    {
        index = handles_.size();
        handles_.emplace_back(JsonFloatCompact::create_unique(std::move(value)));
    }
    
    hash_to_index_map_[hash_value] = index;
    return index | float_mask;
}

HakkaJsonType detail::JsonHandleManagerFloatCompact::type([[maybe_unused]] HandleManagerToken token) const
{
    assert((token & JsonHandleManagerCompact::type_mask) == 0); // Scalar top 2 bits are zero
    return HakkaJsonType::HAKKA_JSON_FLOAT;
}

UniformCompactPointerView detail::JsonHandleManagerFloatCompact::get_view(HandleManagerToken token) const
{
    assert((token & JsonHandleManagerCompact::type_mask) == 0); // Scalar top 2 bits are zero
    uint32_t index = get_index(token);
    std::lock_guard lock(mutex_);
    return UniformCompactPointerView(handles_[index].get<JsonFloatCompact>());
}

void detail::JsonHandleManagerFloatCompact::release(HandleManagerToken token)
{
    assert((token & JsonHandleManagerCompact::type_mask) == 0); // Scalar top 2 bits are zero
    uint32_t index = get_index(token);
    
    std::lock_guard lock(mutex_);
    const auto &target = handles_[index];
    if (target.get<JsonFloatCompact>()->dec_ref() != 0)
        return; // No need to shrink the vector if nothing is released

    // Release the object
    hash_to_index_map_.erase(target.get<JsonFloatCompact>()->hash());
    freelist_.push_back(index);
    std::push_heap(freelist_.begin(), freelist_.end(), std::greater<size_t>());
    handles_[index].emplace(nullptr); // unique_ptr will delete the object

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

const static auto init_registry_compact = []() -> bool
{
    JsonHandleManagerRegistryCompact::get_instance().register_manager(JsonHandleManagerType::Scalar,
                                                               &ScalarManagerCompact::get_instance());
    return true;
}();
