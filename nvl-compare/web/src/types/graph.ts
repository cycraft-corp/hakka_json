/**
 * Node types for each implementation
 * Grouped by implementation for clarity
 */
export type NodeType =
  // ========== HakkaJson (C++) ==========
  // Manager layer
  | 'hakka-registry'           // ManagerRegistry singleton
  | 'hakka-scalar-manager'     // ScalarManagerCompact
  | 'hakka-string-manager'     // StringManagerCompact (interning pool)
  | 'hakka-array-manager'      // ArrayManagerCompact
  | 'hakka-object-manager'     // ObjectManagerCompact
  // Handle layer
  | 'hakka-handle'             // 32-bit HandleManagerToken (uint32_t)
  | 'hakka-owned-pointer'      // OwnedUniformCompactPointer in handles_[] (tagged void*)
  | 'hakka-freelist'           // Recycled slot in freelist_
  // Value layer
  | 'hakka-nan-boxed'          // NaN-boxed scalar (int/float/bool/null)
  | 'hakka-interned-string'    // Interned string in pool
  | 'hakka-array-value'        // JsonArrayCompact
  | 'hakka-object-value'       // JsonObjectCompact
  | 'hakka-array-storage'      // Array storage node (generated)
  | 'hakka-object-storage'     // Object storage node (generated)
  // POD-level HakkaJson internals
  | 'hakka-handle-type-bits'   // top 2 bits of HandleManagerToken (Scalar/String/Array/Object)
  | 'hakka-handle-index-bits'  // bottom 30 bits - index into handles_[]
  | 'hakka-ptr-type-tag'       // high 4 bits of OwnedUniformCompactPointer (HakkaJsonType enum)
  | 'hakka-ptr-address'        // lower 60 bits - actual memory address
  | 'hakka-primitive-value'    // JsonPrimitiveCompact::value_ field
  | 'hakka-primitive-refcount' // JsonPrimitiveCompact::ref_count (atomic<uint64_t>)
  | 'hakka-int-data'           // int64_t value inside JsonIntCompact
  | 'hakka-float-data'         // double value inside JsonFloatCompact (IEEE 754)
  | 'hakka-nan-pattern'        // NaN-boxed bit pattern for bool/null/invalid
  | 'hakka-string-proxy'       // PicoStringProxy inside JsonStringCompact
  | 'hakka-string-ptr'         // char* data pointer
  | 'hakka-string-len'         // size_t length field

  // ========== CPython json ==========
  | 'py-dict'                  // PyDictObject (48+ bytes)
  | 'py-dict-keys'             // PyDictKeysObject (shared keys)
  | 'py-dict-entry'            // (key, value) in dk_entries[]
  | 'py-list'                  // PyListObject (40 bytes)
  | 'py-list-items'            // ob_item[] heap array
  | 'py-unicode'               // PyUnicodeObject/PyASCIIObject
  | 'py-long'                  // PyLongObject
  | 'py-float'                 // PyFloatObject
  | 'py-bool-singleton'        // Py_True / Py_False
  | 'py-none-singleton'        // Py_None
  | 'py-type-ptr'              // ob_type pointer overhead
  | 'py-none'                  // None value (alias for visualization)
  | 'py-bool'                  // Boolean value (alias for visualization)
  | 'py-refcount'              // Reference count node (generated)
  // POD-level CPython
  | 'py-ob-refcnt'             // ob_refcnt field (Py_ssize_t)
  | 'py-ob-type'               // ob_type field (PyTypeObject*)
  | 'py-lv-tag'                // lv_tag field (Python 3.12+)
  | 'py-ob-size'               // ob_size field (Python 3.11-)
  | 'py-long-digit'            // long_value.ob_digit[] in PyLongObject
  | 'py-float-fval'            // ob_fval in PyFloatObject (double)
  | 'py-unicode-length'        // length field
  | 'py-unicode-hash'          // hash field (Py_hash_t)
  | 'py-unicode-state'         // state bitfield
  | 'py-unicode-data'          // inline char/wchar data

  // ========== serde_json (Rust) ==========
  | 'serde-value-enum'         // Value enum (24 bytes)
  | 'serde-string'             // String (24 bytes + heap)
  | 'serde-string-data'        // heap backing for String
  | 'serde-vec'                // Vec<Value> header
  | 'serde-vec-data'           // heap array of Values
  | 'serde-indexmap'           // IndexMap<String, Value>
  | 'serde-map-entry'          // (String, Value) entry
  | 'serde-number'             // Number (PosInt/NegInt/Float)
  | 'serde-array'              // Array container (generated)
  // POD-level Rust
  | 'serde-discriminant'       // enum discriminant (u8/u64)
  | 'serde-n-variant'          // Number's N enum variant
  | 'rust-ptr'                 // raw pointer (*mut T)
  | 'rust-usize'               // usize (len, cap)
  | 'rust-hash-seed'           // RandomState k0/k1

  // ========== encoding/json (Go) ==========
  | 'go-interface'             // interface{} / any (16 bytes)
  | 'go-string'                // string header (16 bytes)
  | 'go-string-data'           // backing []byte
  | 'go-slice'                 // []interface{} header (24 bytes)
  | 'go-slice-data'            // backing array
  | 'go-hmap'                  // map header (48 bytes)
  | 'go-bucket'                // bmap (272 bytes for 8 k-v pairs)
  | 'go-float64'               // all numbers become float64
  | 'go-bool'                  // Boolean value (generated)
  // POD-level Go
  | 'go-type-struct'           // _type metadata struct
  | 'go-type-size'             // _type.size
  | 'go-type-kind'             // _type.kind
  | 'go-type-hash'             // _type.hash
  | 'go-hmap-count'            // hmap.count
  | 'go-hmap-b'                // hmap.B (log2 buckets)
  | 'go-hmap-hash0'            // hmap.hash0 (seed)
  | 'go-tophash'               // bmap.tophash[8]
  | 'go-bucket-keys'           // keys region in bmap
  | 'go-bucket-values'         // values region in bmap
  | 'go-string-len'            // string.len field

  // ========== Jansson (C) ==========
  | 'jansson-object'           // json_object_t (json_t + hashtable_t)
  | 'jansson-hashtable'        // hashtable_t (~56 bytes)
  | 'jansson-pair'             // hashtable_pair (56+ bytes, includes key inline)
  | 'jansson-array'            // json_array_t (json_t + size + entries + table)
  | 'jansson-array-data'       // json_t** elements heap array
  | 'jansson-string'           // json_string_t (json_t + char* + length)
  | 'jansson-string-data'      // char* backing on heap
  | 'jansson-integer'          // json_integer_t (json_t + json_int_t)
  | 'jansson-real'             // json_real_t (json_t + double)
  | 'jansson-true'             // json_true singleton
  | 'jansson-false'            // json_false singleton
  | 'jansson-null'             // json_null singleton
  // POD-level Jansson internals
  | 'jansson-type-enum'        // json_type enum (int, 4 bytes)
  | 'jansson-refcount'         // volatile size_t refcount (8 bytes)
  | 'jansson-int-value'        // json_int_t (int64_t) inside json_integer_t
  | 'jansson-real-value'       // double (IEEE 754) inside json_real_t
  | 'jansson-string-ptr'       // char* value pointer in json_string_t
  | 'jansson-string-length'    // size_t length in json_string_t
  | 'jansson-array-size'       // size_t allocated capacity
  | 'jansson-array-entries'    // size_t actual entry count
  | 'jansson-array-table'      // json_t** table pointer
  | 'jansson-ht-size'          // hashtable_t.size (number of pairs)
  | 'jansson-ht-order'         // hashtable_t.order (log2 of bucket count)
  | 'jansson-ht-buckets'       // hashtable_bucket* pointer
  | 'jansson-pair-hash'        // size_t hash value in hashtable_pair
  | 'jansson-pair-value-ptr'   // json_t* value pointer in hashtable_pair
  | 'jansson-pair-key-len'     // size_t key_len in hashtable_pair
  | 'jansson-pair-key-data'    // char key[] flexible array member
  | 'jansson-list-prev'        // struct hashtable_list.prev pointer
  | 'jansson-list-next'        // struct hashtable_list.next pointer

  // ========== POD (Plain Old Data) Types ==========
  | 'pod-int8'                 // int8_t / char (1 byte)
  | 'pod-uint8'                // uint8_t / unsigned char (1 byte)
  | 'pod-int16'                // int16_t / short (2 bytes)
  | 'pod-uint16'               // uint16_t / unsigned short (2 bytes)
  | 'pod-int32'                // int32_t / int (4 bytes)
  | 'pod-uint32'               // uint32_t / unsigned int (4 bytes)
  | 'pod-int64'                // int64_t / long long (8 bytes)
  | 'pod-uint64'               // uint64_t / unsigned long long (8 bytes)
  | 'pod-float32'              // float (4 bytes, IEEE 754)
  | 'pod-float64'              // double (8 bytes, IEEE 754)
  | 'pod-ptr'                  // void* pointer (8 bytes on 64-bit)
  | 'pod-char-array'           // char[] inline string data
  | 'pod-byte-array'           // uint8_t[] raw bytes
  | 'pod-bitfield';            // packed bit flags

/**
 * Relationship types between nodes
 */
export type RelationshipType =
  // HakkaJson relationships (high-level)
  | 'registry-manages'         // Registry → Manager
  | 'manager-stores'           // Manager → OwnedPointer (handles_[])
  | 'handle-resolves'          // Handle → Manager (type bits)
  | 'handle-indexes'           // Handle → OwnedPointer (index bits)
  | 'pointer-wraps'            // OwnedPointer → Value
  | 'value-contains'           // Array/Object → child handles
  | 'string-interned-at'       // Reference to interned string
  // HakkaJson deep/POD relationships
  | 'handle-to-type-bits'      // HandleManagerToken → top 2 bits
  | 'handle-to-index-bits'     // HandleManagerToken → bottom 30 bits
  | 'ptr-to-type-tag'          // OwnedUniformCompactPointer → high 4 bits
  | 'ptr-to-address'           // OwnedUniformCompactPointer → lower 60 bits
  | 'primitive-to-refcount'    // JsonPrimitiveCompact → ref_count
  | 'primitive-to-value'       // JsonPrimitiveCompact → value_ field
  | 'int-to-data'              // JsonIntCompact → int64_t value POD
  | 'float-to-data'            // JsonFloatCompact → double value POD
  | 'float-to-nan-bits'        // JsonFloatCompact → NaN-boxed bit pattern
  | 'string-to-proxy'          // JsonStringCompact → PicoStringProxy
  | 'proxy-to-ptr'             // PicoStringProxy → char* data
  | 'proxy-to-len'             // PicoStringProxy → size_t length
  // Generic relationships used by generators
  | 'manages'                  // Manager → managed element
  | 'contains'                 // Container → contained element
  | 'owns'                     // Ownership relationship
  | 'interns'                  // String interning relationship
  | 'references'               // Reference to another node
  | 'points-to'                // Pointer relationship

  // CPython relationships
  | 'dict-to-keys'             // PyDict → PyDictKeysObject
  | 'keys-to-entry'            // PyDictKeys → entries[]
  | 'entry-key'                // entry → key PyUnicode
  | 'entry-value'              // entry → value PyObject
  | 'list-to-items'            // PyList → ob_item[]
  | 'items-element'            // ob_item → element
  | 'ob-type'                  // PyObject → ob_type (overhead)
  | 'has-type-ptr'             // Object → type pointer (overhead)
  | 'has-refcount'             // Object → refcount node
  | 'has-key'                  // Object/Map → key element
  // CPython deep relationships
  | 'pyobj-refcnt'             // PyObject → ob_refcnt field
  | 'pyobj-type'               // PyObject → ob_type field
  | 'pylong-lvtag'             // PyLong → lv_tag (Python 3.12+)
  | 'pyobj-size'               // PyVarObject → ob_size field
  | 'pylong-digits'            // PyLong → long_value.ob_digit[]
  | 'digit-value'              // ob_digit[i] → uint32 POD
  | 'pyfloat-fval'             // PyFloat → ob_fval (double)
  | 'pyunicode-length'         // PyUnicode → length field
  | 'pyunicode-hash'           // PyUnicode → hash field
  | 'pyunicode-state'          // PyUnicode → state bitfield
  | 'pyunicode-data'           // PyUnicode → char[] data
  | 'char-at'                  // char[] → individual char

  // Rust serde_json relationships
  | 'enum-contains'            // Value enum → inner data
  | 'string-to-heap'           // String → heap data
  | 'vec-to-data'              // Vec → backing array
  | 'vec-element'              // Vec data → element Value
  | 'map-to-entry'             // IndexMap → entries
  | 'map-entry-key'            // entry → key String
  | 'map-entry-value'          // entry → value Value
  // Rust deep relationships
  | 'enum-discriminant'        // Value → discriminant byte
  | 'number-to-n'              // Number → N enum
  | 'n-to-payload'             // N → u64/i64/f64 POD
  | 'string-ptr'               // String → ptr field
  | 'string-len'               // String → len field
  | 'string-cap'               // String → cap field
  | 'ptr-to-bytes'             // ptr → heap bytes
  | 'indexmap-indices'         // IndexMap → indices RawTable
  | 'indexmap-entries'         // IndexMap → entries Vec
  | 'bucket-hash'              // Bucket → hash field
  | 'bucket-key'               // Bucket → key field
  | 'bucket-value'             // Bucket → value field

  // Go relationships
  | 'iface-type'               // interface → type ptr
  | 'iface-data'               // interface → data ptr
  | 'wraps'                    // interface{} → wrapped value
  | 'string-to-bytes'          // string → []byte
  | 'slice-to-data'            // slice → backing array
  | 'slice-element'            // slice data → element
  | 'hmap-to-buckets'          // hmap → bucket array
  | 'bucket-overflow'          // bucket → overflow bucket
  | 'bucket-entry'             // bucket → k-v entry
  | 'has-bucket'               // map → bucket
  // Go deep relationships
  | 'iface-type-ptr'           // interface → _type pointer
  | 'iface-data-ptr'           // interface → data pointer
  | 'type-to-size'             // _type → size field
  | 'type-to-kind'             // _type → kind field
  | 'type-to-hash'             // _type → hash field
  | 'hmap-count'               // hmap → count field
  | 'hmap-b'                   // hmap → B field
  | 'hmap-hash0'               // hmap → hash0 field
  | 'hmap-buckets-ptr'         // hmap → buckets pointer
  | 'bmap-tophash'             // bmap → tophash[8] array
  | 'bmap-keys'                // bmap → keys region
  | 'bmap-values'              // bmap → values region
  | 'bmap-overflow'            // bmap → overflow pointer
  | 'stringhdr-ptr'            // string → str pointer
  | 'stringhdr-len'            // string → len field

  // Jansson relationships (high-level)
  | 'object-to-hashtable'      // json_object → hashtable
  | 'hashtable-to-pair'        // hashtable → pair
  | 'has-hashtable'            // object → hashtable
  | 'has-pair'                 // hashtable → pair entry
  | 'pair-chain'               // pair → next pair (collision)
  | 'pair-to-key'              // pair → key char*
  | 'pair-to-value'            // pair → value json_t*
  | 'array-to-elements'        // json_array → elements[]
  | 'element-ptr'              // elements → json_t*
  // Jansson deep/POD relationships
  | 'json-t-to-type'           // json_t → type enum field
  | 'json-t-to-refcount'       // json_t → refcount field
  | 'integer-to-value'         // json_integer_t → json_int_t value POD
  | 'real-to-value'            // json_real_t → double value POD
  | 'jstring-to-ptr'           // json_string_t → char* value pointer
  | 'jstring-to-len'           // json_string_t → size_t length
  | 'jstring-ptr-to-data'      // char* → heap char[] data
  | 'jarray-to-size'           // json_array_t → size
  | 'jarray-to-entries'        // json_array_t → entries
  | 'jarray-to-table'          // json_array_t → json_t** table pointer
  | 'jarray-table-to-element'  // table[i] → json_t* element
  | 'ht-to-size'               // hashtable_t → size
  | 'ht-to-order'              // hashtable_t → order
  | 'ht-to-buckets'            // hashtable_t → buckets pointer
  | 'pair-to-hash'             // hashtable_pair → hash value
  | 'pair-to-key-len'          // hashtable_pair → key_len
  | 'pair-to-key-data'         // hashtable_pair → key[] inline char array
  | 'pair-to-list-prev'        // hashtable_pair → list.prev pointer
  | 'pair-to-list-next'        // hashtable_pair → list.next pointer
  | 'pair-to-ordered-prev'     // hashtable_pair → ordered_list.prev
  | 'pair-to-ordered-next'     // hashtable_pair → ordered_list.next

  // Generic struct field relationships
  | 'struct-field'             // struct → named field
  | 'field-value'              // field → POD value
  | 'array-index'              // array → element by index
  | 'pointer-deref';           // pointer → dereferenced memory

/**
 * Node in the NVL graph
 */
export interface NvlNode {
  id: string;
  label?: string;
  color?: string;
  size?: number;
  caption?: string;
  properties?: {
    type: NodeType;
    implementation: 'hakka' | 'cpython' | 'serde' | 'go' | 'jansson' | 'pod';
    // Memory information
    address?: string;        // Hex address for visualization
    sizeBytes?: number;      // Actual allocation size
    offset?: number;         // Offset within parent struct
    // Value information
    value?: string;          // Display value for primitives
    rawHex?: string;         // Raw hex representation (e.g., "0x0000250F")
    // HakkaJson specific
    tokenBits?: string;      // "type:XX index:XXXXXX"
    isInterned?: boolean;    // String interning status
    refCount?: number;       // For refcounted implementations
    // Overhead indicators
    isOverhead?: boolean;    // Visual indicator for overhead
    isDuplicate?: boolean;   // Duplicate allocation marker
    // POD-level information
    isPod?: boolean;         // Is this a POD (leaf) node?
    cType?: string;          // C type name (e.g., "uint32_t", "double")
    fieldName?: string;      // Struct field name (e.g., "ob_refcnt")
    bitWidth?: number;       // For bitfields
    bitOffset?: number;      // For bitfields
    // Array element info
    arrayIndex?: number;     // Index in array (for ob_digit[], etc.)
  };
}

/**
 * Relationship (edge) in the NVL graph
 */
export interface NvlRelationship {
  id: string;
  from: string;
  to: string;
  type: RelationshipType;
  caption?: string;
  color?: string;
  width?: number;
  properties?: {
    isOverhead?: boolean;    // Overhead pointer (e.g., ob_type)
    index?: number;          // Array/vector index
    key?: string;            // Object key name
  };
}

/**
 * Complete graph data for one implementation
 */
export interface GraphData {
  id: string;
  name: string;
  language: string;
  description: string;
  stats: {
    nodeCount: number;
    edgeCount: number;
    totalBytes: number;
    overheadBytes: number;
    overheadPercent: number;
    uniqueStrings: number;
    duplicateStrings: number;
    internedStrings?: number;      // HakkaJson
    nanBoxedValues?: number;       // HakkaJson
    interfaceWrappers?: number;    // Go
    refCountedObjects?: number;    // CPython, Jansson
    bytesSaved?: number;           // Bytes saved through string interning (HakkaJson)
  };
  nodes: NvlNode[];
  relationships: NvlRelationship[];
}

/**
 * Implementation metadata for selector
 */
export interface Implementation {
  id: string;
  name: string;
  language: string;
  file: string;
  color: string;  // Primary color for this implementation
}

/**
 * Implementation ID type for type-safe data loading
 */
export type ImplementationId = 'hakka_json' | 'serde_json' | 'cpython_json' | 'go_json' | 'jansson';

/**
 * Available implementations for comparison
 */
export const IMPLEMENTATIONS: Implementation[] = [
  { id: 'hakka_json', name: 'HakkaJson', language: 'C++', file: 'hakka_json.json', color: '#10B981' },
  { id: 'serde_json', name: 'serde_json', language: 'Rust', file: 'serde_json.json', color: '#F97316' },
  { id: 'cpython_json', name: 'json', language: 'CPython', file: 'cpython_json.json', color: '#3B82F6' },
  { id: 'go_json', name: 'encoding/json', language: 'Go', file: 'go_json.json', color: '#06B6D4' },
  { id: 'jansson', name: 'Jansson', language: 'C', file: 'jansson.json', color: '#8B5CF6' },
];
