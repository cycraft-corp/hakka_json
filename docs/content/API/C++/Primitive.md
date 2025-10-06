# Primitive Types

## Overview

All primitive types in Hakka JSON are **immutable**. Once created, their values cannot change. To use a different value, create a new instance.

Primitive types are the leaf nodes in JSON documents. They include:

- **[JsonInt](Int.md)**: 64-bit signed integer values
- **[JsonFloat](Float.md)**: 64-bit floating-point values (IEEE 754 double-precision)
- **[JsonString](String.md)**: UTF-8 encoded string values
- **[JsonBool](Bool.md)**: Boolean values (true/false)
- **[JsonNull](Null.md)**: Null values
- **[JsonInvalid](Invalid.md)**: Invalid/uninitialized values

## Type System Architecture

### CRTP-Based Implementation

All primitive types inherit from `JsonPrimitiveCompact<Derived, ValueType>`, which uses the Curiously Recurring Template Pattern (CRTP). This eliminates virtual function overhead and enables compile-time polymorphism.

### NaN-Boxing Technique

`JsonBool`, `JsonNull`, and `JsonInvalid` are implemented using NaN-boxing within `JsonFloatCompact`. This technique stores different value types in a single 64-bit IEEE 754 double by using distinct NaN (Not-a-Number) bit patterns:

```cpp
// NaN value encoding (high bits of mantissa)
NULL_NAN    = get_nan(0);  // Represents null
TRUE_NAN    = get_nan(1);  // Represents true
FALSE_NAN   = get_nan(2);  // Represents false
INVALID_NAN = get_nan(3);  // Represents invalid/uninitialized
```

!!! warning
    Do not directly use these internal NaN values for comparison. Use the Handle system's `type()` and `compare()` methods instead.

**Type Discrimination**: The `type()` method examines the bit pattern to determine which type the value represents (Float, Bool, Null, or Invalid).

**Benefits**:

- All NaN-boxed values stored in same manager (FloatManager)
- Singleton instances for true, false, null, invalid (never deallocated)
- No additional memory overhead for type tags

## Memory Management

### Value Deduplication

Primitive types automatically share storage when identical values are created:

```cpp
auto handle1 = JsonIntCompact::create(42);
auto handle2 = JsonIntCompact::create(42);
// Both handles reference the same underlying object
```

**Implementation**: Each manager maintains a hash table (`hash_to_index_map_`) that maps values to their storage indices. Before allocating, the manager checks if an identical value exists.

**Deduplication Scope**:

- `JsonInt`: All integer values deduplicated
- `JsonFloat`: All float values deduplicated
- `JsonString`: All string values deduplicated
- `JsonBool`: Single instance each for true/false (NaN-boxed singletons)
- `JsonNull`: Single instance (NaN-boxed singleton)
- `JsonInvalid`: Single instance (NaN-boxed singleton)

### Reference Counting

All primitive types use atomic reference counting for lifetime management:

```cpp
std::atomic<uint64_t> ref_count;

uint64_t inc_ref() const {
    return ref_count.fetch_add(1, std::memory_order_relaxed) + 1;
}

uint64_t dec_ref() const {
    return ref_count.fetch_sub(1, std::memory_order_relaxed) - 1;
}
```

**Memory Ordering**: Uses `memory_order_relaxed` because reference counting operations do not require inter-thread synchronization. The manager's mutex provides synchronization for object access.

**Lifetime**: Objects are destroyed when their reference count reaches zero.

## Handle System Integration

Primitive types are accessed through `JsonHandleCompact`, a 32-bit handle that encodes both type information and object index. See [Handle System](Handle.md) for details.

**Token Encoding for Scalars**:
```
Bits 31-30: 00 (Scalar type)
Bit 29:     0 = Float/Bool/Null/Invalid
            1 = Integer
Bits 28-0:  Index into manager's handles vector
```

## Thread Safety

All primitive type operations are thread-safe:

- **Reference counting**: Uses atomic operations
- **Object creation**: Manager mutex protects hash table and freelist
- **Object access**: Manager mutex protects pointer retrieval
- **Deduplication**: Thread-safe lookup and insertion in hash table

**Mutable access**: Not applicable—primitive types are immutable.

## Type-Specific Documentation

For detailed API documentation, see individual type pages:

- [JsonInt](Int.md) - Integer values and operations
- [JsonFloat](Float.md) - Floating-point values and operations
- [JsonString](String.md) - String values and operations
- [JsonBool](Bool.md) - Boolean values (NaN-boxed)
- [JsonNull](Null.md) - Null values (NaN-boxed)
- [JsonInvalid](Invalid.md) - Invalid/uninitialized values (NaN-boxed)
