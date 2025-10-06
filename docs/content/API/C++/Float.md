# JsonFloatCompact

`JsonFloatCompact` represents immutable floating-point values and NaN-boxed types (Bool, Null, Invalid) in the Hakka JSON library. It inherits from `JsonPrimitiveCompact<JsonFloatCompact, double>` and uses the CRTP pattern for compile-time polymorphism.

**Header**: `include/hakka_json_float.hpp`

## NaN-Boxing Architecture

`JsonFloatCompact` uses NaN-boxing to store multiple types in a single 64-bit IEEE 754 double:

- **FLOAT**: Normal floating-point values
- **BOOL**: `TRUE_NAN` and `FALSE_NAN` bit patterns
- **NULL**: `NULL_NAN` bit pattern
- **INVALID**: `INVALID_NAN` bit pattern

Type discrimination is performed by examining the bit pattern at runtime using the `type()` method.

!!! note
    See [Primitive Types Overview](Primitive.md) for detailed information about NaN-boxing technique.

!!! warning
    You do not need to manually create the NaN-boxed singleton instances (Bool, Null, Invalid). They are immortal instances created during static initialization in `src/handles/scalar_manager.cpp`. The `create()` overloads for these types simply return handles to pre-existing singletons. Directly modifying reference counts for these singleton objects results in undefined behavior.

## Type Definitions

```cpp
using ValueType = double;
```

## Factory Methods

### create (float)

```cpp
[[nodiscard]] static JsonHandleCompact create(ValueType value);
```

Creates a `JsonFloatCompact` instance for a floating-point value. Automatically deduplicates identical values.

- **Parameters**:
    - `value`: The 64-bit floating-point value (`double`).

- **Returns**:
    - A `JsonHandleCompact` that references the created `JsonFloatCompact` instance.

- **Error Handling**:
    - Throws `std::bad_alloc` if memory allocation fails.
    - Exception safety: Basic guarantee.

- **Complexity**:
    - $O(1)$ average case time complexity (hash table lookup/insert with deduplication).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(3.14159);
    ```

---

### create (bool)

```cpp
[[nodiscard]] static JsonHandleCompact create(bool value);
```

Creates a NaN-boxed boolean value. Returns a handle to a singleton instance.

- **Parameters**:
    - `value`: Boolean value (`true` or `false`).

- **Returns**:
    - A `JsonHandleCompact` referencing the singleton `TRUE_NAN` or `FALSE_NAN` instance.

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
    auto false_handle = JsonFloatCompact::create(false);
    ```

---

### create (null)

```cpp
[[nodiscard]] static JsonHandleCompact create(std::nullptr_t value);
```

Creates a NaN-boxed null value. Returns a handle to a singleton instance.

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
    ```

---

### create (invalid)

```cpp
[[nodiscard]] static JsonHandleCompact create();
```

Creates an invalid/uninitialized value. Returns a handle with token value 0.

- **Parameters**:
    - None.

- **Returns**:
    - A `JsonHandleCompact` with token value 0 (INVALID state).

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
    ```

---

### create_unique

```cpp
[[nodiscard]] static std::unique_ptr<JsonFloatCompact> create_unique(ValueType value);
```

Creates a unique pointer to a `JsonFloatCompact` instance for a floating-point value. Does not deduplicate values.

- **Parameters**:
    - `value`: The 64-bit floating-point value (`double`).

- **Returns**:
    - A `std::unique_ptr<JsonFloatCompact>` owning the created instance.
    - Returns `nullptr` if memory allocation fails.

- **Error Handling**:
    - Returns `nullptr` on allocation failure (uses `new (std::nothrow)`).
    - No exceptions thrown.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto floatPtr = JsonFloatCompact::create_unique(2.71828);
    if (floatPtr) {
        // Use floatPtr
    }
    ```

---

## Serialization Methods

### dump

```cpp
tl::expected<std::string, HakkaJsonResultEnum> dump(uint32_t max_depth = 0) const;
```

Serializes the value to a string representation. The output depends on the NaN-boxed type.

- **Parameters**:
    - `max_depth`: Unused for `JsonFloatCompact`; included for interface consistency.

- **Returns**:
    - On success: A `std::string` containing:
        - `"null"` for NULL type
        - `"true"` or `"false"` for BOOL type
        - `"INVALID"` for INVALID type
        - Decimal representation (using `%g` format) for FLOAT type
    - On failure: `HAKKA_JSON_INTERNAL_ERROR` (rare).

- **Error Handling**:
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ for NULL, BOOL, INVALID types.
    - $O(d)$ for FLOAT type, where $d$ is the number of significant digits.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(3.14);
    auto view = handle.get_view();
    if (auto* float_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        auto result = (*float_ptr)->dump();
        if (result) {
            std::cout << result.value() << std::endl; // Output: 3.14
        }
    }

    auto null_handle = JsonFloatCompact::create(nullptr);
    auto null_view = null_handle.get_view();
    if (auto* null_ptr = std::get_if<const JsonFloatCompact*>(&null_view)) {
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

Serializes the value into a null-terminated byte buffer.

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
    - $\Theta(1)$ for NULL, BOOL, INVALID types.
    - $O(d)$ for FLOAT type, where $d$ is the number of significant digits.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(2.718);
    auto view = handle.get_view();
    if (auto* float_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        char buffer[20];
        uint32_t size = sizeof(buffer);
        auto result = (*float_ptr)->to_bytes(buffer, &size);
        if (result == HAKKA_JSON_SUCCESS) {
            std::cout << buffer << std::endl; // Output: 2.718
        }
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
    - The number of bytes required to serialize the value (excluding null terminator).
    - Returns 0 if `dump()` fails.

- **Error Handling**:
    - Returns 0 on dump failure (uses `value_or("")`).
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ for NULL, BOOL, INVALID types.
    - $O(d)$ for FLOAT type, where $d$ is the number of significant digits (calls `dump()` internally).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(nullptr);
    auto view = handle.get_view();
    if (auto* null_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t size = (*null_ptr)->dump_size();
        std::cout << "Size: " << size << std::endl; // Output: Size: 4 (for "null")
    }
    ```

---

## Type Information Methods

### type

```cpp
HakkaJsonType type() const;
```

Determines the type by examining the NaN bit pattern.

- **Parameters**:
    - None.

- **Returns**:
    - `HAKKA_JSON_NULL` if value matches `NULL_NAN` bit pattern
    - `HAKKA_JSON_BOOL` if value matches `TRUE_NAN` or `FALSE_NAN` bit pattern
    - `HAKKA_JSON_INVALID` if value matches `INVALID_NAN` bit pattern
    - `HAKKA_JSON_FLOAT` otherwise

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (bit pattern comparison).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(true);
    HakkaJsonType t = handle.get_type();
    assert(t == HAKKA_JSON_BOOL);

    auto float_handle = JsonFloatCompact::create(3.14);
    assert(float_handle.get_type() == HAKKA_JSON_FLOAT);
    ```

---

## Comparison Methods

### compare

```cpp
tl::expected<int, HakkaJsonResultEnum> compare(const JsonHandleCompact &other) const;
```

Compares this value with another JSON value. Supports comparison with `JsonIntCompact`, `JsonFloatCompact`, `JsonBoolCompact`, and `JsonNullCompact` types. The comparison behavior depends on the NaN-boxed type.

- **Parameters**:
    - `other`: The `JsonHandleCompact` to compare against.

- **Returns**:
    - On success: An integer comparison result:
        - `< 0` if this value is less than `other`
        - `0` if this value equals `other`
        - `> 0` if this value is greater than `other`
    - On failure: `HAKKA_JSON_TYPE_ERROR` if `other` is not `INT`, `FLOAT`, `BOOL`, or `NULL`.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if type is incompatible or if this instance is INVALID type.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle1 = JsonFloatCompact::create(3.14);
    auto handle2 = JsonFloatCompact::create(2.71);

    auto view1 = handle1.get_view();
    if (auto* float_ptr = std::get_if<const JsonFloatCompact*>(&view1)) {
        auto result = (*float_ptr)->compare(handle2);
        if (result) {
            if (result.value() > 0) {
                std::cout << "3.14 > 2.71" << std::endl;
            }
        }
    }
    ```

---

### hash

```cpp
uint64_t hash() const;
```

Computes the hash value by hashing the bit representation of the double value.

- **Parameters**:
    - None.

- **Returns**:
    - A 64-bit hash value. Special NaN values have unique hashes based on their bit patterns.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(3.14);
    auto view = handle.get_view();
    if (auto* float_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t h = (*float_ptr)->hash();
        std::cout << "Hash: " << h << std::endl;
    }
    ```

---

### free_hash

```cpp
static uint64_t free_hash(double value);
```

Static utility function that computes hash for a floating-point value without creating an instance.

- **Parameters**:
    - `value`: The floating-point value to hash.

- **Returns**:
    - A 64-bit hash value based on the bit representation of the value.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    double value = 3.14;
    uint64_t h = JsonFloatCompact::free_hash(value);
    std::cout << "Hash: " << h << std::endl;
    ```

---

## Value Access Methods

### get

```cpp
tl::expected<PrimitiveType, HakkaJsonResultEnum> get() const;
```

Retrieves the value, returning the appropriate type based on the NaN-boxed type.

- **Parameters**:
    - None.

- **Returns**:
    - On success: A `PrimitiveType` containing:
        - `nullptr` for NULL type
        - `bool` (true/false) for BOOL type
        - `double` for FLOAT type
    - On failure: `HAKKA_JSON_TYPE_ERROR` for INVALID type.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` for INVALID type.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (bit pattern checks and value extraction).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonFloatCompact::create(3.14);
    auto view = handle.get_view();
    if (auto* float_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        auto value = (*float_ptr)->get();
        if (value) {
            double float_val = std::get<double>(value.value());
            std::cout << "Value: " << float_val << std::endl; // Output: Value: 3.14
        }
    }

    auto bool_handle = JsonFloatCompact::create(true);
    auto bool_view = bool_handle.get_view();
    if (auto* bool_ptr = std::get_if<const JsonFloatCompact*>(&bool_view)) {
        auto value = (*bool_ptr)->get();
        if (value) {
            bool bool_val = std::get<bool>(value.value());
            std::cout << "Value: " << std::boolalpha << bool_val << std::endl; // Output: Value: true
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
    auto handle = JsonFloatCompact::create(3.14);
    auto view = handle.get_view();
    if (auto* float_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t count = (*float_ptr)->inc_ref();
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
    auto handle = JsonFloatCompact::create(3.14);
    auto view = handle.get_view();
    if (auto* float_ptr = std::get_if<const JsonFloatCompact*>(&view)) {
        uint64_t count = (*float_ptr)->dec_ref();
        std::cout << "New ref count: " << count << std::endl;
        // Note: Manually calling dec_ref() is not recommended; use Handle system
    }
    ```

!!! warning
    Manual reference counting is intended for Python FFI integration. Normal usage should rely on the `JsonHandleCompact` system for automatic reference management.

---

## Memory Management

### Float Values

Float values use automatic deduplication. When multiple handles are created with the same floating-point value (including NaN values), they reference the same underlying object:

```cpp
auto handle1 = JsonFloatCompact::create(3.14);
auto handle2 = JsonFloatCompact::create(3.14);
// Both handles reference the same JsonFloatCompact instance
```

### NaN-Boxed Singletons

Bool, Null, and Invalid values are implemented as immortal singletons that are never deallocated:

```cpp
auto true1 = JsonFloatCompact::create(true);
auto true2 = JsonFloatCompact::create(true);
// Both reference the same singleton TRUE_NAN instance

auto null1 = JsonFloatCompact::create(nullptr);
auto null2 = JsonFloatCompact::create(nullptr);
// Both reference the same singleton NULL_NAN instance
```

The singleton instances are created during static initialization and persist for the program's lifetime.

## See Also

- [Primitive Types Overview](Primitive.md) - NaN-boxing architecture details
- [Handle System](Handle.md) - Handle-based memory management
- [JsonIntCompact](Int.md) - Integer type
- [JsonBoolCompact](Bool.md) - Boolean type (NaN-boxed)
- [JsonNullCompact](Null.md) - Null type (NaN-boxed)
- [JsonInvalidCompact](Invalid.md) - Invalid type (NaN-boxed)
