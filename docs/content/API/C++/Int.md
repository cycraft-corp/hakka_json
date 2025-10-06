# JsonIntCompact

`JsonIntCompact` represents immutable 64-bit signed integer values in the Hakka JSON library. It inherits from `JsonPrimitiveCompact<JsonIntCompact, int64_t>` and uses the CRTP pattern for compile-time polymorphism.

**Header**: `include/hakka_json_int.hpp`

## Type Definitions

```cpp
using ValueType = int64_t;
```

## Factory Methods

### create

```cpp
[[nodiscard]] static JsonHandleCompact create(ValueType value);
```

Creates a `JsonIntCompact` instance and returns a handle to it. Automatically deduplicates identical values.

- **Parameters**:
    - `value`: The 64-bit signed integer value (`int64_t`).

- **Returns**:
    - A `JsonHandleCompact` that references the created `JsonIntCompact` instance.

- **Error Handling**:
    - Throws `std::bad_alloc` if memory allocation fails.
    - Exception safety: Basic guarantee (may leak if allocation fails during manager operations).

- **Complexity**:
    - $O(1)$ average case time complexity (hash table lookup/insert with deduplication).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonIntCompact::create(42);
    ```

---

### create_unique

```cpp
[[nodiscard]] static std::unique_ptr<JsonIntCompact> create_unique(ValueType value);
```

Creates a unique pointer to a `JsonIntCompact` instance. Does not deduplicate values.

- **Parameters**:
    - `value`: The 64-bit signed integer value (`int64_t`).

- **Returns**:
    - A `std::unique_ptr<JsonIntCompact>` owning the created instance.
    - Returns `nullptr` if memory allocation fails.

- **Error Handling**:
    - Returns `nullptr` on allocation failure (uses `new (std::nothrow)`).
    - No exceptions thrown.

- **Complexity**:
    - $O(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto intPtr = JsonIntCompact::create_unique(100);
    if (intPtr) {
        // Use intPtr
    }
    ```

---

## Serialization Methods

### dump

```cpp
tl::expected<std::string, HakkaJsonResultEnum> dump(uint32_t max_depth = 0) const;
```

Serializes the integer value to a string representation.

- **Parameters**:
    - `max_depth`: Unused for `JsonIntCompact`; included for interface consistency.

- **Returns**:
    - On success: A `std::string` containing the decimal representation of the integer.
    - On failure: `HAKKA_JSON_INTERNAL_ERROR` (rare, occurs on exception).

- **Error Handling**:
    - Returns `HAKKA_JSON_INTERNAL_ERROR` if `std::to_string` throws an exception.
    - Exception safety: Strong guarantee (no state changes on failure).

- **Complexity**:
    - $O(\log |v|)$ time complexity, where $v$ is the integer value (proportional to number of digits).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonIntCompact::create(42);
    auto view = handle.get_view();
    if (auto* int_ptr = std::get_if<const JsonIntCompact*>(&view)) {
        auto result = (*int_ptr)->dump();
        if (result) {
            std::cout << result.value() << std::endl; // Output: 42
        }
    }
    ```

---

### to_bytes

```cpp
HakkaJsonResultEnum to_bytes(char *buffer, uint32_t *buffer_size) const;
```

Serializes the integer value into a null-terminated byte buffer.

- **Parameters**:
    - `buffer`: Pointer to the destination byte buffer.
    - `buffer_size`: Pointer to the buffer size. On input, specifies the buffer capacity. On output, contains the number of bytes written (excluding null terminator) or the required size if buffer is too small.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if serialization succeeds.
    - `HAKKA_JSON_NOT_ENOUGH_MEMORY` if the buffer is too small. The required size is written to `*buffer_size`.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` when `*buffer_size < required_size`.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(\log |v|)$ time complexity, where $v$ is the integer value (proportional to number of digits).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonIntCompact::create(12345);
    auto view = handle.get_view();
    if (auto* int_ptr = std::get_if<const JsonIntCompact*>(&view)) {
        char buffer[20];
        uint32_t size = sizeof(buffer);
        auto result = (*int_ptr)->to_bytes(buffer, &size);
        if (result == HAKKA_JSON_SUCCESS) {
            std::cout << buffer << std::endl; // Output: 12345
            std::cout << "Written: " << size << " bytes" << std::endl;
        }
    }
    ```

    **Insufficient buffer case**:
    ```cpp
    char buffer[3];
    uint32_t size = sizeof(buffer);
    auto result = (*int_ptr)->to_bytes(buffer, &size);
    if (result == HAKKA_JSON_NOT_ENOUGH_MEMORY) {
        std::cout << "Buffer too small, need " << size << " bytes" << std::endl;
    }
    ```

---

### dump_size

```cpp
uint64_t dump_size() const;
```

Calculates the size of the serialized string representation.

- **Parameters**:
    - None.

- **Returns**:
    - The number of bytes required to serialize the integer (excluding null terminator).

- **Error Handling**:
    - None (assumes `dump()` succeeds).
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(\log |v|)$ time complexity, where $v$ is the integer value (calls `dump()` internally).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonIntCompact::create(-9876);
    auto view = handle.get_view();
    if (auto* int_ptr = std::get_if<const JsonIntCompact*>(&view)) {
        uint64_t size = (*int_ptr)->dump_size();
        std::cout << "Size: " << size << std::endl; // Output: Size: 5 (for "-9876")
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
    - `HAKKA_JSON_INT`

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonIntCompact::create(42);
    HakkaJsonType t = handle.get_type();
    assert(t == HAKKA_JSON_INT);
    ```

---

## Comparison Methods

### compare

```cpp
tl::expected<int, HakkaJsonResultEnum> compare(const JsonHandleCompact &other) const;
```

Compares this integer with another JSON value. Supports comparison with `JsonIntCompact`, `JsonFloatCompact`, and `JsonBoolCompact` types.

- **Parameters**:
    - `other`: The `JsonHandleCompact` to compare against.

- **Returns**:
    - On success: An integer comparison result:
        - `< 0` if this value is less than `other`
        - `0` if this value equals `other`
        - `> 0` if this value is greater than `other`
    - On failure: `HAKKA_JSON_TYPE_ERROR` if `other` is not `INT`, `FLOAT`, or `BOOL`.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if type is incompatible.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle1 = JsonIntCompact::create(42);
    auto handle2 = JsonIntCompact::create(100);

    auto view1 = handle1.get_view();
    if (auto* int_ptr = std::get_if<const JsonIntCompact*>(&view1)) {
        auto result = (*int_ptr)->compare(handle2);
        if (result) {
            if (result.value() < 0) {
                std::cout << "42 < 100" << std::endl;
            }
        }
    }
    ```

    **Type error case**:
    ```cpp
    auto handle1 = JsonIntCompact::create(42);
    auto handle2 = JsonStringCompact::create("hello");

    auto view1 = handle1.get_view();
    if (auto* int_ptr = std::get_if<const JsonIntCompact*>(&view1)) {
        auto result = (*int_ptr)->compare(handle2);
        if (!result) {
            std::cout << "Type error: " << result.error() << std::endl;
        }
    }
    ```

---

### hash

```cpp
uint64_t hash() const;
```

Computes the hash value of the integer using `std::hash<int64_t>`.

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
    auto handle = JsonIntCompact::create(42);
    auto view = handle.get_view();
    if (auto* int_ptr = std::get_if<const JsonIntCompact*>(&view)) {
        uint64_t h = (*int_ptr)->hash();
        std::cout << "Hash: " << h << std::endl;
    }
    ```

---

## Value Access Methods

### get

```cpp
tl::expected<PrimitiveType, HakkaJsonResultEnum> get() const;
```

Retrieves the integer value stored in this instance.

- **Parameters**:
    - None.

- **Returns**:
    - On success: A `PrimitiveType` containing the `int64_t` value.
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
    auto handle = JsonIntCompact::create(42);
    auto view = handle.get_view();
    if (auto* int_ptr = std::get_if<const JsonIntCompact*>(&view)) {
        auto value = (*int_ptr)->get();
        if (value) {
            int64_t int_val = std::get<int64_t>(value.value());
            std::cout << "Value: " << int_val << std::endl; // Output: Value: 42
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
    auto handle = JsonIntCompact::create(42);
    auto view = handle.get_view();
    if (auto* int_ptr = std::get_if<const JsonIntCompact*>(&view)) {
        uint64_t count = (*int_ptr)->inc_ref();
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
    auto handle = JsonIntCompact::create(42);
    auto view = handle.get_view();
    if (auto* int_ptr = std::get_if<const JsonIntCompact*>(&view)) {
        uint64_t count = (*int_ptr)->dec_ref();
        std::cout << "New ref count: " << count << std::endl;
        // Note: Manually calling dec_ref() is not recommended; use Handle system
    }
    ```

!!! warning
    Manual reference counting is intended for Python FFI integration. Normal usage should rely on the `JsonHandleCompact` system for automatic reference management.

---

## Memory Management

`JsonIntCompact` uses automatic value deduplication. When multiple handles are created with the same integer value, they reference the same underlying object:

```cpp
auto handle1 = JsonIntCompact::create(42);
auto handle2 = JsonIntCompact::create(42);
// Both handles reference the same JsonIntCompact instance
```

The instance is destroyed when its reference count reaches zero.

## See Also

- [Primitive Types Overview](Primitive.md)
- [Handle System](Handle.md)
- [JsonFloatCompact](Float.md)
- [JsonStringCompact](String.md)
