# JsonInvalidCompact

`JsonInvalidCompact` represents an invalid or uninitialized value in the Hakka JSON library. It is implemented as a NaN-boxed singleton within `JsonFloatCompact` using the `INVALID_NAN` bit pattern.

**Important**: `JsonInvalidCompact` is not a standalone class. It is a type marker used in the `UniformCompactPointerView` variant system. The actual implementation resides in `JsonFloatCompact`.

**Header**: `include/hakka_json_float.hpp`

## Architecture

`JsonInvalidCompact` is implemented using NaN-boxing:

- **Storage**: Single 64-bit double with `INVALID_NAN` bit pattern
- **Singleton**: One immortal instance shared by all invalid handles
- **Special Token**: Invalid handles have token value `0` (sentinel value)
- **Lifetime**: Never deallocated (created during static initialization)
- **Type Discrimination**: The `type()` method returns `HAKKA_JSON_INVALID` when it detects the `INVALID_NAN` bit pattern

!!! note
    See [Primitive Types Overview](Primitive.md#nan-boxing-technique) for detailed information about NaN-boxing.

!!! warning
    You do not need to manually create these NaN-boxed singleton instances. They are immortal instances created during static initialization in `src/handles/scalar_manager.cpp`. The `create()` method simply returns a handle to these pre-existing singletons. Directly modifying reference counts for these singleton objects results in undefined behavior.

## Use Cases

Invalid values represent:

- **Uninitialized handles**: Default-constructed `JsonHandleCompact` instances
- **Error states**: Operations that fail to produce valid values
- **Sentinel values**: Explicit markers for missing or undefined data

## Factory Methods

### create

```cpp
[[nodiscard]] static JsonHandleCompact create();
```

Creates an invalid value by returning a handle with token value `0`.

- **Parameters**:
    - None.

- **Returns**:
    - A `JsonHandleCompact` with token value `0` (INVALID state).

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto invalid_handle = JsonFloatCompact::create();
    assert(invalid_handle.get_type() == HAKKA_JSON_INVALID);
    assert(!invalid_handle.is_valid()); // Token is 0
    ```

---

## Serialization Methods

### dump

```cpp
tl::expected<std::string, HakkaJsonResultEnum> dump(uint32_t max_depth = 0) const;
```

Serializes the invalid value to the string `"INVALID"`.

- **Parameters**:
    - `max_depth`: Unused for `JsonInvalidCompact`; included for interface consistency.

- **Returns**:
    - On success: A `std::string` containing `"INVALID"`.
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
    auto handle = JsonFloatCompact::create();
    auto view = handle.get_view();
    if (auto* invalid_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        auto result = (*invalid_ptr)->dump();
        if (result) {
            std::cout << result.value() << std::endl; // Output: INVALID
        }
    }
    ```

---

### to_bytes

```cpp
HakkaJsonResultEnum to_bytes(char *buffer, uint32_t *buffer_size) const;
```

Serializes the invalid value into a null-terminated byte buffer.

- **Parameters**:
    - `buffer`: Pointer to the destination byte buffer.
    - `buffer_size`: Pointer to the buffer size. On input, specifies the buffer capacity. On output, contains the number of bytes written (excluding null terminator) or the required size if buffer is too small.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if serialization succeeds.
    - `HAKKA_JSON_NOT_ENOUGH_MEMORY` if the buffer is too small. The required size is written to `*buffer_size`.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` when `*buffer_size < 8` (7 bytes for "INVALID" + 1 for null terminator).
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (copies constant-size string).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create();
    auto view = handle.get_view();
    if (auto* invalid_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        char buffer[10];
        uint32_t size = sizeof(buffer);
        auto result = (*invalid_ptr)->to_bytes(buffer, &size);
        if (result == HAKKA_JSON_SUCCESS) {
            std::cout << buffer << std::endl; // Output: INVALID
            std::cout << "Written: " << size << " bytes" << std::endl; // Output: 7
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
    - Always returns `7` (size of `"INVALID"`).

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (returns constant value).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create();
    auto view = handle.get_view();
    if (auto* invalid_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t size = (*invalid_ptr)->dump_size();
        std::cout << "Size: " << size << std::endl; // Output: Size: 7
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
    - `HAKKA_JSON_INVALID`

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (bit pattern check).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create();
    HakkaJsonType t = handle.get_type();
    assert(t == HAKKA_JSON_INVALID);
    ```

---

## Comparison Methods

### compare

```cpp
tl::expected<int, HakkaJsonResultEnum> compare(const JsonHandleCompact &other) const;
```

Attempts to compare this invalid value with another JSON value. Always returns an error.

- **Parameters**:
    - `other`: The `JsonHandleCompact` to compare against.

- **Returns**:
    - Always returns `HAKKA_JSON_TYPE_ERROR` (invalid values cannot be compared).

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` for all comparisons.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto invalid_handle = JsonFloatCompact::create();
    auto int_handle = JsonIntCompact::create(42);

    auto view = invalid_handle.get_view();
    if (auto* invalid_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        auto result = (*invalid_ptr)->compare(int_handle);
        if (!result) {
            assert(result.error() == HAKKA_JSON_TYPE_ERROR);
        }
    }
    ```

---

### hash

```cpp
uint64_t hash() const;
```

Computes the hash value of the invalid value by hashing the `INVALID_NAN` bit representation.

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
    auto handle = JsonFloatCompact::create();
    auto view = handle.get_view();
    if (auto* invalid_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t h = (*invalid_ptr)->hash();
        std::cout << "Hash: " << h << std::endl;
    }
    ```

---

## Value Access Methods

### get

```cpp
tl::expected<PrimitiveType, HakkaJsonResultEnum> get() const;
```

Attempts to retrieve the value of an invalid instance. Always returns an error.

- **Parameters**:
    - None.

- **Returns**:
    - Always returns `HAKKA_JSON_TYPE_ERROR` (invalid values have no accessible value).

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` for all calls.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create();
    auto view = handle.get_view();
    if (auto* invalid_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        auto value = (*invalid_ptr)->get();
        if (!value) {
            assert(value.error() == HAKKA_JSON_TYPE_ERROR);
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
    auto handle = JsonFloatCompact::create();
    auto view = handle.get_view();
    if (auto* invalid_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t count = (*invalid_ptr)->inc_ref();
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
    auto handle = JsonFloatCompact::create();
    auto view = handle.get_view();
    if (auto* invalid_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t count = (*invalid_ptr)->dec_ref();
        std::cout << "New ref count: " << count << std::endl;
        // Note: Manually calling dec_ref() is not recommended; use Handle system
    }
    ```

!!! warning
    Manual reference counting is intended for Python FFI integration. Normal usage should rely on the `JsonHandleCompact` system for automatic reference management.

---

## Default Construction

Default-constructed handles are automatically invalid:

```cpp
JsonHandleCompact handle;  // Default constructed
assert(handle.get_type() == HAKKA_JSON_INVALID);
assert(!handle); // Boolean conversion returns false for invalid handles
```

## Singleton Behavior

All invalid handles reference the same underlying singleton:

```cpp
auto invalid1 = JsonFloatCompact::create();
auto invalid2 = JsonFloatCompact::create();
// Both reference the same INVALID_NAN singleton instance

// Default-constructed handles also reference the same singleton
JsonHandleCompact invalid3;
// invalid3 also represents the invalid state (token = 0)
```

## Handle Token Value

Invalid handles have a special token value of `0`:

```cpp
JsonHandleCompact invalid_handle = JsonFloatCompact::create();
// Internal token is 0

// This is why boolean conversion works:
if (!invalid_handle) {
    std::cout << "Handle is invalid" << std::endl;
}

// And why is_valid() returns false:
assert(!invalid_handle.is_valid());
```

## Memory Management

The invalid singleton is created during static initialization and persists for the program's lifetime. It is never deallocated, regardless of how many handles reference it.

**Reference Counting**: While the singleton has reference counting methods, they do not affect its lifetime. The singleton is immortal.

**Handle Release**: Calling `release()` on an invalid handle sets its token to `0` (which is already the invalid state).

## Common Patterns

### Error Handling

Use invalid handles to represent error states:

```cpp
JsonHandleCompact parse_value(const std::string& input) {
    if (input.empty()) {
        return JsonFloatCompact::create(); // Return invalid for error
    }
    // Parse and return valid handle
    return JsonIntCompact::create(42);
}

auto result = parse_value("");
if (!result.is_valid()) {
    std::cout << "Parsing failed" << std::endl;
}
```

### Initialization

Use invalid handles as sentinel values before initialization:

```cpp
JsonHandleCompact handle; // Default-constructed (invalid)

// Later, initialize with actual value
if (some_condition) {
    handle = JsonIntCompact::create(42);
}

// Check if initialization occurred
if (handle.is_valid()) {
    // Use handle
}
```

## See Also

- [JsonFloatCompact](Float.md) - Complete API documentation (invalid is implemented within JsonFloatCompact)
- [Primitive Types Overview](Primitive.md) - NaN-boxing architecture
- [JsonBoolCompact](Bool.md) - Boolean type (also NaN-boxed)
- [JsonNullCompact](Null.md) - Null type (also NaN-boxed)
- [Handle System](Handle.md) - Handle-based memory management and validity checks
