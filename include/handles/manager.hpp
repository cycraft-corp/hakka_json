#ifndef __HAKKA_JSON_HANDLE_MANAGER_HPP__
#define __HAKKA_JSON_HANDLE_MANAGER_HPP__

#pragma once

#include <cstdint>
#include <vector>
#include <mutex>
#include <unordered_map>

#include <hakka_json_enum.h>

#include <uniform_compact_pointer.hpp>

namespace hakka {

/**
 * @enum JsonHandleManagerType
 * @brief Enum representing different types of JSON objects managed by JsonHandleManager.
 */
enum class JsonHandleManagerType
{
    Scalar = 0,
    String = 1,
    Array = 2,
    Object = 3,
};

/**
 * @typedef HandleManagerToken
 * @brief A 32-bit token that uniquely identifies and encodes information about a JSON object.
 *
 * **HandleManagerToken Binary Format:**
 *
 * The `HandleManagerToken` is a 32-bit unsigned integer that encodes two pieces of information:
 *
 * 1. **Type (2 bits):**
 *    - The top 2 bits (bits 31 and 30) store the type of the object.
 *    - This allows the system to differentiate between various object types.
 *
 * 2. **Index (30 bits):**
 *    - The remaining 30 bits (bits 29 through 0) store the index of the object in the `handles_` array.
 *    - This index is used to access the corresponding object in `handles_`.
 *
 * **Binary Layout:**
 * ```
 * | 31 ... 30 | 29 ... 0 |
 * |   Type    |  Index   |
 * ```
 *
 * **Example:**
 *
 * - A token with value `0x80000001`:
 *     - Binary: `10000000 00000000 00000000 00000001`
 *     - Type: `10` (binary) -> Type 2 (`Array`)
 *     - Index: `1` (decimal) -> Refers to `handles_[1]`
 *
 * - A token with value `0x40000000`:
 *     - Binary: `01000000 00000000 00000000 00000000`
 *     - Type: `01` (binary) -> Type 1 (`String`)
 *     - Index: `0` (decimal) -> Refers to `handles_[0]`
 */
using HandleManagerToken = uint32_t;

/**
 * @class JsonHandleManagerCompact
 * @brief Manages handles to JSON objects, providing efficient storage and retrieval mechanisms.
 *
 * This class is responsible for allocating, managing, and releasing handles to JSON objects.
 * It uses a combination of a vector (`handles_`) to store object pointers and a min-heap (`freelist_`)
 * to manage free indices for reuse, optimizing both space and access time.
 */
class JsonHandleManagerCompact
{
protected:
    static constexpr auto type_mask = 0xC0000000; /**< Mask to extract the type bits from a handle token. */
    // The array might contain multiple elements, so we need to use the recursive mutex
    mutable std::recursive_mutex mutex_; /**< Mutex to ensure thread safety for handle management operations. */

    /**
    * @brief Vector storing pointers to active JSON objects.
    *
    * **Handle Management Algorithm:**
    *
    * - **`handles_`:**
    *     - An array storing pointers to reachable JSON objects (`OwnedUniformCompactPointer`).
    *     - Indices in `handles_` remain fixed even if the object at that index is released (set to `nullptr`).
    *     - New objects are inserted into indices from `freelist_` if available; otherwise, they are appended to the end of `handles_`.
    *
    * - **`freelist_`:**
    *     - A min-heap storing free indices in `handles_`.
    *     - Ensures the smallest index is reused first to keep `handles_` as compact as possible.
    *
    * - **Shrink-to-fit:**
    *     - Triggered when all objects in the range `(last_active, end)` are inactive.
    *     - This range is collapsed, reducing the size of `handles_`.
    *     - Active indices in the middle of `handles_` are **not** reorganized or collected.
    *
    * **Example States:**
    *
    * - **Initial State:**
    *   ```
    *   handles_: [ Obj0 | Obj1 | Obj2 | Obj3 | Obj4 ]
    *   indices:    0      1      2      3      4
    *   freelist_: []
    *   ```
    *
    * - **After releasing Obj1 and Obj3:**
    *   ```
    *   handles_: [ Obj0 | nullptr | Obj2 | nullptr | Obj4 ]
    *   indices:    0       1        2       3       4
    *   freelist_: [1, 3]  (Min-heap of free indices)
    *   ```
    *
    * - **Adding a new object (Obj5):**
    *   ```
    *   handles_: [ Obj0 | Obj5 | Obj2 | nullptr | Obj4 ]
    *   indices:    0      1      2       3        4
    *   freelist_: [3]    (Index 1 reused from freelist_)
    *   ```
    *
    * - **Shrink-to-fit triggered (if all elements in `(last_active, end)` are inactive):**
    *   - **Before shrink:**
    *     ```
    *     handles_: [ Obj0 | Obj5 | Obj2 | nullptr | nullptr ]
    *     indices:    0      1      2        3        4
    *     freelist_: [3, 4]
    *     ```
    *   - **After shrink:**
    *     ```
    *     handles_: [ Obj0 | Obj5 | Obj2 ]
    *     indices:    0      1      2
    *     freelist_: []
    *     ```
    */
    std::vector<OwnedUniformCompactPointer> handles_; /**< Array of pointers to active JSON objects. */
    std::vector<size_t> freelist_;                   /**< Min-heap of free indices for reuse. */

    /**
    * @brief Mapping from hash values to indices in `handles_`.
    *
    * This map allows quick lookup of objects based on their hash values, facilitating efficient retrieval.
    * Uses a vector of indices to support hash collisions (e.g., for Python-compatible hashing where
    * 0.0, False, and 0 all hash to the same value).
    */
    std::unordered_map<std::size_t, std::vector<std::size_t>> hash_to_index_map_;

public:
    JsonHandleManagerCompact() = default;
    virtual ~JsonHandleManagerCompact() = default;

    /**
    * @brief Get the type of a JSON object using its handle.
    * @param token The handle token representing the JSON object.
    * @return The type of the JSON object.
    */
    virtual HakkaJsonType type(HandleManagerToken token) const = 0;

    /**
    * @brief Retrieve the pointer to a JSON object using its handle.
    * @param token The handle token representing the JSON object.
    * @return Pointer to the JSON object (`UniformCompactPointerView`).
    */
    virtual UniformCompactPointerView get_view(HandleManagerToken token) const = 0;

    virtual UniformCompactPointer get_mut_ptr(HandleManagerToken /*token*/) const {
        // for immutable types, return an invalid pointer
        // Stucture types need to override this
        return UniformCompactPointer(std::monostate{});
    };

    /**
    * @brief Release the handle, decrementing its reference count.
    * @param token The handle token to be released.
    */
    virtual void release(HandleManagerToken token) = 0;

    /**
    * @note The `create` function is not implemented here because its behavior varies across derived classes.
    * Derived classes must implement their own `create` method to generate new handles.
    *
    * **Example:**
    * ```
    * virtual HandleManagerToken create(Args... args) = 0;
    * ```
    */

    /**
    * @brief Heuristic to determine if handles should be shrunk.
    * @param handles The handles vector
    * @param freelist The freelist vector  
    * @return true if shrinking should be skipped (too many free slots), false if shrinking should proceed
    */
    static bool should_skip_shrinking(const std::vector<OwnedUniformCompactPointer> &handles, const std::vector<size_t> &freelist)
    {
        // This is a heuristic approach: If the freelist is more than half of the handles, don't bother shrinking.
        // Why not shrink the handles every time? Because it's 50x slower, and we don't need to shrink every time.
        return freelist.size() < handles.size() / 2;
    }

protected:
};

} // namespace hakka

#endif // __HAKKA_JSON_HANDLE_MANAGER_HPP__
