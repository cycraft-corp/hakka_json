import type { GraphData, NvlNode, NvlRelationship, ImplementationId, NodeType, RelationshipType } from '../types/graph';

/**
 * Context passed to generators during graph construction
 */
export interface GeneratorContext {
  /** Tracks string interning across the generation */
  stringTracker: StringTracker;
  /** Tracks all immutable values for duplicate detection */
  valueTracker: ValueTracker;
  /** Generates unique IDs for nodes and edges */
  idGenerator: IdGenerator;
  /** Parent node ID for relationship creation */
  parentId?: string;
  /** JSON path for debugging/labeling */
  path: string;
}

/**
 * String tracking for interning simulation
 */
export interface StringTracker {
  /** Track a string value, returns node ID and whether it's new */
  track(value: string): { nodeId: string; isNew: boolean };
  /** Get all tracked strings */
  getStrings(): Map<string, string>;
  /** Get interning statistics */
  getStats(): StringStats;
  /** Reset tracker state */
  reset(): void;
}

export interface StringStats {
  totalStrings: number;
  uniqueStrings: number;
  duplicateStrings: number;
  bytesSaved: number;
}

/**
 * Value tracking for duplicate detection of immutables
 * Tracks strings, numbers, booleans, and null
 */
export interface ValueTracker {
  /** Check if a value has been seen before (is duplicate) */
  isDuplicate(value: unknown): boolean;
  /** Mark a value as seen */
  markSeen(value: unknown): void;
  /** Reset tracker state */
  reset(): void;
}

/**
 * ID generator interface
 */
export interface IdGenerator {
  /** Generate a unique node ID */
  nodeId(prefix?: string): string;
  /** Generate a unique edge ID */
  edgeId(): string;
  /** Reset the generator */
  reset(): void;
}

/**
 * Result of generating a single JSON value
 */
export interface GenerationResult {
  /** The primary node representing this value */
  rootNodeId: string;
  /** All nodes created for this value (including children) */
  nodes: NvlNode[];
  /** All relationships created */
  relationships: NvlRelationship[];
}

/**
 * Graph generator interface - each implementation provides one
 */
export interface GraphGenerator {
  /** Implementation identifier */
  readonly id: ImplementationId;
  /** Human-readable name */
  readonly name: string;
  /** Generate a complete graph from parsed JSON */
  generate(json: unknown): GraphData;
}

/**
 * Memory size constants for different node types (in bytes)
 * These are approximations for visualization purposes
 */
export const SIZE_ESTIMATES = {
  // HakkaJson - sizes based on actual C++ implementation
  // Registry: singleton with array of 4 manager pointers
  'hakka-registry': 32,  // 4 × 8-byte pointers
  // Managers: mutex (~40 bytes) + vector (24) + freelist vector (24) + hash_map (~56) + vtable (8)
  'hakka-scalar-manager': 200,
  'hakka-string-manager': 200,
  'hakka-array-manager': 200,
  'hakka-object-manager': 200,
  // Handle: 4-byte HandleManagerToken (top 2 bits = type, bottom 30 bits = index)
  'hakka-handle': 4,
  // NaN-boxed: 8-byte double with special NaN patterns for bool/null/invalid
  'hakka-nan-boxed': 8,
  // PicoString: tiered fixed-size storage with size encoded in pointer's lower 3 bits
  'hakka-interned-string': (len: number) => {
    if (len <= 1) return 1;   // PicoString1
    if (len <= 2) return 2;   // PicoString2
    if (len <= 4) return 4;   // PicoString4
    if (len <= 8) return 8;   // PicoString8
    if (len <= 16) return 16; // PicoString16
    if (len <= 32) return 32; // PicoString32
    if (len <= 64) return 64; // PicoString64
    return 24 + len;          // Fallback to std::string for len > 64
  },
  // JsonArrayCompact: vector<JsonHandleCompact> + refcount (atomic<uint64_t>)
  // vector header (24 bytes) + refcount (8 bytes) + elements (4 bytes each handle)
  'hakka-array': (count: number) => 32 + count * 4,
  // JsonObjectCompact: 2 handles (keys array, values array) + refcount
  // Much smaller than expected! Keys/values are separate array allocations
  'hakka-object': 24,  // 2 × 4-byte handles + 8-byte refcount + vtable (8)

  // ==========================================================================
  // serde_json (Rust) - Sizes from Rust std lib and serde_json source
  // Reference: library/alloc/src/string.rs, library/alloc/src/vec/mod.rs
  // ==========================================================================
  //
  // Memory Overhead = Everything that's not the actual JSON data
  // - Discriminants, pointers, lengths, capacities are ALL overhead
  // - Only actual string bytes and numeric values are DATA
  //

  // Container sizes (for backwards compatibility)
  'serde-value-enum': 32,                              // Value enum total size
  'serde-string': (len: number) => 24 + len,           // String total (stack + heap)
  'serde-number': 16,                                  // Number total
  'serde-bool': 1,
  'serde-null': 0,
  'serde-array': (count: number) => 24 + count * 32,   // Vec<Value> (32B per Value)
  'serde-indexmap': (count: number) => 48 + count * 32,
  'serde-btreemap': 16,                                // BTreeMap stack size (root + len)

  // Value enum fields (32 bytes total = discriminant + payload + padding)
  'serde-discriminant': 8,               // enum discriminant (aligned to 8B)
  'serde-padding': 8,                    // alignment padding

  // String fields (24 bytes stack)
  'serde-string-ptr': 8,                 // ptr to heap
  'serde-string-len': 8,                 // length
  'serde-string-cap': 8,                 // capacity
  'serde-string-data': (len: number) => len,  // heap bytes (actual DATA)

  // Number fields (16 bytes)
  'serde-n-discriminant': 8,             // N enum discriminant
  'serde-n-value': 8,                    // u64/i64/f64 value (actual DATA)

  // Vec fields (24 bytes stack)
  'serde-vec-ptr': 8,                    // ptr to heap
  'serde-vec-len': 8,                    // length
  'serde-vec-cap': 8,                    // capacity
  'serde-vec-slot': 32,                  // each slot holds a Value (32B)

  // BTreeMap fields (16 bytes stack + ~628 bytes per LeafNode!)
  // Reference: rust/library/alloc/src/collections/btree/node.rs
  // B = 6, CAPACITY = 11, LeafNode allocates 11 slots regardless of usage!
  'serde-btree-root': 8,                 // root node pointer
  'serde-btree-len': 8,                  // entry count
  'serde-btree-node': 0,                 // container for children
  'serde-btree-leaf': 0,                 // container for entries

  // LeafNode<String, Value> internals (~628 bytes total!)
  'serde-leafnode-parent': 8,            // Option<NonNull<InternalNode>>
  'serde-leafnode-parent-idx': 2,        // MaybeUninit<u16>
  'serde-leafnode-len': 2,               // u16 entry count
  'serde-leafnode-keys': 0,              // container for key slots
  'serde-leafnode-vals': 0,              // container for val slots
  'serde-leafnode-key-slot': 24,         // String slot (ptr + len + cap)
  'serde-leafnode-val-slot': 32,         // Value slot (32 bytes)
  'serde-leafnode-wasted-key': 24,       // WASTED key slot (overhead!)
  'serde-leafnode-wasted-val': 32,       // WASTED val slot (overhead!)

  // ==========================================================================
  // CPython 3.11/3.12 - Sizes from actual source code
  // Reference: Include/cpython/object.h, Include/object.h
  // ==========================================================================
  //
  // Memory Overhead = Structural overhead + Wasted memory (duplicates)
  // - Structural: ob_refcnt (8B) + ob_type (8B) per object + type objects
  // - Wasted: duplicate immutable values (json module doesn't intern)
  //
  // Data-only sizes below EXCLUDE the 16-byte PyObject header
  // (header is added as separate overhead nodes for visualization)
  //
  // Infrastructure nodes - ACCURATE sizes from CPython 3.11 source
  'py-type-pool': 0,              // Conceptual grouping (type objects are global statics)
  'py-type-object': 424,          // PyTypeObject base struct (53 fields × 8B)
  'py-number-methods': 280,       // PyNumberMethods (35 func ptrs × 8B)
  'py-sequence-methods': 80,      // PySequenceMethods (10 func ptrs × 8B)
  'py-mapping-methods': 24,       // PyMappingMethods (3 func ptrs × 8B)
  // Aggregate value nodes (for backwards compatibility)
  'py-dict': 32,            // PyDictObject fields only (ma_used + ma_version + ma_keys + ma_values)
  'py-list': 24,            // PyListObject fields only (ob_size + ob_item + allocated)
  'py-unicode': (len: number) => 28 + len,  // PyASCIIObject fields (length + hash + state + wstr + data)
  'py-long': 8,             // PyLongObject fields only (ob_size + digits)
  'py-float': 8,            // PyFloatObject fields only (ob_fval)
  'py-bool': 8,             // PyBoolObject fields only (same as py-long)
  'py-none': 0,             // Py_None has only header, no additional fields
  'py-type-ptr': 8,         // ob_type pointer (overhead)
  'py-refcount': 8,         // ob_refcnt (overhead)

  // PyDictObject internal fields (32 bytes total)
  'py-dict-ma-used': 8,           // ma_used: Py_ssize_t
  'py-dict-ma-version': 8,        // ma_version_tag: uint64_t
  'py-dict-ma-keys-ptr': 8,       // ma_keys: pointer to PyDictKeysObject
  'py-dict-ma-values-ptr': 8,     // ma_values: pointer (NULL for combined dict)

  // PyDictKeysObject fields (~40 bytes header + indices + entries)
  'py-dict-keys-refcnt': 8,       // dk_refcnt: Py_ssize_t
  'py-dict-keys-log2': 3,         // dk_log2_size(1) + dk_log2_index_bytes(1) + dk_kind(1)
  'py-dict-keys-version': 4,      // dk_version: uint32_t
  'py-dict-keys-usable': 8,       // dk_usable: Py_ssize_t
  'py-dict-keys-nentries': 8,     // dk_nentries: Py_ssize_t
  'py-dict-keys-indices': (size: number) => size,  // dk_indices[]: variable size
  'py-dict-keys-entries': 0,      // Container for entries (size in children)

  // PyDictKeyEntry fields (24 bytes per entry)
  'py-dict-entry-hash': 8,        // me_hash: Py_hash_t
  'py-dict-entry-key-ptr': 8,     // me_key: PyObject*
  'py-dict-entry-value-ptr': 8,   // me_value: PyObject*

  // PyListObject internal fields (24 bytes excluding header)
  'py-list-ob-size': 8,           // ob_size: Py_ssize_t (item count)
  'py-list-ob-item-ptr': 8,       // ob_item: PyObject**
  'py-list-allocated': 8,         // allocated: Py_ssize_t
  'py-list-items-array': (count: number) => count * 8,  // Array of pointers
  'py-list-item-slot': 8,         // Individual PyObject* slot

  // PyLongObject internal fields
  'py-long-ob-size': 8,           // ob_size: Py_ssize_t (digit count + sign)
  'py-long-digit': 4,             // ob_digit[i]: uint32_t (30-bit digit)

  // PyFloatObject internal fields
  'py-float-fval': 8,             // ob_fval: double (8 bytes)

  // PyUnicodeObject (PyASCIIObject) internal fields
  'py-unicode-length': 8,         // length: Py_ssize_t
  'py-unicode-hash': 8,           // hash: Py_hash_t (-1 if not computed)
  'py-unicode-state': 4,          // state: bitfield (interned, kind, compact, ascii, ready)
  'py-unicode-wstr': 8,           // wstr: wchar_t* (deprecated, usually NULL)
  'py-unicode-data': (len: number) => len + 1,  // inline data + null terminator

  // ==========================================================================
  // Go encoding/json - Sizes from Go runtime source
  // Reference: runtime/runtime2.go, runtime/string.go, runtime/map.go
  // ==========================================================================
  //
  // Memory Overhead = Everything that's not the actual JSON data
  // - interface{} wrappers, pointers, lengths = ALL OVERHEAD
  // - Only actual string bytes and numeric values are DATA
  //

  // interface{} (eface) - 16 bytes wrapper for EVERY dynamic value
  // Reference: runtime/runtime2.go
  'go-interface': 16,                              // eface total size
  'go-iface-type': 8,                              // _type pointer (OVERHEAD)
  'go-iface-data': 8,                              // data pointer (OVERHEAD)

  // string (stringStruct) - 16 bytes header + heap data
  // Reference: runtime/string.go
  'go-string': (len: number) => 16 + len,          // header + data
  'go-string-ptr': 8,                              // str pointer (OVERHEAD)
  'go-string-len': 8,                              // len field (OVERHEAD - metadata!)
  'go-string-data': (len: number) => len,          // heap bytes (DATA)

  // Scalar types
  'go-float64': 8,                                 // float64 (DATA)
  'go-bool': 1,                                    // bool (DATA)

  // []interface{} slice - 24 bytes header + backing array
  // Reference: runtime/slice.go
  'go-slice': (count: number) => 24 + count * 16,  // header + elements
  'go-slice-ptr': 8,                               // array pointer (OVERHEAD)
  'go-slice-len': 8,                               // length (OVERHEAD - metadata!)
  'go-slice-cap': 8,                               // capacity (OVERHEAD)
  'go-slice-slot': 16,                             // each interface{} slot (OVERHEAD)
  'go-slice-wasted-slot': 16,                      // WASTED slot (OVERHEAD - unused)

  // hmap structure - 48 bytes
  // Reference: runtime/map.go
  'go-hmap': 48,                                   // hmap total size
  'go-hmap-count': 8,                              // count: int (OVERHEAD - length!)
  'go-hmap-flags': 1,                              // flags: uint8 (OVERHEAD)
  'go-hmap-b': 1,                                  // B: uint8 log2 buckets (OVERHEAD)
  'go-hmap-noverflow': 2,                          // noverflow: uint16 (OVERHEAD)
  'go-hmap-hash0': 4,                              // hash0: uint32 seed (OVERHEAD)
  'go-hmap-buckets': 8,                            // buckets: pointer (OVERHEAD)
  'go-hmap-oldbuckets': 8,                         // oldbuckets: pointer (OVERHEAD)
  'go-hmap-nevacuate': 8,                          // nevacuate: uintptr (OVERHEAD)
  'go-hmap-extra': 8,                              // extra: *mapextra (OVERHEAD)

  // bucket (bmap) - 272 bytes for map[string]interface{}
  // Reference: runtime/map.go, bucketCnt = 8
  // tophash[8] + keys[8]×16 + vals[8]×16 + overflow = 8 + 128 + 128 + 8 = 272
  'go-bucket': (_count: number) => 272,            // FIXED 272 bytes regardless of usage!
  'go-bucket-tophash': 8,                          // tophash[8]: 8 × uint8 (OVERHEAD)
  'go-bucket-tophash-slot': 1,                     // individual tophash (OVERHEAD)
  'go-bucket-tophash-wasted': 1,                   // WASTED tophash slot (OVERHEAD)
  'go-bucket-keys': 128,                           // keys region: 8 × 16B string (OVERHEAD container)
  'go-bucket-vals': 128,                           // vals region: 8 × 16B interface{} (OVERHEAD container)
  'go-bucket-overflow': 8,                         // overflow pointer (OVERHEAD)
  'go-bucket-key-slot': 16,                        // individual key slot (string header)
  'go-bucket-val-slot': 16,                        // individual val slot (interface{})
  'go-bucket-wasted-key': 16,                      // WASTED key slot (OVERHEAD - unused!)
  'go-bucket-wasted-val': 16,                      // WASTED val slot (OVERHEAD - unused!)

  // ==========================================================================
  // Jansson (C) - Sizes from actual source code
  // Reference: jansson.h, jansson_private.h, hashtable.h, hashtable.c
  // ==========================================================================
  //
  // Memory Overhead = Everything that's not the actual JSON data
  // - json_t headers (type + refcount), pointers, lengths, hash values = ALL OVERHEAD
  // - Only actual string bytes and numeric values are DATA
  //

  // json_t base structure (16 bytes with padding)
  // Reference: jansson.h
  'jansson-json-t': 16,                              // json_t total size
  'jansson-type-enum': 4,                            // json_type enum (OVERHEAD)
  'jansson-refcount': 8,                             // volatile size_t refcount (OVERHEAD)
  'jansson-padding': 4,                              // alignment padding (OVERHEAD)

  // json_object_t = json_t (16) + hashtable_t (56) = 72 bytes
  // Reference: jansson_private.h
  'jansson-object': 72,                              // json_object_t total

  // hashtable_t (56 bytes)
  // Reference: hashtable.h
  'jansson-hashtable': 56,                           // hashtable_t total (embedded in json_object_t)
  'jansson-ht-size': 8,                              // size_t size (OVERHEAD - count!)
  'jansson-ht-buckets-ptr': 8,                       // bucket_t* buckets (OVERHEAD - pointer)
  'jansson-ht-order': 8,                             // size_t order (OVERHEAD - log2)
  'jansson-ht-list-prev': 8,                         // list.prev (OVERHEAD)
  'jansson-ht-list-next': 8,                         // list.next (OVERHEAD)
  'jansson-ht-ordered-prev': 8,                      // ordered_list.prev (OVERHEAD)
  'jansson-ht-ordered-next': 8,                      // ordered_list.next (OVERHEAD)

  // hashtable_bucket (bucket_t) - 16 bytes each
  // Reference: hashtable.h - INITIAL_HASHTABLE_ORDER = 3 means 8 buckets
  'jansson-bucket': 16,                              // bucket_t (first + last pointers)
  'jansson-bucket-first': 8,                         // first pointer (OVERHEAD)
  'jansson-bucket-last': 8,                          // last pointer (OVERHEAD)
  'jansson-bucket-array': (_order: number) => 8 * 16, // 8 buckets × 16 bytes = 128 bytes FIXED!

  // hashtable_pair - 56 bytes + key_len + 1 (null terminator)
  // Reference: hashtable.h
  'jansson-pair': 56,                                // hashtable_pair base (without key data)
  'jansson-pair-list-prev': 8,                       // list.prev (OVERHEAD - collision chain)
  'jansson-pair-list-next': 8,                       // list.next (OVERHEAD - collision chain)
  'jansson-pair-ordered-prev': 8,                    // ordered_list.prev (OVERHEAD - insertion order)
  'jansson-pair-ordered-next': 8,                    // ordered_list.next (OVERHEAD - insertion order)
  'jansson-pair-hash': 8,                            // size_t hash (OVERHEAD - cached hash)
  'jansson-pair-value-ptr': 8,                       // json_t* value (OVERHEAD - pointer)
  'jansson-pair-key-len': 8,                         // size_t key_len (OVERHEAD - length!)
  'jansson-pair-key-data': (len: number) => len,     // char key[] (DATA - flexible array)
  'jansson-pair-key-null': 1,                        // null terminator (OVERHEAD)

  // json_string_t = json_t (16) + char* (8) + size_t (8) = 32 bytes
  // Reference: jansson_private.h
  'jansson-string': 32,                              // json_string_t header
  'jansson-string-ptr': 8,                           // char* value (OVERHEAD - pointer)
  'jansson-string-length': 8,                        // size_t length (OVERHEAD - length!)
  'jansson-string-data': (len: number) => len,       // heap char[] (DATA)
  'jansson-string-null': 1,                          // null terminator (OVERHEAD)

  // json_integer_t = json_t (16) + json_int_t (8) = 24 bytes
  // Reference: jansson_private.h
  'jansson-integer': 24,                             // json_integer_t total
  'jansson-int-value': 8,                            // json_int_t value (DATA)

  // json_real_t = json_t (16) + double (8) = 24 bytes
  // Reference: jansson_private.h
  'jansson-real': 24,                                // json_real_t total
  'jansson-real-value': 8,                           // double value (DATA)

  // json_t with JSON_TRUE/JSON_FALSE/JSON_NULL - just the base (16 bytes)
  // These are singleton-like but still allocated per instance in Jansson
  'jansson-true': 16,                                // json_t for true
  'jansson-false': 16,                               // json_t for false
  'jansson-null': 16,                                // json_t for null

  // json_array_t = json_t (16) + size_t size (8) + size_t entries (8) + json_t** table (8) = 40 bytes
  // Reference: jansson_private.h
  'jansson-array': 40,                               // json_array_t header
  'jansson-array-size': 8,                           // size_t size (OVERHEAD - capacity)
  'jansson-array-entries': 8,                        // size_t entries (OVERHEAD - count!)
  'jansson-array-table-ptr': 8,                      // json_t** table (OVERHEAD - pointer)
  'jansson-array-slot': 8,                           // json_t* per element (OVERHEAD - pointer)
} as const;

/**
 * Helper to estimate size based on node type
 */
export function estimateSize(
  nodeType: NodeType,
  value?: unknown
): number {
  const estimate = SIZE_ESTIMATES[nodeType as keyof typeof SIZE_ESTIMATES];

  if (typeof estimate === 'function') {
    if (typeof value === 'string') {
      return estimate(value.length);
    }
    if (Array.isArray(value)) {
      return estimate(value.length);
    }
    if (typeof value === 'object' && value !== null) {
      return estimate(Object.keys(value).length);
    }
    return estimate(0);
  }

  return estimate ?? 16;
}

/**
 * Create a node with common defaults
 */
export function createNode(
  id: string,
  type: NodeType,
  label: string,
  options: {
    caption?: string;
    sizeBytes?: number;
    isOverhead?: boolean;
    isDuplicate?: boolean;
    isInterned?: boolean;
    refCount?: number;
    value?: unknown;
    implementation: 'hakka' | 'cpython' | 'serde' | 'go' | 'jansson';
  }
): NvlNode {
  return {
    id,
    label,
    caption: options.caption,
    properties: {
      type,
      implementation: options.implementation,
      sizeBytes: options.sizeBytes ?? estimateSize(type, options.value),
      isOverhead: options.isOverhead,
      isDuplicate: options.isDuplicate,
      isInterned: options.isInterned,
      refCount: options.refCount,
      value: typeof options.value === 'string' ? options.value : undefined,
    },
  };
}

/**
 * Create a relationship with common defaults
 */
export function createRelationship(
  id: string,
  from: string,
  to: string,
  type: RelationshipType,
  properties?: Record<string, unknown>
): NvlRelationship {
  return {
    id,
    from,
    to,
    type,
    properties,
  };
}
