# Common Types and Definitions

## Overview

The `common.h` header provides foundational type definitions, macros, and compatibility layers for the Hakka JSON C API. It establishes a consistent interface for both C and C++ consumers while maintaining zero-overhead abstractions and cross-language interoperability.

**Key Features**:

- **C/C++ Compatibility**: Automatic detection and appropriate header inclusion
- **Type Safety**: Strong typing for handles and iterators via `uint64_t` base type
- **FFI-Ready**: 64-bit handle encoding suitable for foreign function interfaces
- **Namespace Isolation**: Prefixed types (`Hakka*`) prevent naming collisions
- **Linkage Control**: C linkage macros for seamless C++ integration

## Architecture

### Handle System

All Hakka JSON objects are accessed through opaque 64-bit handles. This design provides:

- **Memory Safety**: Handles are tokens, not raw pointers/preventing direct memory access
- **ABI Stability**: Handle representation remains constant across library versions
- **Cross-Language Support**: Fixed-width integers (64-bit) ensure consistent layout in Python, Rust, Go FFIs
- **Type Discrimination**: Four distinct handle types for different object categories

```c
// Handle type hierarchy
HakkaHandle       // Base handle: all JSON values (int, float, string, array, object, etc.)
HakkaStringIter   // String iteration handle
HakkaArrayIter    // Array iteration handle
HakkaObjectIter   // Object iteration handle
```

**Design Rationale**: While the internal C++ implementation uses 32-bit handles for memory efficiency, the C API exposes 64-bit handles to:

1. Align with native pointer sizes on 64-bit architectures
2. Reserve upper bits for future extensions (versioning, flags, generation counters)
3. Simplify FFI integration (most bindings expect pointer-sized integers)

### C/C++ Compatibility Layer

The header uses conditional compilation to provide appropriate types:

**C++ Mode** (`__cplusplus` defined):
```cpp
#include <cstddef>    // size_t, nullptr
#include <cstdint>    // uint64_t, int64_t, etc.

using HakkaHandle = uint64_t;
using HakkaStringIter = uint64_t;
// ... type aliases
```

**C Mode** (C99/C11):
```c
#include <stdbool.h>  // bool, true, false
#include <stddef.h>   // size_t, NULL
#include <stdint.h>   // uint64_t, int64_t, etc.

typedef uint64_t HakkaHandle;
typedef uint64_t HakkaStringIter;
// ... typedefs
```

This dual approach ensures:

- **Zero overhead**: Type aliases in C++, typedefs in C
- **Standard compliance**: Uses standard library headers (no custom implementations)
- **Consistency**: Identical binary layout regardless of compilation mode

## Type Definitions

### HakkaHandle

```c
typedef uint64_t HakkaHandle;
```

**Purpose**: Universal handle type for all JSON values (primitives and containers).

**Usage**:

- Return type for creation functions: `hakka_int_create()`, `hakka_array_create()`, etc.
- Parameter type for all value manipulation functions
- Storage type for array elements and object values

**Encoding** (internal implementation detail):

- Lower 32 bits: Index into type-specific manager
- Upper 32 bits: Reserved (currently unused, available for future extensions)

**Invalid Handle**: The value `0` represents an invalid/uninitialized handle. All valid handles are non-zero.

### HakkaStringIter

```c
typedef uint64_t HakkaStringIter;
```

**Purpose**: Opaque iterator for UTF-8 string traversal.

**Usage**:

- Iterate over grapheme clusters (user-perceived characters)
- Support forward and reverse iteration
- Enable substring extraction and codepoint access

**Lifecycle**:

1. Created via `hakka_string_iter_create(HakkaHandle string)`
2. Advanced via `hakka_string_iter_next()`, `hakka_string_iter_prev()`
3. Destroyed via `hakka_string_iter_destroy(HakkaStringIter iter)`

### HakkaArrayIter

```c
typedef uint64_t HakkaArrayIter;
```

**Purpose**: Opaque iterator for array traversal.

**Usage**:
- Forward and reverse iteration over array elements
- Range-based iteration support
- Random access via index-based positioning

**Lifecycle**:
1. Created via `hakka_array_iter_create(HakkaHandle array)`
2. Advanced via `hakka_array_iter_next()`, `hakka_array_iter_prev()`
3. Destroyed via `hakka_array_iter_destroy(HakkaArrayIter iter)`

### HakkaObjectIter

```c
typedef uint64_t HakkaObjectIter;
```

**Purpose**: Opaque iterator for object key-value pair traversal.

**Usage**:

- Iterate over object entries (key-value pairs)
- Retrieve current key and value via iterator
- Support insertion-order traversal

**Lifecycle**:

1. Created via `hakka_object_iter_create(HakkaHandle object)`
2. Advanced via `hakka_object_iter_next()`, `hakka_object_iter_prev()`
3. Destroyed via `hakka_object_iter_destroy(HakkaObjectIter iter)`

## Macros

### extern_c

```c
#define extern_c extern "C"
```

**Purpose**: Declares C linkage for C++ compilers.

**Usage**:
```cpp
extern_c HakkaHandle hakka_int_create(int64_t value);
```

**Effect**:
- Prevents C++ name mangling for exported functions
- Enables C code to link against C++ compiled libraries
- No-op in C mode (C compilers ignore `extern "C"`)

### C_BOOL

```c
#define C_BOOL uint8_t
```

**Purpose**: Portable boolean type for C interfaces.

**Rationale**:

- C99 `_Bool` has implementation-defined size (usually 1 byte, but not guaranteed)
- C++ `bool` is guaranteed 1 byte, but may differ from C `_Bool` in ABI
- `uint8_t` ensures consistent 8-bit representation across all platforms

**Usage**:
```c
C_BOOL hakka_object_contains(HakkaHandle object, const char* key);
// Returns: 1 (true) if key exists, 0 (false) otherwise
```

**Convention**:

- `0` = false
- Non-zero (typically `1`) = true

## Dependencies

### hakka_json_enum.h

Provides enumeration types for:

- **HakkaType**: JSON type discrimination (`HAKKA_TYPE_INT`, `HAKKA_TYPE_ARRAY`, etc.)
- **HakkaError**: Error codes for exception-less error reporting
- **HakkaCompareResult**: Comparison results for sorting and ordering

**Inclusion Order**:
```c
#include <hakka_json_enum.h>  // Enums first
#include "common.h"            // Common types second
#include "int.h"               // Specific type APIs third
```

## Thread Safety

All type definitions in `common.h` are **thread-safe** as they are:

1. **Compile-time constants**: Macros and typedefs have no runtime state
2. **Immutable**: Type aliases cannot be modified after definition
3. **Header-only**: No shared global state introduced

**Note**: Thread safety of handle operations depends on the underlying C++ implementation's manager synchronization (documented per function).

## Portability

### Platform Support

- **Operating Systems**: Linux, macOS, Windows
- **Architectures**: x86-64, ARM64, RISC-V (any platform with C99/C++17 support)
- **Compilers**: GCC 7+, Clang 10+, MSVC 2019+

### ABI Considerations

- **Handle Size**: Always 64 bits (8 bytes) on all platforms
- **Alignment**: Natural alignment (8-byte boundaries)
- **Endianness**: Handle encoding is endian-agnostic (opaque tokens, not bit-fields)

## Example Usage

### C API Consumer

```c
#include <hakka_json/common.h>
#include <hakka_json/int.h>
#include <hakka_json/array.h>

void example(void) {
    // Create integer handle
    HakkaHandle int_val = hakka_int_create(42);

    // Create array handle
    HakkaHandle arr = hakka_array_create();
    hakka_array_push_back(arr, int_val);

    // Iterate using array iterator
    HakkaArrayIter iter = hakka_array_iter_create(arr);
    while (!hakka_array_iter_at_end(iter)) {
        HakkaHandle elem = hakka_array_iter_get(iter);
        // Process element...
        hakka_array_iter_next(iter);
    }
    hakka_array_iter_destroy(iter);

    // Cleanup
    hakka_handle_destroy(arr);
    hakka_handle_destroy(int_val);
}
```

### C++ API Consumer (using C API)

```cpp
extern "C" {
    #include <hakka_json/common.h>
    #include <hakka_json/int.h>
}

void example_cpp() {
    HakkaHandle value = hakka_int_create(100);

    // C++ can use C API via extern "C" linkage
    int64_t num = hakka_int_get_value(value);

    hakka_handle_destroy(value);
}
```

## Design Decisions

### Why 64-bit Handles?

**Question**: The C++ implementation uses 32-bit handles internally. Why does the C API expose 64-bit handles?

**Answer**:

1. **FFI Alignment**: Most scripting languages (Python, Ruby, Lua) expect pointer-sized integers (64-bit on modern systems)
2. **Future-Proofing**: Upper 32 bits reserved for:
    - Generation counters (detect use-after-free)
    - Version tags (ABI evolution)
    - Handle flags (metadata without extra lookups)

3. **Performance**: 64-bit handles avoid truncation/extension overhead on 64-bit architectures
4. **Simplicity**: One handle size for all use cases (no small vs large handle variants)

**Trade-off**: 2x memory overhead for handle storage is acceptable given:

- Handles are transient (stack/register allocated in most code)
- Actual JSON data dominates memory usage (strings, arrays, objects)
- Memory safety and clarity outweigh 4-byte savings per handle

### Why Separate Iterator Types?

**Question**: Why not use `HakkaHandle` for iterators?

**Answer**:

1. **Type Safety**: Prevents accidentally passing array iterators to string functions
2. **API Clarity**: Function signatures self-document iterator requirements
3. **Evolution**: Iterator implementations can diverge from handle encoding independently
4. **Compiler Diagnostics**: C++ compilers catch type mismatches at compile-time

## See Also

<!-- - [Enum Types](../CAPI/enums.md) - Error codes and type enumerations -->
<!-- - [Integer API](../CAPI/int.md) - Integer value manipulation -->
- [Array API](../CAPI/array.md) - Array container operations
- [Object API](../CAPI/object.md) - Object container operations
<!-- - [Memory Management](../CAPI/memory.md) - Handle lifecycle and ownership -->
