# JsonNullCompact

`JsonNullCompact` represents the null value in the Hakka JSON library. It is implemented as a NaN-boxed singleton within `JsonFloatCompact` using the `NULL_NAN` bit pattern.

**Important**: `JsonNullCompact` is not a standalone class. It is a type marker used in the `UniformCompactPointerView` variant system. The actual implementation resides in `JsonFloatCompact`.

**Header**: `include/hakka_json_float.hpp`

## Architecture

`JsonNullCompact` is implemented using NaN-boxing:

- **Storage**: Single 64-bit double with `NULL_NAN` bit pattern
- **Singleton**: One immortal instance shared by all null handles
- **Lifetime**: Never deallocated (created during static initialization)
- **Type Discrimination**: The `type()` method returns `HAKKA_JSON_NULL` when it detects the `NULL_NAN` bit pattern

!!! note
    See [Primitive Types Overview](Primitive.md#nan-boxing-technique) for detailed information about NaN-boxing.

!!! warning
    You do not need to manually create these NaN-boxed singleton instances. They are immortal instances created during static initialization in `src/handles/scalar_manager.cpp`. The `create()` method simply returns a handle to these pre-existing singletons. Directly modifying reference counts for these singleton objects results in undefined behavior.

## Factory Methods

### create

```cpp
[[nodiscard]] static JsonHandleCompact create(std::nullptr_t value);
```

Creates a null value by returning a handle to the singleton `NULL_NAN` instance.

- **Parameters**:
    - `value`: `nullptr` literal.

- **Returns**:
    - A `JsonHandleCompact` referencing the singleton `NULL_NAN` instance.

- **Error Handling**:
    - None (singleton is pre-allocated).
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (returns pre-existing singleton).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto null_handle = JsonFloatCompact::create(nullptr);
    assert(null_handle.get_type() == HAKKA_JSON_NULL);
    ```

---

## Serialization Methods

### dump

```cpp
tl::expected<std::string, HakkaJsonResultEnum> dump(uint32_t max_depth = 0) const;
```

Serializes the null value to the string `"null"`.

- **Parameters**:
    - `max_depth`: Unused for `JsonNullCompact`; included for interface consistency.

- **Returns**:
    - On success: A `std::string` containing `"null"`.
    - On failure: `HAKKA_JSON_INTERNAL_ERROR` (rare, occurs on exception).

- **Error Handling**:
    - Returns `HAKKA_JSON_INTERNAL_ERROR` if string allocation throws an exception.
    - Exception safety: Strong guarantee (no state changes on failure).

- **Complexity**:
    - $\Theta(1)$ time complexity (returns constant string).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(nullptr);
    auto view = handle.get_view();
    if (auto* null_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        auto result = (*null_ptr)->dump();
        if (result) {
            std::cout << result.value() << std::endl; // Output: null
        }
    }
    ```

---

### to_bytes

```cpp
HakkaJsonResultEnum to_bytes(char *buffer, uint32_t *buffer_size) const;
```

Serializes the null value into a null-terminated byte buffer.

- **Parameters**:
    - `buffer`: Pointer to the destination byte buffer.
    - `buffer_size`: Pointer to the buffer size. On input, specifies the buffer capacity. On output, contains the number of bytes written (excluding null terminator) or the required size if buffer is too small.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if serialization succeeds.
    - `HAKKA_JSON_NOT_ENOUGH_MEMORY` if the buffer is too small. The required size is written to `*buffer_size`.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` when `*buffer_size < 5` (4 bytes for "null" + 1 for null terminator).
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (copies constant-size string).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(nullptr);
    auto view = handle.get_view();
    if (auto* null_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        char buffer[10];
        uint32_t size = sizeof(buffer);
        auto result = (*null_ptr)->to_bytes(buffer, &size);
        if (result == HAKKA_JSON_SUCCESS) {
            std::cout << buffer << std::endl; // Output: null
            std::cout << "Written: " << size << " bytes" << std::endl; // Output: 4
        }
    }
    ```

---

### dump_size

```cpp
uint64_t dump_size() const;
```

Returns the size of the serialized string representation.

- **Parameters**:
    - None.

- **Returns**:
    - Always returns `4` (size of `"null"`).

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (returns constant value).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(nullptr);
    auto view = handle.get_view();
    if (auto* null_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t size = (*null_ptr)->dump_size();
        std::cout << "Size: " << size << std::endl; // Output: Size: 4
    }
    ```

---

## Type Information Methods

### type

```cpp
HakkaJsonType type() const;
```

Returns the JSON type of this instance.

- **Parameters**:
    - None.

- **Returns**:
    - `HAKKA_JSON_NULL`

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (bit pattern check).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(nullptr);
    HakkaJsonType t = handle.get_type();
    assert(t == HAKKA_JSON_NULL);
    ```

---

## Comparison Methods

### compare

```cpp
tl::expected<int, HakkaJsonResultEnum> compare(const JsonHandleCompact &other) const;
```

Compares this null value with another JSON value. Supports comparison with `JsonIntCompact`, `JsonFloatCompact`, `JsonBoolCompact`, and `JsonNullCompact` types.

- **Parameters**:
    - `other`: The `JsonHandleCompact` to compare against.

- **Returns**:
    - On success: An integer comparison result:
        - `< 0` if this value is less than `other`
        - `0` if this value equals `other` (both are null)
        - `> 0` if this value is greater than `other`
    - On failure: `HAKKA_JSON_TYPE_ERROR` if `other` is not `INT`, `FLOAT`, `BOOL`, or `NULL`.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if type is incompatible.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto null1 = JsonFloatCompact::create(nullptr);
    auto null2 = JsonFloatCompact::create(nullptr);

    auto view1 = null1.get_view();
    if (auto* null_ptr = std::get_if<const JsonFloatCompact*>(&view1)) {
        auto result = (*null_ptr)->compare(null2);
        if (result) {
            assert(result.value() == 0); // null == null
        }
    }
    ```

---

### hash

```cpp
uint64_t hash() const;
```

Computes the hash value of the null value by hashing the `NULL_NAN` bit representation.

- **Parameters**:
    - None.

- **Returns**:
    - A 64-bit hash value.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(nullptr);
    auto view = handle.get_view();
    if (auto* null_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t h = (*null_ptr)->hash();
        std::cout << "Hash: " << h << std::endl;
    }
    ```

---

## Value Access Methods

### get

```cpp
tl::expected<PrimitiveType, HakkaJsonResultEnum> get() const;
```

Retrieves the null value as `std::nullptr_t` wrapped in a `PrimitiveType`.

- **Parameters**:
    - None.

- **Returns**:
    - On success: A `PrimitiveType` containing `std::nullptr_t`.
    - On failure: Error (rare, should not occur for valid instances).

- **Error Handling**:
    - No errors expected for valid instances.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(nullptr);
    auto view = handle.get_view();
    if (auto* null_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        auto value = (*null_ptr)->get();
        if (value) {
            auto null_val = std::get<std::nullptr_t>(value.value());
            assert(null_val == nullptr);
        }
    }
    ```

---

## Reference Counting Methods

### inc_ref

```cpp
uint64_t inc_ref() const;
```

Increments the reference count atomically.

- **Parameters**:
    - None.

- **Returns**:
    - The new reference count after incrementing.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe (uses atomic operations with `memory_order_relaxed`).

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(nullptr);
    auto view = handle.get_view();
    if (auto* null_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t count = (*null_ptr)->inc_ref();
        std::cout << "New ref count: " << count << std::endl;
    }
    ```

---

### dec_ref

```cpp
uint64_t dec_ref() const;
```

Decrements the reference count atomically.

- **Parameters**:
    - None.

- **Returns**:
    - The new reference count after decrementing.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe (uses atomic operations with `memory_order_relaxed`).

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(nullptr);
    auto view = handle.get_view();
    if (auto* null_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t count = (*null_ptr)->dec_ref();
        std::cout << "New ref count: " << count << std::endl;
        // Note: Manually calling dec_ref() is not recommended; use Handle system
    }
    ```

!!! warning
    Manual reference counting is intended for Python FFI integration. Normal usage should rely on the `JsonHandleCompact` system for automatic reference management.

---

## Singleton Behavior

All null handles reference the same underlying singleton:

```cpp
auto null1 = JsonFloatCompact::create(nullptr);
auto null2 = JsonFloatCompact::create(nullptr);
// Both reference the same NULL_NAN singleton instance

// The singleton is never deallocated
// Reference counting does not apply to the singleton
```

## Memory Management

The null singleton is created during static initialization and persists for the program's lifetime. It is never deallocated, regardless of how many handles reference it.

**Reference Counting**: While the singleton has reference counting methods, they do not affect its lifetime. The singleton is immortal.

## See Also

- [JsonFloatCompact](Float.md) - Complete API documentation (null is implemented within JsonFloatCompact)
- [Primitive Types Overview](Primitive.md) - NaN-boxing architecture
- [JsonBoolCompact](Bool.md) - Boolean type (also NaN-boxed)
- [JsonInvalidCompact](Invalid.md) - Invalid type (also NaN-boxed)
- [Handle System](Handle.md) - Handle-based memory management
