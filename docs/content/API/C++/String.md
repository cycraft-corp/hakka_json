# JsonStringCompact

`JsonStringCompact` represents immutable UTF-8 encoded string values in the Hakka JSON library. It inherits from `JsonPrimitiveCompact<JsonStringCompact, scc::PicoStringProxy>` and uses the CRTP pattern for compile-time polymorphism.

**Header**: `include/hakka_json_string.hpp`

## Type Definitions

```cpp
using ValueType = scc::PicoStringProxy;
```

## Storage

`JsonStringCompact` uses `PicoString` for memory-efficient string storage with size-based optimization:

- **0 bytes**: Empty string (special case)
- **1 byte**: `PicoString1`
- **2 bytes**: `PicoString2`
- **4 bytes**: `PicoString4`
- **8 bytes**: `PicoString8`
- **16 bytes**: `PicoString16`
- **32 bytes**: `PicoString32`
- **64 bytes**: `PicoString64`
- **>64 bytes**: `PicoStringUnlimited`

## Factory Methods

### create (string_view)

```cpp
[[nodiscard]] static JsonHandleCompact create(std::string_view value);
```

Creates a `JsonStringCompact` instance from a string view. Automatically deduplicates identical values.

- **Parameters**:
    - `value`: The string view (`std::string_view`).

- **Returns**:
    - A `JsonHandleCompact` that references the created `JsonStringCompact` instance.

- **Error Handling**:
    - Throws `std::bad_alloc` if memory allocation fails.
    - Exception safety: Basic guarantee.

- **Complexity**:
    - $O(1)$ average case time complexity (hash table lookup/insert with deduplication).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello");
    assert(handle.get_type() == HAKKA_JSON_STRING);
    ```

---

### create (std::string)

```cpp
[[nodiscard]] static JsonHandleCompact create(const std::string &value);
```

Creates a `JsonStringCompact` instance from a std::string.

- **Parameters**:
    - `value`: The string (`std::string`).

- **Returns**:
    - A `JsonHandleCompact` that references the created `JsonStringCompact` instance.

- **Error Handling**:
    - Throws `std::bad_alloc` if memory allocation fails.
    - Exception safety: Basic guarantee.

- **Complexity**:
    - $O(1)$ average case time complexity (hash table lookup/insert with deduplication).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    std::string str = "world";
    auto handle = JsonStringCompact::create(str);
    ```

---

### create (C-string)

```cpp
template <size_t N>
[[nodiscard]] static JsonHandleCompact create(const char (&value)[N]);
```

Creates a `JsonStringCompact` instance from a C-string literal.

- **Parameters**:
    - `value`: The C-string literal.

- **Returns**:
    - A `JsonHandleCompact` that references the created `JsonStringCompact` instance.

- **Error Handling**:
    - Throws `std::bad_alloc` if memory allocation fails.
    - Exception safety: Basic guarantee.

- **Complexity**:
    - $O(1)$ average case time complexity (hash table lookup/insert with deduplication).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("literal");
    ```

---

### create_unique (string_view)

```cpp
[[nodiscard]] static std::unique_ptr<JsonStringCompact> create_unique(std::string_view value);
```

Creates a unique pointer to a `JsonStringCompact` instance. Does not deduplicate values.

- **Parameters**:
    - `value`: The string view (`std::string_view`).

- **Returns**:
    - A `std::unique_ptr<JsonStringCompact>` owning the created instance.
    - Returns `nullptr` if memory allocation fails.

- **Error Handling**:
    - Returns `nullptr` on allocation failure (uses `new (std::nothrow)`).
    - No exceptions thrown.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is string length (copies string data).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto strPtr = JsonStringCompact::create_unique("unique");
    if (strPtr) {
        // Use strPtr
    }
    ```

---

## Serialization Methods

### dump

```cpp
tl::expected<std::string, HakkaJsonResultEnum> dump(uint32_t max_depth = 0) const;
```

Serializes the string to JSON format with proper escaping.

- **Parameters**:
    - `max_depth`: Unused for `JsonStringCompact`; included for interface consistency.

- **Returns**:
    - On success: A `std::string` containing the JSON-escaped string (with surrounding quotes).
    - On failure: `HAKKA_JSON_INTERNAL_ERROR` (rare, occurs on exception).

- **Error Handling**:
    - Returns `HAKKA_JSON_INTERNAL_ERROR` if exception occurs during escaping.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length (scans and escapes characters).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello\nworld");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->dump();
        if (result) {
            std::cout << result.value() << std::endl;
            // Output: "hello\nworld"
        }
    }
    ```

---

### to_bytes

```cpp
HakkaJsonResultEnum to_bytes(char *buffer, uint32_t *buffer_size) const;
```

Serializes the string into a null-terminated byte buffer.

- **Parameters**:
    - `buffer`: Pointer to the destination byte buffer.
    - `buffer_size`: Pointer to the buffer size. On input, specifies the buffer capacity. On output, contains the number of bytes written (excluding null terminator) or the required size if buffer is too small.

- **Returns**:
    - `HAKKA_JSON_SUCCESS` if serialization succeeds.
    - `HAKKA_JSON_NOT_ENOUGH_MEMORY` if the buffer is too small. The required size is written to `*buffer_size`.
    - `HAKKA_JSON_INTERNAL_ERROR` if an exception occurs.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` when buffer is insufficient.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("test");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        char buffer[100];
        uint32_t size = sizeof(buffer);
        auto result = (*str_ptr)->to_bytes(buffer, &size);
        if (result == HAKKA_JSON_SUCCESS) {
            std::cout << buffer << std::endl; // Output: "test"
        }
    }
    ```

---

### dump_size

```cpp
uint64_t dump_size() const;
```

Calculates the size of the serialized JSON string.

- **Parameters**:
    - None.

- **Returns**:
    - The number of bytes required to serialize the string (including quotes and escape sequences).

- **Error Handling**:
    - None (assumes `dump()` succeeds).
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length (calls `dump()` internally).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("test");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        uint64_t size = (*str_ptr)->dump_size();
        std::cout << "Size: " << size << std::endl; // Output: Size: 6 (for "test")
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
    - `HAKKA_JSON_STRING`

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("test");
    HakkaJsonType t = handle.get_type();
    assert(t == HAKKA_JSON_STRING);
    ```

---

## Comparison Methods

### compare

```cpp
tl::expected<int, HakkaJsonResultEnum> compare(const JsonHandleCompact &other) const;
```

Compares this string with another JSON string lexicographically.

- **Parameters**:
    - `other`: The `JsonHandleCompact` to compare against.

- **Returns**:
    - On success: An integer comparison result:
        - `< 0` if this string is less than `other`
        - `0` if this string equals `other`
        - `> 0` if this string is greater than `other`
    - On failure: `HAKKA_JSON_TYPE_ERROR` if `other` is not `STRING`.

- **Error Handling**:
    - Returns `HAKKA_JSON_TYPE_ERROR` if type is incompatible.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(\min(n, m))$ time complexity, where $n$ and $m$ are the string lengths.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle1 = JsonStringCompact::create("apple");
    auto handle2 = JsonStringCompact::create("banana");

    auto view1 = handle1.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view1)) {
        auto result = (*str_ptr)->compare(handle2);
        if (result) {
            assert(result.value() < 0); // "apple" < "banana"
        }
    }
    ```

---

### hash

```cpp
uint64_t hash() const;
```

Computes the hash value of the string using `std::hash<std::string>`.

- **Parameters**:
    - None.

- **Returns**:
    - A 64-bit hash value.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        uint64_t h = (*str_ptr)->hash();
        std::cout << "Hash: " << h << std::endl;
    }
    ```

---

## Value Access Methods

### get

```cpp
tl::expected<PrimitiveType, HakkaJsonResultEnum> get() const;
```

Retrieves the string value stored in this instance.

- **Parameters**:
    - None.

- **Returns**:
    - On success: A `PrimitiveType` containing the `std::string` value.
    - On failure: Error (rare, should not occur for valid instances).

- **Error Handling**:
    - No errors expected for valid instances.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length (copies string).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto value = (*str_ptr)->get();
        if (value) {
            std::string str_val = std::get<std::string>(value.value());
            std::cout << "Value: " << str_val << std::endl;
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
    auto handle = JsonStringCompact::create("test");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        uint64_t count = (*str_ptr)->inc_ref();
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
    auto handle = JsonStringCompact::create("test");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        uint64_t count = (*str_ptr)->dec_ref();
        std::cout << "New ref count: " << count << std::endl;
        // Note: Manually calling dec_ref() is not recommended; use Handle system
    }
    ```

!!! warning
    Manual reference counting is intended for Python FFI integration. Normal usage should rely on the `JsonHandleCompact` system for automatic reference management.

---

## String Information Methods

### length

```cpp
tl::expected<int64_t, HakkaJsonResultEnum> length() const;
```

Returns the number of Unicode characters (not bytes) in the string.

- **Parameters**:
    - None.

- **Returns**:
    - On success: The number of Unicode characters (UTF-32 code points).
    - On failure: Error (rare).

- **Error Handling**:
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the byte length (converts to ICU UnicodeString).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("繁體中文");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto len = (*str_ptr)->length();
        if (len) {
            std::cout << "Length: " << len.value() << std::endl; // Output: 4
        }
    }
    ```

---

### utf8_length

```cpp
tl::expected<uint64_t, HakkaJsonResultEnum> utf8_length() const;
```

Returns the number of bytes in the UTF-8 encoded string.

- **Parameters**:
    - None.

- **Returns**:
    - The number of bytes in the string.

- **Error Handling**:
    - None.
    - Exception safety: No-throw guarantee.

- **Complexity**:
    - $\Theta(1)$ time complexity (returns stored size).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("繁體中文");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto bytes = (*str_ptr)->utf8_length();
        if (bytes) {
            std::cout << "Bytes: " << bytes.value() << std::endl; // Output: 12
        }
    }
    ```

---

## String Manipulation Methods

### capitalize

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> capitalize() const;
```

Returns a capitalized version of the string (first character uppercase, rest lowercase).

- **Parameters**:
    - None.

- **Returns**:
    - On success: A new `JsonHandleCompact` with the capitalized string.
    - On failure: Error (rare).

- **Error Handling**:
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello WORLD");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->capitalize();
        if (result) {
            auto result_view = result.value().get_view();
            if (auto* result_ptr = std::get_if<const JsonStringCompact*>(&result_view)) {
                auto str = (*result_ptr)->get();
                std::cout << std::get<std::string>(str.value()) << std::endl;
                // Output: "Hello world"
            }
        }
    }
    ```

---

### casefold

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> casefold() const;
```

Returns a casefolded version of the string (lowercase for case-insensitive comparison).

- **Parameters**:
    - None.

- **Returns**:
    - On success: A new `JsonHandleCompact` with the casefolded string.
    - On failure: Error (rare).

- **Error Handling**:
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("HELLO");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->casefold(); // Output: "hello"
    }
    ```

---

### lower

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> lower() const;
```

Returns a lowercase version of the string.

- **Parameters**:
    - None.

- **Returns**:
    - On success: A new `JsonHandleCompact` with the lowercase string.
    - On failure: `HAKKA_JSON_NOT_ENOUGH_MEMORY` or `HAKKA_JSON_INTERNAL_ERROR`.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` if memory allocation fails.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("HELLO");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->lower(); // Output: "hello"
    }
    ```

---

### upper

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> upper() const;
```

Returns an uppercase version of the string.

- **Parameters**:
    - None.

- **Returns**:
    - On success: A new `JsonHandleCompact` with the uppercase string.
    - On failure: `HAKKA_JSON_NOT_ENOUGH_MEMORY` or `HAKKA_JSON_INTERNAL_ERROR`.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` if memory allocation fails.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->upper(); // Output: "HELLO"
    }
    ```

---

### swapcase

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> swapcase() const;
```

Returns a string with case swapped (uppercase becomes lowercase and vice versa).

- **Parameters**:
    - None.

- **Returns**:
    - On success: A new `JsonHandleCompact` with the swapped-case string.
    - On failure: `HAKKA_JSON_NOT_ENOUGH_MEMORY` or `HAKKA_JSON_INTERNAL_ERROR`.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` if memory allocation fails.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("Hello");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->swapcase(); // Output: "hELLO"
    }
    ```

---

### title

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> title() const;
```

Returns a titlecased version of the string (first character of each word capitalized).

- **Parameters**:
    - None.

- **Returns**:
    - On success: A new `JsonHandleCompact` with the titlecased string.
    - On failure: `HAKKA_JSON_NOT_ENOUGH_MEMORY` or `HAKKA_JSON_INTERNAL_ERROR`.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` if memory allocation fails.
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on exception.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello world");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->title(); // Output: "Hello World"
    }
    ```

---

### count

```cpp
tl::expected<int64_t, HakkaJsonResultEnum> count(std::string_view substring) const;
```

Returns the number of non-overlapping occurrences of substring.

- **Parameters**:
    - `substring`: The substring to count.

- **Returns**:
    - The number of occurrences.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n \cdot m)$ time complexity, where $n$ is the string length and $m$ is the substring length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello hello world");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->count("hello");
        if (result) {
            std::cout << "Count: " << result.value() << std::endl; // Output: 2
        }
    }
    ```

---

### find

```cpp
tl::expected<int64_t, HakkaJsonResultEnum> find(std::string_view substring) const;
```

Returns the lowest index where substring is found, or -1 if not found.

- **Parameters**:
    - `substring`: The substring to find.

- **Returns**:
    - The index of the first occurrence, or `-1` if not found.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n \cdot m)$ time complexity, where $n$ is the string length and $m$ is the substring length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello world");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->find("world");
        if (result) {
            std::cout << "Index: " << result.value() << std::endl; // Output: 6
        }
    }
    ```

---

### rfind

```cpp
tl::expected<int64_t, HakkaJsonResultEnum> rfind(std::string_view substring) const;
```

Returns the highest index where substring is found, or -1 if not found.

- **Parameters**:
    - `substring`: The substring to find.

- **Returns**:
    - The index of the last occurrence, or `-1` if not found.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n \cdot m)$ time complexity, where $n$ is the string length and $m$ is the substring length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello hello world");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->rfind("hello");
        if (result) {
            std::cout << "Index: " << result.value() << std::endl; // Output: 6
        }
    }
    ```

---

### startswith

```cpp
tl::expected<bool, HakkaJsonResultEnum> startswith(std::string_view prefix) const;
```

Returns `true` if the string starts with the specified prefix.

- **Parameters**:
    - `prefix`: The prefix to check.

- **Returns**:
    - `true` if the string starts with the prefix, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(m)$ time complexity, where $m$ is the prefix length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello world");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->startswith("hello");
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### endswith

```cpp
tl::expected<bool, HakkaJsonResultEnum> endswith(std::string_view suffix) const;
```

Returns `true` if the string ends with the specified suffix.

- **Parameters**:
    - `suffix`: The suffix to check.

- **Returns**:
    - `true` if the string ends with the suffix, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(m)$ time complexity, where $m$ is the suffix length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello world");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->endswith("world");
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### removeprefix

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> removeprefix(std::string_view prefix) const;
```

Returns a new string with the prefix removed if it exists.

- **Parameters**:
    - `prefix`: The prefix to remove.

- **Returns**:
    - On success: A new `JsonHandleCompact` with the prefix removed (or original string if prefix not found).
    - On failure: Error (rare).

- **Error Handling**:
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello world");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->removeprefix("hello ");
        // Result: "world"
    }
    ```

---

### removesuffix

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> removesuffix(std::string_view suffix) const;
```

Returns a new string with the suffix removed if it exists.

- **Parameters**:
    - `suffix`: The suffix to remove.

- **Returns**:
    - On success: A new `JsonHandleCompact` with the suffix removed (or original string if suffix not found).
    - On failure: Error (rare).

- **Error Handling**:
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello world");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->removesuffix(" world");
        // Result: "hello"
    }
    ```

---

### replace

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> replace(std::string_view old_substr, std::string_view new_substr) const;
```

Returns a new string with all occurrences of old_substr replaced by new_substr.

- **Parameters**:
    - `old_substr`: The substring to replace.
    - `new_substr`: The replacement substring.

- **Returns**:
    - On success: A new `JsonHandleCompact` with replacements made.
    - On failure: Error (rare).

- **Error Handling**:
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n \cdot m)$ time complexity, where $n$ is the string length and $m$ is the old substring length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello world");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->replace("world", "universe");
        // Result: "hello universe"
    }
    ```

---

### concatenate

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> concatenate(std::string_view other) const;
```

Returns a new string that is the concatenation of this string and another.

- **Parameters**:
    - `other`: The string to concatenate.

- **Returns**:
    - On success: A new `JsonHandleCompact` with the concatenated string.
    - On failure: Error (rare).

- **Error Handling**:
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n + m)$ time complexity, where $n$ and $m$ are the string lengths.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->concatenate(" world");
        // Result: "hello world"
    }
    ```

---

### multiply

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> multiply(int64_t times) const;
```

Returns a new string that is this string repeated `times` times.

- **Parameters**:
    - `times`: The number of repetitions.

- **Returns**:
    - On success: A new `JsonHandleCompact` with the repeated string.
    - On failure: `HAKKA_JSON_NOT_ENOUGH_MEMORY` if memory allocation fails.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` if memory allocation fails.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n \cdot times)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("ha");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->multiply(3);
        // Result: "hahaha"
    }
    ```

---

### slice

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> slice(int64_t start, int64_t stop, int64_t step = 1) const;
```

Returns a substring using Python-style slicing.

- **Parameters**:
    - `start`: The starting index (can be negative).
    - `stop`: The ending index (can be negative).
    - `step`: The step size (default 1).

- **Returns**:
    - On success: A new `JsonHandleCompact` with the sliced substring.
    - On failure: `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if indices are invalid.

- **Error Handling**:
    - Returns `HAKKA_JSON_INDEX_OUT_OF_BOUNDS` if indices are out of range.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the resulting substring length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello world");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->slice(0, 5, 1);
        // Result: "hello"
    }
    ```

---

### split

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> split(std::string_view separator, int64_t maxsplit = -1) const;
```

Splits the string into an array of substrings using the separator.

- **Parameters**:
    - `separator`: The separator string (empty string splits into individual characters).
    - `maxsplit`: Maximum number of splits (default -1 means no limit).

- **Returns**:
    - On success: A `JsonHandleCompact` referencing a `JsonArrayCompact` containing the split substrings.
    - On failure: `HAKKA_JSON_INTERNAL_ERROR`.

- **Error Handling**:
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on failure.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("a,b,c");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->split(",", -1);
        // Result: ["a", "b", "c"]
    }
    ```

---

### rsplit

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> rsplit(std::string_view separator, int64_t maxsplit = -1) const;
```

Splits the string from the right using the separator.

- **Parameters**:
    - `separator`: The separator string (empty string splits into individual characters).
    - `maxsplit`: Maximum number of splits (default -1 means no limit).

- **Returns**:
    - On success: A `JsonHandleCompact` referencing a `JsonArrayCompact` containing the split substrings.
    - On failure: `HAKKA_JSON_INTERNAL_ERROR`.

- **Error Handling**:
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on failure.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("a,b,c");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->rsplit(",", 1);
        // Result: ["a,b", "c"]
    }
    ```

---

### splitlines

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> splitlines(bool keepends = false) const;
```

Splits the string at line boundaries.

- **Parameters**:
    - `keepends`: If `true`, line breaks are included in the resulting substrings (default `false`).

- **Returns**:
    - On success: A `JsonHandleCompact` referencing a `JsonArrayCompact` containing the lines.
    - On failure: `HAKKA_JSON_INTERNAL_ERROR`.

- **Error Handling**:
    - Returns `HAKKA_JSON_INTERNAL_ERROR` on failure.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello\nworld\n");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->splitlines(false);
        // Result: ["hello", "world"]
    }
    ```

---

### zfill

```cpp
tl::expected<JsonHandleCompact, HakkaJsonResultEnum> zfill(int64_t width) const;
```

Pads the string on the left with zeros to fill the specified width.

- **Parameters**:
    - `width`: The target width.

- **Returns**:
    - On success: A new `JsonHandleCompact` with the zero-padded string.
    - On failure: `HAKKA_JSON_NOT_ENOUGH_MEMORY` if memory allocation fails.

- **Error Handling**:
    - Returns `HAKKA_JSON_NOT_ENOUGH_MEMORY` if memory allocation fails.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(width)$ time complexity.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("42");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->zfill(5);
        // Result: "00042"
    }
    ```

---

## String Testing Methods

### isalnum

```cpp
tl::expected<bool, HakkaJsonResultEnum> isalnum() const;
```

Returns `true` if all characters are alphanumeric and string is non-empty.

- **Parameters**:
    - None.

- **Returns**:
    - `true` if all characters are alphanumeric, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("abc123");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->isalnum();
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### isalpha

```cpp
tl::expected<bool, HakkaJsonResultEnum> isalpha() const;
```

Returns `true` if all characters are alphabetic and string is non-empty.

- **Parameters**:
    - None.

- **Returns**:
    - `true` if all characters are alphabetic, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("abc");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->isalpha();
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### isascii

```cpp
tl::expected<bool, HakkaJsonResultEnum> isascii() const;
```

Returns `true` if all characters are ASCII (code points < 128).

- **Parameters**:
    - None.

- **Returns**:
    - `true` if all characters are ASCII, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->isascii();
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### isdecimal

```cpp
tl::expected<bool, HakkaJsonResultEnum> isdecimal() const;
```

Returns `true` if all characters are decimal digits and string is non-empty.

- **Parameters**:
    - None.

- **Returns**:
    - `true` if all characters are decimal digits, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("12345");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->isdecimal();
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### isdigit

```cpp
tl::expected<bool, HakkaJsonResultEnum> isdigit() const;
```

Returns `true` if all characters are digits and string is non-empty.

- **Parameters**:
    - None.

- **Returns**:
    - `true` if all characters are digits, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("12345");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->isdigit();
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### isidentifier

```cpp
tl::expected<bool, HakkaJsonResultEnum> isidentifier() const;
```

Returns `true` if the string is a valid identifier.

- **Parameters**:
    - None.

- **Returns**:
    - `true` if the string is a valid identifier, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("variable_name");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->isidentifier();
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### islower

```cpp
tl::expected<bool, HakkaJsonResultEnum> islower() const;
```

Returns `true` if all cased characters are lowercase and string is non-empty.

- **Parameters**:
    - None.

- **Returns**:
    - `true` if all cased characters are lowercase, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->islower();
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### isnumeric

```cpp
tl::expected<bool, HakkaJsonResultEnum> isnumeric() const;
```

Returns `true` if all characters are numeric and string is non-empty.

- **Parameters**:
    - None.

- **Returns**:
    - `true` if all characters are numeric, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("12345");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->isnumeric();
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### isprintable

```cpp
tl::expected<bool, HakkaJsonResultEnum> isprintable() const;
```

Returns `true` if all characters are printable or string is empty.

- **Parameters**:
    - None.

- **Returns**:
    - `true` if all characters are printable, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello world");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->isprintable();
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### isspace

```cpp
tl::expected<bool, HakkaJsonResultEnum> isspace() const;
```

Returns `true` if all characters are whitespace and string is non-empty.

- **Parameters**:
    - None.

- **Returns**:
    - `true` if all characters are whitespace, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("   ");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->isspace();
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### istitle

```cpp
tl::expected<bool, HakkaJsonResultEnum> istitle() const;
```

Returns `true` if the string is titlecased.

- **Parameters**:
    - None.

- **Returns**:
    - `true` if the string is titlecased, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("Hello World");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->istitle();
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

### isupper

```cpp
tl::expected<bool, HakkaJsonResultEnum> isupper() const;
```

Returns `true` if all cased characters are uppercase and string is non-empty.

- **Parameters**:
    - None.

- **Returns**:
    - `true` if all cased characters are uppercase, `false` otherwise.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity, where $n$ is the string length.

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("HELLO");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto result = (*str_ptr)->isupper();
        if (result) {
            assert(result.value() == true);
        }
    }
    ```

---

## Iterator Methods

### begin

```cpp
JsonStringCompactIter begin() const;
```

Returns an iterator to the beginning of the string.

- **Parameters**:
    - None.

- **Returns**:
    - A `JsonStringCompactIter` pointing to the first Unicode character.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity (converts to ICU UnicodeString).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("hello");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        for (auto it = (*str_ptr)->begin(); it != (*str_ptr)->end(); ++it) {
            char32_t c = *it;
            // Process each Unicode character
        }
    }
    ```

---

### end

```cpp
JsonStringCompactIter end() const;
```

Returns an iterator to the end of the string.

- **Parameters**:
    - None.

- **Returns**:
    - A `JsonStringCompactIter` pointing past the last Unicode character.

- **Error Handling**:
    - None.
    - Exception safety: Strong guarantee.

- **Complexity**:
    - $O(n)$ time complexity (converts to ICU UnicodeString).

- **Thread Safety**:
    - This function is thread-safe.

- **Example**:
    ```cpp
    auto handle = JsonStringCompact::create("world");
    auto view = handle.get_view();
    if (auto* str_ptr = std::get_if<const JsonStringCompact*>(&view)) {
        auto it = (*str_ptr)->end();
        // Used for loop termination
    }
    ```

---

## Memory Management

`JsonStringCompact` uses automatic value deduplication. When multiple handles are created with the same string value, they reference the same underlying object:

```cpp
auto handle1 = JsonStringCompact::create("hello");
auto handle2 = JsonStringCompact::create("hello");
// Both handles reference the same JsonStringCompact instance
```

The instance is destroyed when its reference count reaches zero.

## See Also

- [Primitive Types Overview](Primitive.md)
- [Handle System](Handle.md)
- [JsonIntCompact](Int.md)
- [JsonFloatCompact](Float.md)
