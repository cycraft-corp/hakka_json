# JsonArrayCompact

`JsonArrayCompact` represents mutable JSON arrays in the Hakka JSON library. Unlike primitive types which use value deduplication, arrays are mutable containers that store sequences of `JsonHandleCompact` elements.

**Header**: `include/hakka_json_array.hpp`

## Architecture

`JsonArrayCompact` is a mutable container type with the following characteristics:

- **Storage**: `std::vector<JsonHandleCompact>` for dynamic element storage
- **Mutability**: Arrays can be modified after creation (unlike immutable primitive types)
- **No Deduplication**: Each array instance maintains its own element storage
- **CRTP Pattern**: Inherits from `JsonStructuredCompact<JsonArrayCompact>` for compile-time polymorphism
- **Reference Counting**: Atomic reference counting for Python FFI integration
- **Handle Management**: Managed by `ArrayManagerCompact` with type mask `0x80000000`

!!! note
    Arrays are mutable containers designed for dynamic collections. They do not use the value deduplication strategy employed by primitive types.

## Factory Methods

### create

```cpp
[[nodiscard]] static JsonHandleCompact create();
```

Creates an empty JSON array.

- **Parameters**:
    - None.

- **Returns**:
    - A `JsonHandleCompact` referencing a new empty array.

- **Error Handling**:
    - Returns invalid handle if allocation fails.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(1) amortized time complexity (may trigger vector reallocation).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    assert(array.get_type() == HAKKA_JSON_ARRAY);
    ```

---

### create_unique

```cpp
[[nodiscard]] static std::unique_ptr<JsonArrayCompact> create_unique();
```

Creates an empty JSON array as a unique pointer for direct ownership.

- **Parameters**:
    - None.

- **Returns**:
    - A `std::unique_ptr<JsonArrayCompact>` owning a new empty array.

- **Error Handling**:
    - Returns `nullptr` if allocation fails.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array_ptr = JsonArrayCompact::create_unique();
    assert(array_ptr != nullptr);
    ```

---

### loads (std::string)

```cpp
static tl::expected<JsonHandleCompact, HakkaJsonResultEnum> loads(const std::string &json_str, uint32_t max_depth = 2048);
```

Deserializes a JSON string into a `JsonArrayCompact` instance.

- **Parameters**:
    - `json_str`: The JSON string to deserialize.
    - `max_depth`: Maximum recursion depth for nested structures (default: 2048).

- **Returns**:
    - On success: A `JsonHandleCompact` containing the deserialized array.
    - On failure: `HAKKA_JSON_TYPE_ERROR` if the JSON is not an array, `HAKKA_JSON_RECURSION_DEPTH_EXCEEDED` if max depth is exceeded, or other parsing errors.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if the root JSON value is not an array.
    - Returns `HAKKA_JSON_RECURSION_DEPTH_EXCEEDED` if nesting exceeds `max_depth`.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the length of the JSON string.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    std::string json_str = R"([1, 2, 3])";
    auto array = JsonArrayCompact::loads(json_str);
    if (array) {
        auto view = array.value().get_view();
        auto* arr = std::get<const JsonArrayCompact*>(view);
        assert(arr->length() == 3);
    }
    ```

---

### loads (std::string_view)

```cpp
static tl::expected<JsonHandleCompact, HakkaJsonResultEnum> loads(std::string_view json_str, uint32_t max_depth = 2048);
```

Deserializes a JSON string view into a `JsonArrayCompact` instance.

- **Parameters**:
    - `json_str`: The JSON string view to deserialize.
    - `max_depth`: Maximum recursion depth for nested structures (default: 2048).

- **Returns**:
    - On success: A `JsonHandleCompact` containing the deserialized array.
    - On failure: `HAKKA_JSON_TYPE_ERROR` if the JSON is not an array, `HAKKA_JSON_RECURSION_DEPTH_EXCEEDED` if max depth is exceeded, or other parsing errors.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if the root JSON value is not an array.
    - Returns `HAKKA_JSON_RECURSION_DEPTH_EXCEEDED` if nesting exceeds `max_depth`.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the length of the JSON string.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    const char json_str[] = R"(["value1", "value2"])";
    auto array = JsonArrayCompact::loads(std::string_view(json_str, sizeof(json_str) - 1));
    if (array) {
        auto view = array.value().get_view();
        auto* arr = std::get<const JsonArrayCompact*>(view);
        assert(arr->length() == 2);
    }
    ```

---

## Serialization Methods

### dump

```cpp
tl::expected<std::string, HakkaJsonResultEnum> dump(uint32_t max_depth = 0) const;
```

Serializes the array to a JSON string.

- **Parameters**:
    - `max_depth`: Maximum recursion depth for nested structures (0 means no limit).

- **Returns**:
    - On success: A `std::string` containing the JSON representation.
    - On failure: `HAKKA_JSON_RECURSION_DEPTH_EXCEEDED` if max depth is reached, `HAKKA_JSON_INTERNAL_ERROR` on exception.

- **Error Handling**:
    - Returns `HAKKA_JSON_RECURSION_DEPTH_EXCEEDED` when `max_depth` is exceeded.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` if string allocation throws an exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the total number of elements including nested structures.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto view = array.get_view();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));

    auto* arr_view = std::get<const JsonArrayCompact*>(view);
    auto result = arr_view->dump(512);
    if (result) {
        std::cout << result.value() << std::endl; // Output: [42]
    }
    ```

---

### to_bytes

```cpp
HakkaJsonResultEnum to_bytes(char *buffer, uint32_t *buffer_size) const;
```

Serializes the array into a null-terminated byte buffer.

- **Parameters**:
    - `buffer`: Pointer to the destination byte buffer.
    - `buffer_size`: Pointer to the buffer size. On input, specifies the buffer capacity. On output, contains the number of bytes written (excluding null terminator) or the required size if buffer is too small.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if serialization succeeds.
    - `HAKKA_JSON_NOT_ENOUGH_MEMORY` if the buffer is too small. The required size is written to `*buffer_size`.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` when buffer capacity is insufficient.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the total number of elements including nested structures.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));

    char buffer[512];
    uint32_t size = sizeof(buffer);
    auto view = array.get_view();
    auto* arr_view = std::get<const JsonArrayCompact*>(view);
    auto result = arr_view->to_bytes(buffer, &size);
    if (result == HAKKA_JSON_SUCCESS) {
        std::cout << buffer << std::endl; // Output: [42]
        std::cout << "Written: " << size << " bytes" << std::endl;
    }
    ```

---

### dump_size

```cpp
uint64_t dump_size() const;
```

Returns the size of the serialized JSON string representation.

- **Parameters**:
    - None.

- **Returns**:
    - The number of bytes required to serialize the array (excluding null terminator).

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the total number of elements including nested structures.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));

    auto view = array.get_view();
    auto* arr_view = std::get<const JsonArrayCompact*>(view);
    uint64_t size = arr_view->dump_size();
    std::cout << "Size: " << size << std::endl; // Output: Size: 4 (for "[42]")
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
    - `HAKKA_JSON_ARRAY`

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    HakkaJsonType t = array.get_type();
    assert(t == HAKKA_JSON_ARRAY);
    ```

---

## Comparison Methods

### compare

```cpp
tl::expected<int, HakkaJsonResultEnum> compare(const JsonHandleCompact &other) const;
```

Compares this array with another JSON value. Arrays are compared element-wise lexicographically.

- **Parameters**:
    - `other`: The `JsonHandleCompact` to compare against.

- **Returns**:
    - On success: An integer comparison result:
        - `< 0` if this array is less than `other`
        - `0` if this array equals `other`
        - `> 0` if this array is greater than `other`
    - On failure: `HAKKA_JSON_TYPE_ERROR` if `other` is not an array.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `other` is not `HAKKA_JSON_ARRAY`.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(min(n,m)) time complexity where n and m are the lengths of the two arrays.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array1 = JsonArrayCompact::create();
    auto array2 = JsonArrayCompact::create();

    auto* arr1 = std::get<JsonArrayCompact*>(array1.get_mut_ptr());
    auto* arr2 = std::get<JsonArrayCompact*>(array2.get_mut_ptr());

    arr1->push_back(JsonIntCompact::create(1));
    arr2->push_back(JsonIntCompact::create(1));

    auto view1 = array1.get_view();
    auto* arr1_view = std::get<const JsonArrayCompact*>(view1);
    auto result = arr1_view->compare(array2);
    if (result) {
        assert(result.value() == 0); // Equal arrays
    }
    ```

---

### hash

```cpp
uint64_t hash() const;
```

Computes the hash value of the array by XORing the hashes of all elements.

- **Parameters**:
    - None.

- **Returns**:
    - A 64-bit hash value.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - O(n) time complexity where n is the number of elements.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));

    auto view = array.get_view();
    auto* arr_view = std::get<const JsonArrayCompact*>(view);
    uint64_t h = arr_view->hash();
    std::cout << "Hash: " << h << std::endl;
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
    auto array = JsonArrayCompact::create();
    auto view = array.get_view();
    auto* arr = std::get<const JsonArrayCompact*>(view);
    uint64_t count = arr->inc_ref();
    std::cout << "New ref count: " << count << std::endl;
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
    auto array = JsonArrayCompact::create();
    auto view = array.get_view();
    auto* arr = std::get<const JsonArrayCompact*>(view);
    uint64_t count = arr->dec_ref();
    std::cout << "New ref count: " << count << std::endl;
    // Note: Manually calling dec_ref() is not recommended; use Handle system
    ```

!!! warning
    Manual reference counting is intended for Python FFI integration. Normal usage should rely on the `JsonHandleCompact` system for automatic reference management.

---

## Element Access Methods

### get

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> get(KeyType key) const;
```

Retrieves an element at the specified index. This is part of the `JsonStructuredCompact` interface.

- **Parameters**:
    - `key`: A `KeyType` variant containing an `int64_t` index.

- **Returns**:
    - On success: A `JsonHandleCompact` referencing the element at the index.
    - On failure: `HAKKA_JSON_TYPE_ERROR` if key is not `int64_t`, `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is out of range.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `key` does not hold an `int64_t`.
    - Returns `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is negative or >= length.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));

    auto view = array.get_view();
    auto* arr_view = std::get<const JsonArrayCompact*>(view);
    auto elem = arr_view->get(KeyType(0));
    if (elem) {
        assert(elem.value().get_type() == HAKKA_JSON_INT);
    }
    ```

---

### set

```cpp
HakkaJsonResultEnum set(KeyType key, JsonHandleCompact value);
```

Sets the element at the specified index to a new value.

- **Parameters**:
    - `key`: A `KeyType` variant containing an `int64_t` index.
    - `value`: The new `JsonHandleCompact` value to set.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_TYPE_ERROR` if key is not `int64_t`.
    - `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is out of range.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `key` does not hold an `int64_t`.
    - Returns `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is negative or >= length.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));
    arr->set(KeyType(0), JsonIntCompact::create(100));

    auto elem = arr->at(0);
    if (elem) {
        auto view = elem.value().get_view();
        auto* int_val = std::get<const JsonIntCompact*>(view);
        assert(std::get<int64_t>(int_val->get().value()) == 100);
    }
    ```

---

### at

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> at(uint32_t index) const;
```

Retrieves an element at the specified index with bounds checking.

- **Parameters**:
    - `index`: The zero-based index of the element to retrieve.

- **Returns**:
    - On success: A `JsonHandleCompact` referencing the element at the index.
    - On failure: `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is out of range.

- **Error Handling**:
    - Returns `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index >= length.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));

    auto elem = arr->at(0);
    if (elem) {
        assert(elem.value().get_type() == HAKKA_JSON_INT);
    }
    ```

---

### remove

```cpp
HakkaJsonResultEnum remove(KeyType key);
```

Removes the element at the specified index, shifting subsequent elements left.

- **Parameters**:
    - `key`: A `KeyType` variant containing an `int64_t` index.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_TYPE_ERROR` if key is not `int64_t`.
    - `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is out of range.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `key` does not hold an `int64_t`.
    - Returns `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is negative or >= length.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the number of elements after the removed index (due to shifting).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(1));
    arr->push_back(JsonIntCompact::create(2));

    auto result = arr->remove(KeyType(0));
    assert(result == HAKKA_JSON_SUCCESS);
    assert(arr->length() == 1);
    ```

---

## Array Manipulation Methods

### push_back

```cpp
HakkaJsonResultEnum push_back(JsonHandleCompact value);
```

Appends an element to the end of the array.

- **Parameters**:
    - `value`: The `JsonHandleCompact` to append.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_INVALID_ARGUMENT` if the handle is invalid.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_INVALID_ARGUMENT` if `value.is_valid()` returns false.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(1) amortized time complexity (may trigger reallocation).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());

    auto result = arr->push_back(JsonIntCompact::create(42));
    assert(result == HAKKA_JSON_SUCCESS);
    assert(arr->length() == 1);
    ```

---

### pop

```cpp
HakkaJsonResultEnum pop(uint32_t index, JsonHandleCompact *pop_outed);
```

Removes and returns the element at the specified index.

- **Parameters**:
    - `index`: The zero-based index of the element to remove.
    - `pop_outed`: Pointer to store the removed element.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_INVALID_ARGUMENT` if `pop_outed` is null.
    - `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is out of range.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_INVALID_ARGUMENT` if `pop_outed` is null.
    - Returns `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index >= length.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the number of elements after the removed index (due to shifting).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));

    JsonHandleCompact popped;
    auto result = arr->pop(0, &popped);
    assert(result == HAKKA_JSON_SUCCESS);
    assert(popped.get_type() == HAKKA_JSON_INT);
    assert(arr->length() == 0);
    ```

---

### pop_back

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> pop_back() noexcept;
```

Removes and returns the last element of the array.

- **Parameters**:
    - None.

- **Returns**:
    - On success: A `JsonHandleCompact` containing the removed element.
    - On failure: `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if the array is empty, `HAKKA_JSON_INTERNAL_ERROR` on exception.

- **Error Handling**:
    - Returns `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if the array is empty.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (always removes last element with no shifting).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));

    auto popped = arr->pop_back();
    if (popped) {
        assert(popped.value().get_type() == HAKKA_JSON_INT);
        assert(arr->length() == 0);
    }
    ```

---

### insert

```cpp
HakkaJsonResultEnum insert(KeyType key, JsonHandleCompact value);
```

Inserts an element at the specified index, shifting subsequent elements right.

- **Parameters**:
    - `key`: A `KeyType` variant containing an `int64_t` index.
    - `value`: The `JsonHandleCompact` to insert.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_TYPE_ERROR` if key is not `int64_t`.
    - `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is out of range.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `key` does not hold an `int64_t`.
    - Returns `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is negative or > length.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the number of elements after the insertion index (due to shifting).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(1));
    arr->push_back(JsonIntCompact::create(3));

    auto result = arr->insert(KeyType(1), JsonIntCompact::create(2));
    assert(result == HAKKA_JSON_SUCCESS);
    assert(arr->length() == 3);
    ```

---

### erase

```cpp
HakkaJsonResultEnum erase(KeyType key);
```

Removes the element at the specified index. This is an alias for `remove()`.

- **Parameters**:
    - `key`: A `KeyType` variant containing an `int64_t` index.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_TYPE_ERROR` if key is not `int64_t`.
    - `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is out of range.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `key` does not hold an `int64_t`.
    - Returns `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is negative or >= length.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the number of elements after the removed index (due to shifting).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(1));
    arr->push_back(JsonIntCompact::create(2));

    auto result = arr->erase(KeyType(0));
    assert(result == HAKKA_JSON_SUCCESS);
    assert(arr->length() == 1);
    ```

---

### clear

```cpp
HakkaJsonResultEnum clear();
```

Removes all elements from the array and releases memory.

- **Parameters**:
    - None.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` always succeeds.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the number of elements (calls destructors and releases memory).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));

    auto result = arr->clear();
    assert(result == HAKKA_JSON_SUCCESS);
    assert(arr->length() == 0);
    ```

---

### shrink_to_fit

```cpp
void shrink_to_fit();
```

Reduces the capacity of the array to match its size, releasing unused memory.

- **Parameters**:
    - None.

- **Returns**:
    - None.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity (may trigger reallocation and element copying).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->reserve(100);
    arr->push_back(JsonIntCompact::create(42));
    arr->shrink_to_fit(); // Reduce capacity to match size
    ```

---

### extend

```cpp
HakkaJsonResultEnum extend(const JsonHandleCompact &other);
```

Appends all elements from another array to the end of this array.

- **Parameters**:
    - `other`: A `JsonHandleCompact` referencing another `JsonArrayCompact`.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_TYPE_ERROR` if `other` is not an array.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `other` is not `HAKKA_JSON_ARRAY`.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(m) time complexity where m is the length of the other array.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array1 = JsonArrayCompact::create();
    auto array2 = JsonArrayCompact::create();

    auto* arr1 = std::get<JsonArrayCompact*>(array1.get_mut_ptr());
    auto* arr2 = std::get<JsonArrayCompact*>(array2.get_mut_ptr());

    arr1->push_back(JsonIntCompact::create(1));
    arr2->push_back(JsonIntCompact::create(2));

    auto result = arr1->extend(array2);
    assert(result == HAKKA_JSON_SUCCESS);
    assert(arr1->length() == 2);
    ```

---

### count

```cpp
HakkaJsonResultEnum count(const JsonHandleCompact &value, uint32_t *out_count) const;
```

Counts the number of occurrences of a value in the array.

- **Parameters**:
    - `value`: The `JsonHandleCompact` to search for.
    - `out_count`: Pointer to store the count result.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_INVALID_ARGUMENT` if `out_count` is null.
    - Other errors if element comparison fails.

- **Error Handling**:
    - Returns `HAKKA_JSON_INVALID_ARGUMENT` if `out_count` is null.
    - Skips elements that cannot be compared due to type mismatch.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the number of elements (linear search with comparisons).

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));
    arr->push_back(JsonIntCompact::create(42));

    uint32_t count = 0;
    auto result = arr->count(JsonIntCompact::create(42), &count);
    assert(result == HAKKA_JSON_SUCCESS);
    assert(count == 2);
    ```

---

### index

```cpp
HakkaJsonResultEnum index(const JsonHandleCompact &value, uint32_t start, uint32_t stop, uint32_t *out_index) const;
```

Finds the first occurrence of a value within the specified range.

- **Parameters**:
    - `value`: The `JsonHandleCompact` to search for.
    - `start`: The starting index (inclusive).
    - `stop`: The ending index (exclusive).
    - `out_index`: Pointer to store the found index.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the value is found.
    - `HAKKA_JSON_INVALID_ARGUMENT` if `out_index` is null.
    - `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if `start` >= length.
    - `HAKKA_JSON_KEY_NOT_FOUND` if the value is not found.

- **Error Handling**:
    - Returns `HAKKA_JSON_INVALID_ARGUMENT` if `out_index` is null.
    - Returns `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if `start` >= length.
    - Returns `HAKKA_JSON_KEY_NOT_FOUND` if the value is not found in the range.
    - Skips elements that cannot be compared due to type mismatch.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the size of the search range (linear search with comparisons).

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(1));
    arr->push_back(JsonIntCompact::create(2));
    arr->push_back(JsonIntCompact::create(3));

    uint32_t index = 0;
    auto result = arr->index(JsonIntCompact::create(2), 0, 3, &index);
    assert(result == HAKKA_JSON_SUCCESS);
    assert(index == 1);
    ```

---

### remove_value

```cpp
HakkaJsonResultEnum remove_value(const JsonHandleCompact &value);
```

Removes the first occurrence of the specified value from the array.

- **Parameters**:
    - `value`: The `JsonHandleCompact` to remove.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the value is found and removed.
    - `HAKKA_JSON_KEY_NOT_FOUND` if the value is not found.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_KEY_NOT_FOUND` if no matching element is found.
    - Skips elements that cannot be compared due to type mismatch.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the number of elements (linear search plus shifting).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(1));
    arr->push_back(JsonIntCompact::create(2));

    auto result = arr->remove_value(JsonIntCompact::create(1));
    assert(result == HAKKA_JSON_SUCCESS);
    assert(arr->length() == 1);
    ```

---

## Array Operations

### multiply

```cpp
HakkaJsonResultEnum multiply(uint32_t times);
```

Repeats the array contents a specified number of times (similar to Python's `list * n`).

- **Parameters**:
    - `times`: The number of times to repeat the array contents.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - Other errors if memory allocation fails.

- **Error Handling**:
    - May return memory allocation errors if the result is too large.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n×times) time complexity where n is the original array length.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));

    auto result = arr->multiply(3);
    assert(result == HAKKA_JSON_SUCCESS);
    assert(arr->length() == 3); // [42, 42, 42]
    ```

---

### reverse

```cpp
HakkaJsonResultEnum reverse();
```

Reverses the order of elements in the array in-place.

- **Parameters**:
    - None.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the number of elements.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(1));
    arr->push_back(JsonIntCompact::create(2));
    arr->push_back(JsonIntCompact::create(3));

    auto result = arr->reverse();
    assert(result == HAKKA_JSON_SUCCESS);
    // Array is now [3, 2, 1]
    ```

---

### reserve

```cpp
HakkaJsonResultEnum reserve(size_t size) noexcept;
```

Reserves capacity for at least the specified number of elements.

- **Parameters**:
    - `size`: The minimum capacity to reserve.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_NOT_ENOUGH_MEMORY` if allocation fails.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` if allocation fails.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on other exceptions.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity if reallocation occurs (where n is the current size).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());

    auto result = arr->reserve(100);
    assert(result == HAKKA_JSON_SUCCESS);
    // Capacity is now at least 100
    ```

---

## Slicing Methods

### get_slice

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> get_slice(int start, int end, int step) const;
```

Creates a new array containing a slice of elements from this array.

- **Parameters**:
    - `start`: The starting index (supports negative indexing from end).
    - `end`: The ending index (exclusive, supports negative indexing).
    - `step`: The step size between elements.

- **Returns**:
    - On success: A `JsonHandleCompact` referencing a new array with the sliced elements.
    - On failure: `HAKKA_JSON_INVALID_ARGUMENT` if step is 0, `HAKKA_JSON_NOT_ENOUGH_MEMORY` if allocation fails, `HAKKA_JSON_INTERNAL_ERROR` on exception.

- **Error Handling**:
    - Returns `HAKKA_JSON_INVALID_ARGUMENT` if `step` is 0.
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` if allocation fails.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Supports negative indices (e.g., -1 for last element).
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n/step) time complexity where n is the size of the slice range.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    for (int i = 1; i <= 5; ++i) {
        arr->push_back(JsonIntCompact::create(i));
    }

    auto slice = arr->get_slice(1, 4, 1); // Elements [2, 3, 4]
    if (slice) {
        auto* slice_arr = std::get<const JsonArrayCompact*>(slice.value().get_view());
        assert(slice_arr->length() == 3);
    }
    ```

---

### set_slice

```cpp
HakkaJsonResultEnum set_slice(int start, int end, int step, JsonHandleCompact value);
```

Replaces a slice of elements with elements from another array.

- **Parameters**:
    - `start`: The starting index (supports negative indexing from end).
    - `end`: The ending index (exclusive, supports negative indexing).
    - `step`: The step size between elements.
    - `value`: A `JsonHandleCompact` referencing a `JsonArrayCompact` with replacement values.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_INVALID_ARGUMENT` if step is 0, value is not an array, or size mismatch for step > 1.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_INVALID_ARGUMENT` if `step` is 0 or `value` is not an array.
    - Returns `HAKKA_JSON_INVALID_ARGUMENT` if step > 1 and replacement size doesn't match slice size.
    - Returns `HAKKA_JSON_INVALID_ARGUMENT` if negative indices result in invalid range.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Supports negative indices (e.g., -1 for last element).
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity for step=1 (may resize array), O(n/step) for step > 1.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    for (int i = 1; i <= 5; ++i) {
        arr->push_back(JsonIntCompact::create(i));
    }

    auto replacement = JsonArrayCompact::create();
    auto* repl = std::get<JsonArrayCompact*>(replacement.get_mut_ptr());
    repl->push_back(JsonIntCompact::create(6));
    repl->push_back(JsonIntCompact::create(7));

    auto result = arr->set_slice(1, 3, 1, replacement);
    assert(result == HAKKA_JSON_SUCCESS);
    // Array is now [1, 6, 7, 4, 5]
    ```

---

## Iterator Methods

### begin

```cpp
JsonArrayIterCompact<IterDirection::FORWARD> begin();
```

Returns a forward iterator to the first element.

- **Parameters**:
    - None.

- **Returns**:
    - A forward iterator pointing to the first element.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(1));
    arr->push_back(JsonIntCompact::create(2));

    for (auto it = arr->begin(); it != arr->end(); ++it) {
        // Iterate over elements
    }
    ```

---

### end

```cpp
JsonArrayIterCompact<IterDirection::FORWARD> end() const;
```

Returns a forward iterator to one past the last element.

- **Parameters**:
    - None.

- **Returns**:
    - A forward iterator pointing to one past the last element.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(1));

    auto it = arr->begin();
    assert(it != arr->end());
    ```

---

### rbegin

```cpp
JsonArrayIterCompact<IterDirection::REVERSE> rbegin();
```

Returns a reverse iterator to the last element.

- **Parameters**:
    - None.

- **Returns**:
    - A reverse iterator pointing to the last element.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(1));
    arr->push_back(JsonIntCompact::create(2));

    for (auto it = arr->rbegin(); it != arr->rend(); ++it) {
        // Iterate in reverse order
    }
    ```

---

### rend

```cpp
JsonArrayIterCompact<IterDirection::REVERSE> rend() const;
```

Returns a reverse iterator to one before the first element.

- **Parameters**:
    - None.

- **Returns**:
    - A reverse iterator pointing to one before the first element.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(1));

    auto it = arr->rbegin();
    assert(it != arr->rend());
    ```

---

## Array Information Methods

### length

```cpp
std::size_t length() const;
```

Returns the number of elements in the array.

- **Parameters**:
    - None.

- **Returns**:
    - The number of elements in the array.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto array = JsonArrayCompact::create();
    auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
    arr->push_back(JsonIntCompact::create(42));

    assert(arr->length() == 1);
    ```

---

## Memory Management

Arrays in Hakka JSON are mutable containers that do not use value deduplication. Each array instance maintains its own storage:

```cpp
auto array1 = JsonArrayCompact::create();
auto array2 = JsonArrayCompact::create();

auto* arr1 = std::get<JsonArrayCompact*>(array1.get_mut_ptr());
auto* arr2 = std::get<JsonArrayCompact*>(array2.get_mut_ptr());

arr1->push_back(JsonIntCompact::create(42));
arr2->push_back(JsonIntCompact::create(42));

// array1 and array2 are distinct instances with separate storage
// Modifying array1 does not affect array2
```

**Reference Counting**: Arrays use atomic reference counting for memory management, designed for Python FFI integration.

**Automatic Cleanup**: The `JsonHandleCompact` system manages array lifecycle automatically. Arrays are deallocated when the last handle is released.

## Nested Arrays

Arrays can contain other arrays, allowing for nested structures:

```cpp
auto outer = JsonArrayCompact::create();
auto inner = JsonArrayCompact::create();

auto* inner_arr = std::get<JsonArrayCompact*>(inner.get_mut_ptr());
inner_arr->push_back(JsonIntCompact::create(1));
inner_arr->push_back(JsonIntCompact::create(2));

auto* outer_arr = std::get<JsonArrayCompact*>(outer.get_mut_ptr());
outer_arr->push_back(inner);

// outer is now [[1, 2]]
auto view = outer.get_view();
auto* outer_view = std::get<const JsonArrayCompact*>(view);
auto dump = outer_view->dump(512);
// dump.value() == "[[1, 2]]"
```

## Iterator Support

`JsonArrayCompact` provides random-access iterators for both forward and reverse traversal:

```cpp
auto array = JsonArrayCompact::create();
auto* arr = std::get<JsonArrayCompact*>(array.get_mut_ptr());
arr->push_back(JsonIntCompact::create(1));
arr->push_back(JsonIntCompact::create(2));
arr->push_back(JsonIntCompact::create(3));

// Forward iteration
for (auto it = arr->begin(); it != arr->end(); ++it) {
    auto elem_type = (*it).get_type();
    // Process element
}

// Reverse iteration
for (auto it = arr->rbegin(); it != arr->rend(); ++it) {
    auto elem_type = (*it).get_type();
    // Process element
}

// Random access
auto it = arr->begin();
it += 2; // Jump to third element
```

**Iterator Features**:
- **Random Access**: Supports `+=`, `-=`, `+`, `-` operations
- **Bounds Checking**: Automatically clamped to valid range
- **Bidirectional**: Forward and reverse iteration modes
- **STL Compatible**: Implements `std::random_access_iterator_tag`

## Performance Considerations

### Handle Access (Token → Array)

**O(1) constant-time access**: The handle system uses 32-bit tokens with 30-bit index pinning (bits 29-0). Array access via token is just array indexing:
```cpp
handles_[token & 0x3FFFFFFF]  // Extremely fast, ~2-3 CPU cycles
```

**Thread-safe**: Handle manager uses `std::recursive_mutex` for safe concurrent handle operations.

**Memory stable**: Indices never change once allocated - freed indices go to a min-heap freelist for reuse.

### Element Access (Index → Value)

**O(1) constant-time access**: Arrays use `std::vector<JsonHandleCompact>` for element storage. Random access by index is direct pointer arithmetic:
```cpp
elements_[index]  // Single memory access, cache-friendly
```

**Cache performance**: Sequential iteration is highly cache-efficient due to contiguous memory layout.

### Growth and Reallocation

**Amortized O(1) insertion**: `push_back()` uses vector's geometric growth (typically 1.5x or 2x capacity):
- Most insertions: O(1) - just increment size
- Occasional reallocation: O(n) - copy all elements to larger buffer
- **Amortized**: Total cost of n insertions is O(n), average O(1) per insertion

**Memory overhead**: Vector typically allocates 50-100% extra capacity to reduce reallocation frequency.

**Optimization**: Use `reserve(n)` when final size is known to avoid reallocations entirely.

### Memory Layout

**Per-array overhead**:
- 1 handle (elements vector): 4 bytes (token)
- Handle manager entry: ~8 bytes (pointer in vector)
- Vector metadata: 24 bytes (pointer, size, capacity)
- Reference count: 8 bytes (atomic<uint64_t>)
- **Total**: ~44 bytes + element storage

**Per-element cost**: 4 bytes per `JsonHandleCompact` element (32-bit token)

**Memory efficiency**: Extremely compact compared to alternatives:
- Python list: ~8 bytes per pointer + object overhead
- std::vector<std::variant>: 16-32 bytes per element (variant overhead)
- This design: 4 bytes per element + indirection through handle

### Insertion and Removal

**Insertion at end**: O(1) amortized via `push_back()`

**Insertion in middle**: O(n) due to shifting - `insert(index, value)` moves `n-index` elements right

**Removal from end**: Θ(1) constant time via `pop_back()` - no shifting, just decrement size

**Removal from middle**: O(n) due to shifting - `remove(index)` moves `n-index-1` elements left

**Recommendation**: For frequent insertions/deletions in middle, consider using a different container (e.g., linked list) or restructuring operations to work from the end.

### Slicing Operations

**get_slice(start, end, step)**:
- Time: O((end-start)/step) - creates new array and copies elements
- Space: O((end-start)/step) - allocates new array

**set_slice(start, end, step, values)**:
- Time: O(n) for step=1 (may resize), O(n/step) for step > 1
- Space: O(n) temporary buffer for step=1 operations

**Negative indexing**: Adds minimal overhead - just arithmetic adjustment before operation.

### Trade-offs

This design prioritizes **memory compactness** and **random access performance** over insertion/deletion in middle.

**Optimal use cases**:
- Random access by index is primary operation
- Append-heavy workloads (`push_back` dominated)
- Sequential iteration (cache-friendly)
- Memory-constrained environments (4 bytes per element)
- Numerical or homogeneous data (uniform handle representation)

**Consider alternatives if**:
- Frequent insertions/deletions in middle of array
- Need for O(1) insertion at arbitrary positions
- Array size changes dramatically and unpredictably

## See Also

- [JsonObjectCompact](Object.md) - JSON object type for key-value mappings
- [Handle System](Handle.md) - Handle-based memory management
- [JsonStructuredCompact](Structured.md) - Base class for structured types
- [Primitive Types Overview](Primitive.md) - Immutable primitive types with value deduplication
