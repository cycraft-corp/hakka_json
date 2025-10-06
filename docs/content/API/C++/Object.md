# JsonObjectCompact

`JsonObjectCompact` represents mutable JSON objects (key-value mappings) in the Hakka JSON library. Unlike primitive types which use value deduplication, objects are mutable containers that store key-value pairs with Python dict-like behavior.

**Header**: `include/hakka_json_object.hpp`

## Architecture

`JsonObjectCompact` is a mutable container type with the following characteristics:

- **Storage**: Two `JsonArrayCompact` instances (keys array and values array stored as handles)
- **Key Lookup**: O(n) linear search using the `find()` method
- **Mutability**: Objects can be modified after creation (unlike immutable primitive types)
- **No Deduplication**: Each object instance maintains its own storage
- **CRTP Pattern**: Inherits from `JsonStructuredCompact<JsonObjectCompact>` for compile-time polymorphism
- **Reference Counting**: Atomic reference counting for Python FFI integration
- **Handle Management**: Managed by `ObjectManagerCompact` with type mask `0xC0000000`
- **Dict-like Behavior**: Provides Python dict-like operations (pop, popitem, setdefault, update, fromkeys)

!!! note
    Objects use two parallel arrays for storage: one for keys (strings) and one for values. Key lookup is O(n) linear search, not hash-based.

## Factory Methods

### create

```cpp
[[nodiscard]] static JsonHandleCompact create();
```

Creates an empty JSON object.

- **Parameters**:
    - None.

- **Returns**:
    - A `JsonHandleCompact` referencing a new empty object.

- **Error Handling**:
    - Returns invalid handle if allocation fails.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(1) amortized time complexity (may trigger vector reallocation).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    assert(object.get_type() == HAKKA_JSON_OBJECT);
    ```

---

### create_unique

```cpp
[[nodiscard]] static std::unique_ptr<JsonObjectCompact> create_unique();
```

Creates an empty JSON object as a unique pointer for direct ownership.

- **Parameters**:
    - None.

- **Returns**:
    - A `std::unique_ptr<JsonObjectCompact>` owning a new empty object.

- **Error Handling**:
    - Returns `nullptr` if allocation fails.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object_ptr = JsonObjectCompact::create_unique();
    assert(object_ptr != nullptr);
    ```

---

### loads (std::string)

```cpp
static tl::expected<JsonHandleCompact, HakkaJsonResultEnum> loads(const std::string &json_str, uint32_t max_depth = 2048);
```

Deserializes a JSON string into a `JsonObjectCompact` instance.

- **Parameters**:
    - `json_str`: The JSON string to deserialize.
    - `max_depth`: Maximum recursion depth for nested structures (default: 2048).

- **Returns**:
    - On success: A `JsonHandleCompact` containing the deserialized object.
    - On failure: `HAKKA_JSON_TYPE_ERROR` if the JSON is not an object, `HAKKA_JSON_RECURSION_DEPTH_EXCEEDED` if max depth is exceeded, or other parsing errors.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if the root JSON value is not an object.
    - Returns `HAKKA_JSON_RECURSION_DEPTH_EXCEEDED` if nesting exceeds `max_depth`.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the length of the JSON string.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    std::string json_str = R"({"key": "value"})";
    auto object = JsonObjectCompact::loads(json_str);
    if (object) {
        auto view = object.value().get_view();
        auto* obj = std::get<const JsonObjectCompact*>(view);
        assert(obj->length() == 1);
    }
    ```

---

### loads (std::string_view)

```cpp
static tl::expected<JsonHandleCompact, HakkaJsonResultEnum> loads(std::string_view json_str, uint32_t max_depth = 2048);
```

Deserializes a JSON string view into a `JsonObjectCompact` instance.

- **Parameters**:
    - `json_str`: The JSON string view to deserialize.
    - `max_depth`: Maximum recursion depth for nested structures (default: 2048).

- **Returns**:
    - On success: A `JsonHandleCompact` containing the deserialized object.
    - On failure: `HAKKA_JSON_TYPE_ERROR` if the JSON is not an object, `HAKKA_JSON_RECURSION_DEPTH_EXCEEDED` if max depth is exceeded, or other parsing errors.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if the root JSON value is not an object.
    - Returns `HAKKA_JSON_RECURSION_DEPTH_EXCEEDED` if nesting exceeds `max_depth`.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the length of the JSON string.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    const char json_str[] = R"({"key1": 42})";
    auto object = JsonObjectCompact::loads(std::string_view(json_str, sizeof(json_str) - 1));
    if (object) {
        auto view = object.value().get_view();
        auto* obj = std::get<const JsonObjectCompact*>(view);
        assert(obj->length() == 1);
    }
    ```

---

## Serialization Methods

### dump

```cpp
tl::expected<std::string, HakkaJsonResultEnum> dump(uint32_t max_depth = 0) const;
```

Serializes the object to a JSON string.

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
    - O(n) time complexity where n is the total number of key-value pairs including nested structures.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    auto view = object.get_view();
    auto* obj_view = std::get<const JsonObjectCompact*>(view);
    auto result = obj_view->dump(512);
    if (result) {
        std::cout << result.value() << std::endl; // Output: {"key": 42}
    }
    ```

---

### to_bytes

```cpp
HakkaJsonResultEnum to_bytes(char *buffer, uint32_t *buffer_size) const;
```

Serializes the object into a null-terminated byte buffer.

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
    - O(n) time complexity where n is the total number of key-value pairs including nested structures.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    char buffer[512];
    uint32_t size = sizeof(buffer);
    auto view = object.get_view();
    auto* obj_view = std::get<const JsonObjectCompact*>(view);
    auto result = obj_view->to_bytes(buffer, &size);
    if (result == HAKKA_JSON_SUCCESS) {
        std::cout << buffer << std::endl; // Output: {"key": 42}
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
    - The number of bytes required to serialize the object (excluding null terminator).

- **Error Handling**:
    - Returns `0` on internal errors.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the total number of key-value pairs including nested structures.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    auto view = object.get_view();
    auto* obj_view = std::get<const JsonObjectCompact*>(view);
    uint64_t size = obj_view->dump_size();
    std::cout << "Size: " << size << std::endl; // Output: Size: 12 (for {"key": 42})
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
    - `HAKKA_JSON_OBJECT`

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    HakkaJsonType t = object.get_type();
    assert(t == HAKKA_JSON_OBJECT);
    ```

---

## Comparison Methods

### compare

```cpp
tl::expected<int, HakkaJsonResultEnum> compare(const JsonHandleCompact &other) const;
```

Compares this object with another JSON value. Objects are compared by iterating through keys in insertion order and comparing corresponding values.

- **Parameters**:
    - `other`: The `JsonHandleCompact` to compare against.

- **Returns**:
    - On success: An integer comparison result:
        - `< 0` if this object is less than `other`
        - `0` if this object equals `other`
        - `> 0` if this object is greater than `other`
    - On failure: `HAKKA_JSON_TYPE_ERROR` if `other` is not an object or if value types mismatch.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `other` is not `HAKKA_JSON_OBJECT`.
    - Returns `HAKKA_JSON_TYPE_ERROR` if corresponding values have different types.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n×m) time complexity where n is the number of key-value pairs to compare and m is the number of key-value pairs in the other object (due to O(m) linear search for each key lookup).

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto object1 = JsonObjectCompact::create();
    auto object2 = JsonObjectCompact::create();

    auto* obj1 = std::get<JsonObjectCompact*>(object1.get_mut_ptr());
    auto* obj2 = std::get<JsonObjectCompact*>(object2.get_mut_ptr());

    obj1->set("key", JsonIntCompact::create(42));
    obj2->set("key", JsonIntCompact::create(42));

    auto view1 = object1.get_view();
    auto* obj1_view = std::get<const JsonObjectCompact*>(view1);
    auto result = obj1_view->compare(object2);
    if (result) {
        assert(result.value() == 0); // Equal objects
    }
    ```

---

### hash

```cpp
uint64_t hash() const;
```

Computes the hash value of the object by XORing the hashes of keys and values arrays.

- **Parameters**:
    - None.

- **Returns**:
    - A 64-bit hash value.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - O(n) time complexity where n is the number of key-value pairs.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    auto view = object.get_view();
    auto* obj_view = std::get<const JsonObjectCompact*>(view);
    uint64_t h = obj_view->hash();
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
    auto object = JsonObjectCompact::create();
    auto view = object.get_view();
    auto* obj = std::get<const JsonObjectCompact*>(view);
    uint64_t count = obj->inc_ref();
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
    auto object = JsonObjectCompact::create();
    auto view = object.get_view();
    auto* obj = std::get<const JsonObjectCompact*>(view);
    uint64_t count = obj->dec_ref();
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

Retrieves a value by key or by index. Accepts both string keys and integer indices.

- **Parameters**:
    - `key`: A `KeyType` variant containing either a `std::string` key or an `int64_t` index.

- **Returns**:
    - On success: A `JsonHandleCompact` referencing the value (for string key) or a `[key, value]` array (for integer index).
    - On failure: `HAKKA_JSON_TYPE_ERROR` if key type is invalid, `HAKKA_JSON_KEY_NOT_FOUND` if string key doesn't exist, `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is out of range.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `key` is neither `std::string` nor `int64_t`.
    - Returns `HAKKA_JSON_KEY_NOT_FOUND` if the string key is not found.
    - Returns `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is out of range.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity for string key lookup (linear search).
    - O(1) amortized time complexity for integer index access (creates new array with two elements).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    // Get by string key
    auto result = obj->get("key");
    if (result) {
        assert(result.value().get_type() == HAKKA_JSON_INT);
    }

    // Get by index (returns [key, value] array)
    auto result2 = obj->get(KeyType(static_cast<int64_t>(0)));
    if (result2) {
        assert(result2.value().get_type() == HAKKA_JSON_ARRAY);
    }
    ```

---

### set

```cpp
HakkaJsonResultEnum set(KeyType key, JsonHandleCompact value) const;
```

Sets a value for the specified key. If the key exists, updates the value; otherwise, inserts a new key-value pair.

- **Parameters**:
    - `key`: A `KeyType` variant containing a `std::string` key.
    - `value`: The `JsonHandleCompact` value to set.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_TYPE_ERROR` if key is not a string.
    - `HAKKA_JSON_NOT_ENOUGH_MEMORY` if allocation fails.
    - `HAKKA_JSON_INTERNAL_ERROR` on exception.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `key` does not hold a `std::string`.
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` if key handle creation fails.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity for key lookup, O(1) amortized for insertion if key doesn't exist.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());

    auto result = obj->set("key", JsonIntCompact::create(42));
    assert(result == HAKKA_JSON_SUCCESS);

    // Update existing key
    auto result2 = obj->set("key", JsonIntCompact::create(100));
    assert(result2 == HAKKA_JSON_SUCCESS);
    ```

---

### at

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> at(uint32_t index) const;
```

Retrieves a key-value pair at the specified index as a `[key, value]` array.

- **Parameters**:
    - `index`: The zero-based index of the key-value pair.

- **Returns**:
    - On success: A `JsonHandleCompact` referencing a `JsonArrayCompact` containing `[key, value]`.
    - On failure: `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index is out of range, `HAKKA_JSON_NOT_ENOUGH_MEMORY` if allocation fails, `HAKKA_JSON_INTERNAL_ERROR` on other errors.

- **Error Handling**:
    - Returns `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if index >= length.
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` if array creation fails.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(1) amortized time complexity (array creation with two push_back operations).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    auto pair = obj->at(0);
    if (pair) {
        auto* arr = std::get<const JsonArrayCompact*>(pair.value().get_view());
        assert(arr->length() == 2); // [key, value]
    }
    ```

---

### remove

```cpp
HakkaJsonResultEnum remove(KeyType key) const;
```

Removes a key-value pair by key.

- **Parameters**:
    - `key`: A `KeyType` variant containing a `std::string` key.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_TYPE_ERROR` if key is not a string.
    - `HAKKA_JSON_KEY_NOT_FOUND` if the key doesn't exist.
    - `HAKKA_JSON_INTERNAL_ERROR` on exception.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `key` does not hold a `std::string`.
    - Returns `HAKKA_JSON_KEY_NOT_FOUND` if the key is not found.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity for key lookup and array element removal.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    auto result = obj->remove("key");
    assert(result == HAKKA_JSON_SUCCESS);
    assert(obj->length() == 0);
    ```

---

### contains

```cpp
bool contains(KeyType key) const;
```

Checks if a key exists in the object.

- **Parameters**:
    - `key`: A `KeyType` variant containing a `std::string` key.

- **Returns**:
    - `true` if the key exists, `false` otherwise.

- **Error Handling**:
    - Returns `false` if key is not a string or if an exception occurs.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity (linear search through keys).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    assert(obj->contains("key") == true);
    assert(obj->contains("missing") == false);
    ```

---

## Object Manipulation Methods

### insert

```cpp
HakkaJsonResultEnum insert(KeyType key, JsonHandleCompact value) const;
```

Inserts a new key-value pair. Unlike `set()`, this always appends without checking for existing keys.

- **Parameters**:
    - `key`: A `KeyType` variant containing a `std::string` key.
    - `value`: The `JsonHandleCompact` value to insert.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_TYPE_ERROR` if key is not a string.
    - `HAKKA_JSON_NOT_ENOUGH_MEMORY` if allocation fails.
    - `HAKKA_JSON_INTERNAL_ERROR` on exception.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `key` does not hold a `std::string`.
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` if key handle creation fails.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(1) amortized time complexity (appends to arrays).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());

    auto result = obj->insert("key", JsonIntCompact::create(42));
    assert(result == HAKKA_JSON_SUCCESS);
    ```

---

### erase

```cpp
HakkaJsonResultEnum erase(KeyType key) const;
```

Removes a key-value pair by key. This is an alias for `remove()`.

- **Parameters**:
    - `key`: A `KeyType` variant containing a `std::string` key.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_TYPE_ERROR` if key is not a string.
    - `HAKKA_JSON_KEY_NOT_FOUND` if the key doesn't exist.
    - `HAKKA_JSON_INTERNAL_ERROR` on exception.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `key` does not hold a `std::string`.
    - Returns `HAKKA_JSON_KEY_NOT_FOUND` if the key is not found.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity for key lookup and array element removal.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    auto result = obj->erase("key");
    assert(result == HAKKA_JSON_SUCCESS);
    ```

---

### clear

```cpp
HakkaJsonResultEnum clear() const;
```

Removes all key-value pairs from the object.

- **Parameters**:
    - None.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_INTERNAL_ERROR` on exception.

- **Error Handling**:
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the number of key-value pairs.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key1", JsonIntCompact::create(42));
    obj->set("key2", JsonIntCompact::create(100));

    auto result = obj->clear();
    assert(result == HAKKA_JSON_SUCCESS);
    assert(obj->length() == 0);
    ```

---

### shrink_to_fit

```cpp
void shrink_to_fit() const;
```

Reduces the capacity of the underlying arrays to match their sizes, releasing unused memory.

- **Parameters**:
    - None.

- **Returns**:
    - None.

- **Error Handling**:
    - Ignores errors (optimization only).
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity where n is the number of key-value pairs.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));
    obj->shrink_to_fit(); // Optimize memory usage
    ```

---

### pop

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> pop(const KeyType &key);
```

Removes and returns the value associated with the specified key.

- **Parameters**:
    - `key`: A `KeyType` variant containing a `std::string` key.

- **Returns**:
    - On success: A `JsonHandleCompact` containing the removed value.
    - On failure: `HAKKA_JSON_TYPE_ERROR` if key is not a string, `HAKKA_JSON_KEY_NOT_FOUND` if the key doesn't exist, `HAKKA_JSON_INTERNAL_ERROR` on exception.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `key` does not hold a `std::string`.
    - Returns `HAKKA_JSON_KEY_NOT_FOUND` if the key is not found.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity for key lookup and removal.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    auto result = obj->pop("key");
    if (result) {
        assert(result.value().get_type() == HAKKA_JSON_INT);
        assert(obj->length() == 0);
    }
    ```

---

### popitem

```cpp
tl::expected<std::pair<KeyType, JsonHandleCompact>, HakkaJsonResultEnum> popitem();
```

Removes and returns the last key-value pair in insertion order (similar to Python's dict.popitem()).

- **Parameters**:
    - None.

- **Returns**:
    - On success: A `std::pair<KeyType, JsonHandleCompact>` containing the removed key and value.
    - On failure: `HAKKA_JSON_KEY_NOT_FOUND` if the object is empty, `HAKKA_JSON_INTERNAL_ERROR` on exception.

- **Error Handling**:
    - Returns `HAKKA_JSON_KEY_NOT_FOUND` if the object is empty.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (always removes last element with no shifting required).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key1", JsonIntCompact::create(42));
    obj->set("key2", JsonIntCompact::create(100));

    auto result = obj->popitem();
    if (result) {
        auto [key, value] = result.value();
        assert(std::get<std::string>(key) == "key2");
        assert(obj->length() == 1);
    }
    ```

---

### setdefault

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> setdefault(const KeyType &key, JsonHandleCompact default_value = JsonHandleCompact());
```

Returns the value for the key if it exists; otherwise, inserts the key with the default value and returns it.

- **Parameters**:
    - `key`: A `KeyType` variant containing a `std::string` key.
    - `default_value`: The default value to set if the key doesn't exist (default: invalid handle).

- **Returns**:
    - On success: A `JsonHandleCompact` containing the existing or newly set value.
    - On failure: `HAKKA_JSON_TYPE_ERROR` if key is not a string, other errors if insertion fails.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if `key` does not hold a `std::string`.
    - May return insertion errors if the key doesn't exist and insertion fails.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n) time complexity for key lookup, O(1) amortized for insertion if key doesn't exist.

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());

    // Key doesn't exist, inserts default value
    auto result = obj->setdefault("key", JsonIntCompact::create(100));
    if (result) {
        auto view = result.value().get_view();
        auto* int_val = std::get<const JsonIntCompact*>(view);
        assert(std::get<int64_t>(int_val->get().value()) == 100);
    }

    // Key exists, returns existing value
    auto result2 = obj->setdefault("key", JsonIntCompact::create(200));
    if (result2) {
        auto view = result2.value().get_view();
        auto* int_val = std::get<const JsonIntCompact*>(view);
        assert(std::get<int64_t>(int_val->get().value()) == 100);
    }
    ```

---

### update

```cpp
HakkaJsonResultEnum update(const JsonObjectCompact &other);
```

Updates this object with key-value pairs from another object, overwriting existing keys.

- **Parameters**:
    - `other`: A `JsonObjectCompact` reference to merge from.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if the operation succeeds.
    - `HAKKA_JSON_INTERNAL_ERROR` on exception.

- **Error Handling**:
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Skips invalid keys or values.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n×m) time complexity where n is the number of keys in `other` and m is the number of keys in this object (due to O(m) linear search for each key).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    auto object1 = JsonObjectCompact::create();
    auto object2 = JsonObjectCompact::create();

    auto* obj1 = std::get<JsonObjectCompact*>(object1.get_mut_ptr());
    auto* obj2 = std::get<JsonObjectCompact*>(object2.get_mut_ptr());

    obj1->set("key1", JsonIntCompact::create(42));
    obj2->set("key2", JsonIntCompact::create(100));

    auto result = obj1->update(*obj2);
    assert(result == HAKKA_JSON_SUCCESS);
    assert(obj1->length() == 2);
    assert(obj1->contains("key1"));
    assert(obj1->contains("key2"));
    ```

---

### fromkeys

```cpp
static tl::expected<JsonHandleCompact, HakkaJsonResultEnum> fromkeys(const std::vector<KeyType> &keys, JsonHandleCompact value);
```

Creates a new object with the specified keys, all set to the same value.

- **Parameters**:
    - `keys`: A vector of `KeyType` containing string keys.
    - `value`: The `JsonHandleCompact` value to assign to all keys.

- **Returns**:
    - On success: A `JsonHandleCompact` containing the new object.
    - On failure: `HAKKA_JSON_NOT_ENOUGH_MEMORY` if allocation fails, `HAKKA_JSON_INTERNAL_ERROR` on exception.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` if object creation fails.
    - Skips non-string keys.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - O(n×m) time complexity where n is the number of keys and m is the average object size during insertion (due to O(m) linear search for each key).

- **Thread Safety**:
    - This function is NOT thread-safe. External synchronization is required for concurrent access.

- **Example**:
    ```cpp
    std::vector<KeyType> keys = {"key1", "key2", "key3"};
    auto default_value = JsonIntCompact::create(0);

    auto result = JsonObjectCompact::fromkeys(keys, default_value);
    if (result) {
        auto* obj = std::get<JsonObjectCompact*>(result.value().get_mut_ptr());
        assert(obj->length() == 3);
        assert(obj->contains("key1"));
        assert(obj->contains("key2"));
        assert(obj->contains("key3"));
    }
    ```

---

## Object Information Methods

### length

```cpp
std::size_t length() const;
```

Returns the number of key-value pairs in the object.

- **Parameters**:
    - None.

- **Returns**:
    - The number of key-value pairs, or `0` on error.

- **Error Handling**:
    - Returns `0` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    assert(obj->length() == 1);
    ```

---

### keys

```cpp
const JsonArrayCompact &keys() const;
```

Returns a reference to the internal keys array.

- **Parameters**:
    - None.

- **Returns**:
    - A const reference to the `JsonArrayCompact` containing all keys as strings.

- **Error Handling**:
    - None (assumes internal consistency).
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key1", JsonIntCompact::create(42));
    obj->set("key2", JsonIntCompact::create(100));

    const auto& keys = obj->keys();
    assert(keys.length() == 2);
    ```

---

### keys_handle

```cpp
JsonHandleCompact keys_handle() const;
```

Returns a handle to the internal keys array.

- **Parameters**:
    - None.

- **Returns**:
    - A `JsonHandleCompact` referencing the keys array.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    auto keys_handle = obj->keys_handle();
    assert(keys_handle.get_type() == HAKKA_JSON_ARRAY);
    ```

---

### values

```cpp
const JsonArrayCompact &values() const;
```

Returns a reference to the internal values array.

- **Parameters**:
    - None.

- **Returns**:
    - A const reference to the `JsonArrayCompact` containing all values.

- **Error Handling**:
    - None (assumes internal consistency).
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key1", JsonIntCompact::create(42));
    obj->set("key2", JsonIntCompact::create(100));

    const auto& values = obj->values();
    assert(values.length() == 2);
    ```

---

### values_handle

```cpp
JsonHandleCompact values_handle() const;
```

Returns a handle to the internal values array.

- **Parameters**:
    - None.

- **Returns**:
    - A `JsonHandleCompact` referencing the values array.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    auto values_handle = obj->values_handle();
    assert(values_handle.get_type() == HAKKA_JSON_ARRAY);
    ```

---

## Iterator Methods

### begin

```cpp
JsonObjectIterCompact begin() const;
```

Returns a bidirectional iterator to the first key-value pair.

- **Parameters**:
    - None.

- **Returns**:
    - A bidirectional iterator pointing to the first key-value pair.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key1", JsonIntCompact::create(42));
    obj->set("key2", JsonIntCompact::create(100));

    for (auto it = obj->begin(); it != obj->end(); ++it) {
        auto [key, value] = *it;
        // Process key-value pair
    }
    ```

---

### end

```cpp
JsonObjectIterCompact end() const;
```

Returns a bidirectional iterator to one past the last key-value pair.

- **Parameters**:
    - None.

- **Returns**:
    - A bidirectional iterator pointing to one past the last key-value pair.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe for concurrent reads. Concurrent modification requires external synchronization.

- **Example**:
    ```cpp
    auto object = JsonObjectCompact::create();
    auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
    obj->set("key", JsonIntCompact::create(42));

    auto it = obj->begin();
    assert(it != obj->end());
    ```

---

## Storage Architecture

Objects in Hakka JSON use a two-array storage model:

```cpp
struct ObjectType {
    JsonHandleCompact keys;   // Handle to JsonArrayCompact of strings
    JsonHandleCompact values; // Handle to JsonArrayCompact of values
};
```

**Key Characteristics**:
- **Parallel Arrays**: Keys and values are stored in separate parallel arrays at the same indices
- **Insertion Order**: Key-value pairs maintain insertion order
- **Linear Search**: Key lookup is O(n) using linear search through the keys array
- **No Hash Table**: Unlike many JSON libraries, this implementation uses arrays instead of hash tables

This design trades lookup performance for memory efficiency and simplicity.

## Memory Management

Objects are mutable containers that do not use value deduplication:

```cpp
auto object1 = JsonObjectCompact::create();
auto object2 = JsonObjectCompact::create();

auto* obj1 = std::get<JsonObjectCompact*>(object1.get_mut_ptr());
auto* obj2 = std::get<JsonObjectCompact*>(object2.get_mut_ptr());

obj1->set("key", JsonIntCompact::create(42));
obj2->set("key", JsonIntCompact::create(42));

// object1 and object2 are distinct instances with separate storage
// Modifying object1 does not affect object2
```

**Reference Counting**: Objects use atomic reference counting for memory management, designed for Python FFI integration.

**Automatic Cleanup**: The `JsonHandleCompact` system manages object lifecycle automatically. Objects are deallocated when the last handle is released.

## Python Dict-like Behavior

`JsonObjectCompact` provides operations similar to Python dictionaries:

```cpp
auto object = JsonObjectCompact::create();
auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());

// dict[key] = value  =>  obj->set(key, value)
obj->set("key", JsonIntCompact::create(42));

// value = dict[key]  =>  auto value = obj->get(key)
auto value = obj->get("key");

// key in dict  =>  obj->contains(key)
bool exists = obj->contains("key");

// value = dict.pop(key)  =>  auto value = obj->pop(key)
auto popped = obj->pop("key");

// key, value = dict.popitem()  =>  auto [key, value] = obj->popitem()
auto item = obj->popitem();

// value = dict.setdefault(key, default)  =>  auto value = obj->setdefault(key, default)
auto result = obj->setdefault("new_key", JsonIntCompact::create(100));

// dict1.update(dict2)  =>  obj1->update(*obj2)
obj->update(*other_obj);

// dict.fromkeys(keys, value)  =>  JsonObjectCompact::fromkeys(keys, value)
auto new_obj = JsonObjectCompact::fromkeys({"k1", "k2"}, default_value);
```

## Nested Objects

Objects can contain other objects and arrays, allowing for complex nested structures:

```cpp
auto outer = JsonObjectCompact::create();
auto inner = JsonObjectCompact::create();

auto* inner_obj = std::get<JsonObjectCompact*>(inner.get_mut_ptr());
inner_obj->set("nested_key", JsonIntCompact::create(42));

auto* outer_obj = std::get<JsonObjectCompact*>(outer.get_mut_ptr());
outer_obj->set("inner", inner);

// outer is now {"inner": {"nested_key": 42}}
auto view = outer.get_view();
auto* outer_view = std::get<const JsonObjectCompact*>(view);
auto dump = outer_view->dump(512);
// dump.value() == "{\"inner\": {\"nested_key\": 42}}"
```

## Iterator Support

`JsonObjectCompact` provides bidirectional iterators for traversing key-value pairs:

```cpp
auto object = JsonObjectCompact::create();
auto* obj = std::get<JsonObjectCompact*>(object.get_mut_ptr());
obj->set("key1", JsonIntCompact::create(1));
obj->set("key2", JsonIntCompact::create(2));
obj->set("key3", JsonIntCompact::create(3));

// Forward iteration
for (auto it = obj->begin(); it != obj->end(); ++it) {
    auto pair = *it;
    std::string key = std::get<std::string>(pair->first);
    JsonHandleCompact value = pair->second;
    // Process key-value pair
}

// Backward iteration
auto it = obj->end();
--it; // Move to last element
while (it != obj->begin()) {
    auto pair = *it;
    // Process key-value pair
    --it;
}
```

**Iterator Features**:
- **Bidirectional**: Supports `++` and `--` operations
- **Insertion Order**: Iterates in insertion order
- **Value Type**: Returns `std::pair<KeyType, JsonHandleCompact>`
- **STL Compatible**: Implements `std::bidirectional_iterator_tag`

## Performance Considerations

### Handle Access (Token → Object)

**O(1) constant-time access**: The handle system uses 32-bit tokens with 30-bit index pinning (bits 29-0). Object access via token is just array indexing:
```cpp
handles_[token & 0x3FFFFFFF]  // Extremely fast, ~2-3 CPU cycles
```

**Thread-safe**: Handle manager uses `std::recursive_mutex` for safe concurrent handle operations.

**Memory stable**: Indices never change once allocated - freed indices go to a min-heap freelist for reuse.

### Key Lookup (String → Value)

**O(n) linear search** through keys array using `find()` method. Each lookup iterates through all keys comparing strings.

**Performance characteristics**:
- **n < 10 keys**: Linear search is faster than hash table (cache-friendly, no hash computation)
- **10 ≤ n < 50 keys**: Comparable performance, linear search may still win due to cache locality
- **n ≥ 50 keys**: Hash table becomes faster (but this library prioritizes memory efficiency)

**Optimization strategies**:
1. **Key ordering**: Place frequently-accessed keys at beginning of array
2. **Batch operations**: Use `update()` or `fromkeys()` to reduce multiple lookups
3. **External caching**: For repeated lookups, cache key indices externally

### Insertion Performance

**New keys**: O(n) for `set()` (calls `find()` first), O(1) amortized for `insert()` (no search, direct append)

**Existing keys**: O(n) for lookup + O(1) for value update

**Recommendation**: Use `insert()` when you know the key doesn't exist to skip the O(n) search.

### Memory Overhead

**Per-object overhead**:
- 2 handles (keys array + values array): 8 bytes
- Handle manager entry: ~8 bytes (pointer in vector)
- Reference count: 8 bytes (atomic<uint64_t>)
- **Total**: ~24 bytes + key-value storage

**Comparison with hash tables**:
- Hash table: ~32-40 bytes per key (load factor 0.75) + collision overhead
- This design: ~16 bytes per key (two handles) + no collision overhead
- **~40-50% more memory-efficient** for small objects (< 100 keys)

### Trade-offs

This design prioritizes **memory efficiency** and **insertion order preservation** over lookup performance.

**Optimal use cases**:
- Small to medium-sized objects (< 50 keys for best performance)
- Infrequent lookups or iterator-based access
- Memory-constrained environments (embedded systems, mobile)
- Scenarios requiring insertion order (JSON serialization, Python dict compatibility)
- Many short-lived objects (reduced allocation overhead)

**Consider alternatives if**:
- Frequent key lookups with > 50 keys
- Random access dominates over sequential access
- Memory is not a constraint

## See Also

- [JsonArrayCompact](Array.md) - JSON array type for sequences
- [Handle System](Handle.md) - Handle-based memory management
- [JsonStructuredCompact](Structured.md) - Base class for structured types
- [Primitive Types Overview](Primitive.md) - Immutable primitive types with value deduplication
