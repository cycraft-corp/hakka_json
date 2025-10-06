# JsonBoolCompact

`JsonBoolCompact` represents boolean values (true/false) in the Hakka JSON library. It is implemented as a NaN-boxed singleton within `JsonFloatCompact` using the `TRUE_NAN` and `FALSE_NAN` bit patterns.

**Important**: `JsonBoolCompact` is not a standalone class. It is a type marker used in the `UniformCompactPointerView` variant system. The actual implementation resides in `JsonFloatCompact`.

**Header**: `include/hakka_json_float.hpp`

## Architecture

`JsonBoolCompact` is implemented using NaN-boxing:

- **Storage**: Single 64-bit double with `TRUE_NAN` or `FALSE_NAN` bit pattern
- **Singleton**: Two immortal instances (one for true, one for false)
- **Lifetime**: Never deallocated (created during static initialization)
- **Type Discrimination**: The `type()` method returns `HAKKA_JSON_BOOL` when it detects the `TRUE_NAN` or `FALSE_NAN` bit pattern

!!! note
    See [Primitive Types Overview](Primitive.md#nan-boxing-technique) for detailed information about NaN-boxing.

!!! warning
    You do not need to manually create these NaN-boxed singleton instances. They are immortal instances created during static initialization in `src/handles/scalar_manager.cpp`. The `create()` method simply returns a handle to these pre-existing singletons. Directly modifying reference counts for these singleton objects results in undefined behavior.

## Factory Methods

### create

```cpp
[[nodiscard]] static JsonHandleCompact create(bool value);
```

Creates a boolean value by returning a handle to the singleton `TRUE_NAN` or `FALSE_NAN` instance.

- **Parameters**:
    - `value`: Boolean value (`true` or `false`).

- **Returns**:
    - A `JsonHandleCompact` referencing the singleton `TRUE_NAN` instance (if `value` is `true`) or `FALSE_NAN` instance (if `value` is `false`).

- **Error Handling**:
    - None (singletons are pre-allocated).
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (returns pre-existing singleton).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto true_handle = JsonFloatCompact::create(true);
    assert(true_handle.get_type() == HAKKA_JSON_BOOL);

    auto false_handle = JsonFloatCompact::create(false);
    assert(false_handle.get_type() == HAKKA_JSON_BOOL);
    ```

---

## Serialization Methods

### dump

```cpp
tl::expected<std::string, HakkaJsonResultEnum> dump(uint32_t max_depth = 0) const;
```

Serializes the boolean value to the string `"true"` or `"false"`.

- **Parameters**:
    - `max_depth`: Unused for `JsonBoolCompact`; included for interface consistency.

- **Returns**:
    - On success: A `std::string` containing `"true"` or `"false"`.
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
    auto true_handle = JsonFloatCompact::create(true);
    auto view = true_handle.get_view();
    if (auto* bool_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        auto result = (*bool_ptr)->dump();
        if (result) {
            std::cout << result.value() << std::endl; // Output: true
        }
    }

    auto false_handle = JsonFloatCompact::create(false);
    auto view2 = false_handle.get_view();
    if (auto* bool_ptr = std::get_if<const JsonFloatCompact*>(&view2)) {
        auto result = (*bool_ptr)->dump();
        if (result) {
            std::cout << result.value() << std::endl; // Output: false
        }
    }
    ```

---

### to_bytes

```cpp
HakkaJsonResultEnum to_bytes(char *buffer, uint32_t *buffer_size) const;
```

Serializes the boolean value into a null-terminated byte buffer.

- **Parameters**:
    - `buffer`: Pointer to the destination byte buffer.
    - `buffer_size`: Pointer to the buffer size. On input, specifies the buffer capacity. On output, contains the number of bytes written (excluding null terminator) or the required size if buffer is too small.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if serialization succeeds.
    - `HAKKA_JSON_NOT_ENOUGH_MEMORY` if the buffer is too small. The required size is written to `*buffer_size`.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` when `*buffer_size < 5` (for "true") or `*buffer_size < 6` (for "false").
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (copies constant-size string).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto true_handle = JsonFloatCompact::create(true);
    auto view = true_handle.get_view();
    if (auto* bool_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        char buffer[10];
        uint32_t size = sizeof(buffer);
        auto result = (*bool_ptr)->to_bytes(buffer, &size);
        if (result == HAKKA_JSON_SUCCESS) {
            std::cout << buffer << std::endl; // Output: true
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
    - Returns `4` for `true` (size of `"true"`).
    - Returns `5` for `false` (size of `"false"`).

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (returns constant value based on bit pattern).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto true_handle = JsonFloatCompact::create(true);
    auto view = true_handle.get_view();
    if (auto* bool_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t size = (*bool_ptr)->dump_size();
        std::cout << "Size: " << size << std::endl; // Output: Size: 4
    }

    auto false_handle = JsonFloatCompact::create(false);
    auto view2 = false_handle.get_view();
    if (auto* bool_ptr = std::get_if<const JsonFloatCompact*>(&view2)) {
        uint64_t size = (*bool_ptr)->dump_size();
        std::cout << "Size: " << size << std::endl; // Output: Size: 5
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
    - `HAKKA_JSON_BOOL`

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (bit pattern check).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(true);
    HakkaJsonType t = handle.get_type();
    assert(t == HAKKA_JSON_BOOL);
    ```

---

## Comparison Methods

### compare

```cpp
tl::expected<int, HakkaJsonResultEnum> compare(const JsonHandleCompact &other) const;
```

Compares this boolean value with another JSON value. Supports comparison with `JsonIntCompact`, `JsonFloatCompact`, `JsonBoolCompact`, and `JsonNullCompact` types.

- **Parameters**:
    - `other`: The `JsonHandleCompact` to compare against.

- **Returns**:
    - On success: An integer comparison result:
        - `< 0` if this value is less than `other`
        - `0` if this value equals `other`
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
    auto true_handle = JsonFloatCompact::create(true);
    auto false_handle = JsonFloatCompact::create(false);

    auto view1 = true_handle.get_view();
    if (auto* bool_ptr = std::get_if<const JsonFloatCompact*>(&view1)) {
        auto result = (*bool_ptr)->compare(false_handle);
        if (result) {
            assert(result.value() > 0); // true > false
        }
    }

    // Comparing two true values
    auto true_handle2 = JsonFloatCompact::create(true);
    auto view2 = true_handle.get_view();
    if (auto* bool_ptr = std::get_if<const JsonFloatCompact*>(&view2)) {
        auto result = (*bool_ptr)->compare(true_handle2);
        if (result) {
            assert(result.value() == 0); // true == true
        }
    }
    ```

**Comparison Rules**:
- `true == true` returns `0`
- `false == false` returns `0`
- `true > false` returns positive value
- `false < true` returns negative value
- Boolean compared with other types: Implementation-defined ordering

---

### hash

```cpp
uint64_t hash() const;
```

Computes the hash value of the boolean value by hashing the `TRUE_NAN` or `FALSE_NAN` bit representation.

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
    auto handle = JsonFloatCompact::create(true);
    auto view = handle.get_view();
    if (auto* bool_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t h = (*bool_ptr)->hash();
        std::cout << "Hash: " << h << std::endl;
    }
    ```

---

## Value Access Methods

### get

```cpp
tl::expected<PrimitiveType, HakkaJsonResultEnum> get() const;
```

Retrieves the boolean value wrapped in a `PrimitiveType`.

- **Parameters**:
    - None.

- **Returns**:
    - On success: A `PrimitiveType` containing `bool` (`true` or `false`).
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
    auto handle = JsonFloatCompact::create(true);
    auto view = handle.get_view();
    if (auto* bool_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        auto value = (*bool_ptr)->get();
        if (value) {
            bool bool_val = std::get<bool>(value.value());
            assert(bool_val == true);
            std::cout << "Value: " << std::boolalpha << bool_val << std::endl;
            // Output: Value: true
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
    auto handle = JsonFloatCompact::create(true);
    auto view = handle.get_view();
    if (auto* bool_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t count = (*bool_ptr)->inc_ref();
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
    auto handle = JsonFloatCompact::create(true);
    auto view = handle.get_view();
    if (auto* bool_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t count = (*bool_ptr)->dec_ref();
        std::cout << "New ref count: " << count << std::endl;
        // Note: Manually calling dec_ref() is not recommended; use Handle system
    }
    ```

!!! warning
    Manual reference counting is intended for Python FFI integration. Normal usage should rely on the `JsonHandleCompact` system for automatic reference management.

---

## Singleton Behavior

All true handles reference the same `TRUE_NAN` singleton, and all false handles reference the same `FALSE_NAN` singleton:

```cpp
auto true1 = JsonFloatCompact::create(true);
auto true2 = JsonFloatCompact::create(true);
// Both reference the same TRUE_NAN singleton instance

auto false1 = JsonFloatCompact::create(false);
auto false2 = JsonFloatCompact::create(false);
// Both reference the same FALSE_NAN singleton instance

// The singletons are never deallocated
// Reference counting does not apply to the singletons
```

## Memory Management

The boolean singletons are created during static initialization and persist for the program's lifetime. They are never deallocated, regardless of how many handles reference them.

**Reference Counting**: While the singletons have reference counting methods, they do not affect their lifetime. The singletons are immortal.

## See Also

- [JsonFloatCompact](Float.md) - Complete API documentation (bool is implemented within JsonFloatCompact)
- [Primitive Types Overview](Primitive.md) - NaN-boxing architecture
- [JsonNullCompact](Null.md) - Null type (also NaN-boxed)
- [JsonInvalidCompact](Invalid.md) - Invalid type (also NaN-boxed)
- [Handle System](Handle.md) - Handle-based memory management
