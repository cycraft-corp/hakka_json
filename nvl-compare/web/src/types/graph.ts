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
  // Value layer
  | 'hakka-nan-boxed'          // NaN-boxed scalar (int/float/bool/null)
  | 'hakka-interned-string'    // Interned string in pool (PicoString)
  | 'hakka-array'              // JsonArrayCompact (vec<handle> + refcount)
  | 'hakka-object'             // JsonObjectCompact (2 array handles + refcount)

  // ========== CPython json ==========
  // Infrastructure (type objects - overhead)
  | 'py-type-pool'             // Container for type objects
  | 'py-type-object'           // PyTypeObject base struct (424 bytes)
  // Protocol method structs (part of type objects)
  | 'py-number-methods'        // PyNumberMethods (280 bytes = 35 func ptrs)
  | 'py-sequence-methods'      // PySequenceMethods (80 bytes = 10 func ptrs)
  | 'py-mapping-methods'       // PyMappingMethods (24 bytes = 3 func ptrs)
  // Container types
  | 'py-dict'                  // PyDictObject (216 bytes data + 16 header)
  | 'py-dict-keys'             // PyDictKeysObject (shared keys)
  | 'py-dict-entry'            // (key, value) in dk_entries[]
  | 'py-list'                  // PyListObject (40 bytes data + 16 header)
  | 'py-list-items'            // ob_item[] heap array
  // Value types
  | 'py-unicode'               // PyUnicodeObject/PyASCIIObject
  | 'py-long'                  // PyLongObject (16 bytes data + 16 header)
  | 'py-float'                 // PyFloatObject (8 bytes data + 16 header)
  | 'py-bool-singleton'        // Py_True / Py_False
  | 'py-none-singleton'        // Py_None
  | 'py-none'                  // None value (0 bytes data + 16 header)
  | 'py-bool'                  // Boolean value (16 bytes data + 16 header)
  // Overhead nodes (16 bytes per object)
  | 'py-type-ptr'              // ob_type pointer (8 bytes overhead)
  | 'py-refcount'              // ob_refcnt (8 bytes overhead)
  // POD-level CPython - Internal fields of each object type
  // PyObject base fields
  | 'py-ob-refcnt'             // ob_refcnt field (Py_ssize_t, 8B)
  | 'py-ob-type'               // ob_type field (PyTypeObject*, 8B)
  | 'py-ob-size'               // ob_size field for PyVarObject (Py_ssize_t, 8B)

  // PyDictObject fields (32 bytes excluding header)
  | 'py-dict-ma-used'          // ma_used: number of items (8B)
  | 'py-dict-ma-version'       // ma_version_tag: version for dict views (8B)
  | 'py-dict-ma-keys-ptr'      // ma_keys: pointer to PyDictKeysObject (8B)
  | 'py-dict-ma-values-ptr'    // ma_values: pointer or NULL (8B)

  // PyDictKeysObject fields (~40 bytes header + variable)
  | 'py-dict-keys-refcnt'      // dk_refcnt: shared keys refcount (8B)
  | 'py-dict-keys-log2'        // dk_log2_size + dk_log2_index_bytes + dk_kind (3B)
  | 'py-dict-keys-version'     // dk_version (4B)
  | 'py-dict-keys-usable'      // dk_usable: usable entries (8B)
  | 'py-dict-keys-nentries'    // dk_nentries: actual entries (8B)
  | 'py-dict-keys-indices'     // dk_indices[]: hash table indices (variable)
  | 'py-dict-keys-entries'     // dk_entries[]: array of PyDictKeyEntry

  // PyDictKeyEntry fields (24 bytes per entry)
  | 'py-dict-entry-hash'       // me_hash: cached hash (8B)
  | 'py-dict-entry-key-ptr'    // me_key: pointer to key object (8B)
  | 'py-dict-entry-value-ptr'  // me_value: pointer to value object (8B)

  // PyListObject fields (24 bytes excluding header)
  | 'py-list-ob-size'          // ob_size: item count from PyVarObject (8B)
  | 'py-list-ob-item-ptr'      // ob_item: pointer to item array (8B)
  | 'py-list-allocated'        // allocated: capacity (8B)
  | 'py-list-items-array'      // The heap array of PyObject* pointers
  | 'py-list-item-slot'        // Individual slot in ob_item array (8B pointer)

  // PyLongObject fields
  | 'py-long-ob-size'          // ob_size: digit count, sign in high bit (8B)
  | 'py-long-digit'            // ob_digit[]: 30-bit digits (4B each)
  | 'py-lv-tag'                // lv_tag field (Python 3.12+ compact int)

  // PyFloatObject fields
  | 'py-float-fval'            // ob_fval: the double value (8B)

  // PyUnicodeObject (PyASCIIObject) fields
  | 'py-unicode-length'        // length: string length (8B)
  | 'py-unicode-hash'          // hash: cached hash, -1 if not computed (8B)
  | 'py-unicode-state'         // state: bitfield flags (4B)
  | 'py-unicode-wstr'          // wstr: deprecated legacy pointer (8B)
  | 'py-unicode-data'          // inline char data (variable)

  // ========== serde_json (Rust) ==========
  // Container types
  | 'serde-value-enum'         // Value enum container (32 bytes)
  | 'serde-string'             // String container (24 bytes stack)
  | 'serde-string-data'        // heap backing for String (actual data)
  | 'serde-vec'                // Vec<Value> header
  | 'serde-vec-data'           // heap array of Values
  | 'serde-indexmap'           // IndexMap<String, Value>
  | 'serde-btreemap'           // BTreeMap<String, Value> (default Map)
  | 'serde-map-entry'          // (String, Value) entry
  | 'serde-number'             // Number container (16 bytes)
  | 'serde-array'              // Array container (generated)

  // Value enum fields (32 bytes total)
  | 'serde-discriminant'       // enum discriminant [8B] (overhead - type tag)
  | 'serde-padding'            // alignment padding (overhead)

  // String fields (24 bytes stack + heap)
  | 'serde-string-ptr'         // ptr to heap data [8B] (overhead)
  | 'serde-string-len'         // length [8B] (overhead - metadata)
  | 'serde-string-cap'         // capacity [8B] (overhead - over-allocation)

  // Number fields (16 bytes)
  | 'serde-n-discriminant'     // N enum discriminant [8B] (overhead - inner type tag)
  | 'serde-n-value'            // actual numeric value [8B] (DATA)

  // Vec fields (24 bytes stack + heap)
  | 'serde-vec-ptr'            // ptr to heap array [8B] (overhead)
  | 'serde-vec-len'            // length [8B] (overhead - metadata)
  | 'serde-vec-cap'            // capacity [8B] (overhead - over-allocation)
  | 'serde-vec-slot'           // slot in heap array [32B] (overhead - pointer to Value)

  // BTreeMap fields (16 bytes stack + ~628 bytes per LeafNode!)
  | 'serde-btree-root'         // root node pointer [8B] (overhead)
  | 'serde-btree-len'          // entry count [8B] (overhead - metadata)
  | 'serde-btree-node'         // internal B-tree node (overhead - tree structure)
  | 'serde-btree-leaf'         // LeafNode<String, Value> (~628 bytes!)

  // BTreeMap LeafNode<String, Value> internals (B=6, CAPACITY=11)
  // Reference: rust/library/alloc/src/collections/btree/node.rs
  | 'serde-leafnode-parent'    // parent: Option<NonNull<InternalNode>> [8B] (overhead)
  | 'serde-leafnode-parent-idx' // parent_idx: MaybeUninit<u16> [2B] (overhead)
  | 'serde-leafnode-len'       // len: u16 [2B] (overhead - metadata)
  | 'serde-leafnode-keys'      // keys: [MaybeUninit<String>; 11] container
  | 'serde-leafnode-vals'      // vals: [MaybeUninit<Value>; 11] container
  | 'serde-leafnode-key-slot'  // individual key slot [24B] - used or WASTED
  | 'serde-leafnode-val-slot'  // individual val slot [32B] - used or WASTED
  | 'serde-leafnode-wasted-key' // WASTED key slot [24B] (overhead - unused allocation)
  | 'serde-leafnode-wasted-val' // WASTED val slot [32B] (overhead - unused allocation)

  // Legacy/compatibility
  | 'serde-n-variant'          // Number's N enum variant
  | 'rust-ptr'                 // raw pointer (*mut T)
  | 'rust-usize'               // usize (len, cap)
  | 'rust-hash-seed'           // RandomState k0/k1

  // ========== encoding/json (Go) ==========
  // Reference: runtime/runtime2.go, runtime/string.go, runtime/map.go
  //
  // Container/aggregate types
  | 'go-interface'             // interface{} / eface (16 bytes total)
  | 'go-string'                // string header (16 bytes) + heap data
  | 'go-string-data'           // backing []byte on heap (DATA)
  | 'go-slice'                 // []interface{} header (24 bytes)
  | 'go-slice-data'            // backing array
  | 'go-hmap'                  // hmap header (48 bytes)
  | 'go-bucket'                // bmap (272 bytes for 8 k-v pairs)
  | 'go-float64'               // float64 value (8 bytes DATA)
  | 'go-bool'                  // bool value (1 byte DATA)

  // interface{} (eface) internal fields - runtime/runtime2.go
  | 'go-iface-type'            // _type pointer [8B] (OVERHEAD)
  | 'go-iface-data'            // data pointer [8B] (OVERHEAD)

  // string (stringStruct) internal fields - runtime/string.go
  | 'go-string-ptr'            // str pointer [8B] (OVERHEAD)
  | 'go-string-len'            // len field [8B] (OVERHEAD - metadata!)

  // []interface{} slice internal fields - runtime/slice.go
  | 'go-slice-ptr'             // array pointer [8B] (OVERHEAD)
  | 'go-slice-len'             // length [8B] (OVERHEAD - metadata!)
  | 'go-slice-cap'             // capacity [8B] (OVERHEAD)
  | 'go-slice-slot'            // individual interface{} slot [16B]
  | 'go-slice-wasted-slot'     // WASTED slot [16B] (OVERHEAD - unused!)

  // hmap internal fields - runtime/map.go
  | 'go-hmap-count'            // count: int [8B] (OVERHEAD - length!)
  | 'go-hmap-flags'            // flags: uint8 [1B] (OVERHEAD)
  | 'go-hmap-b'                // B: uint8 log2 buckets [1B] (OVERHEAD)
  | 'go-hmap-noverflow'        // noverflow: uint16 [2B] (OVERHEAD)
  | 'go-hmap-hash0'            // hash0: uint32 seed [4B] (OVERHEAD)
  | 'go-hmap-buckets'          // buckets: pointer [8B] (OVERHEAD)
  | 'go-hmap-oldbuckets'       // oldbuckets: pointer [8B] (OVERHEAD)
  | 'go-hmap-nevacuate'        // nevacuate: uintptr [8B] (OVERHEAD)
  | 'go-hmap-extra'            // extra: *mapextra [8B] (OVERHEAD)

  // bucket (bmap) internal fields - runtime/map.go, bucketCnt=8
  | 'go-bucket-tophash'        // tophash[8]: 8 × uint8 container
  | 'go-bucket-tophash-slot'   // individual tophash [1B] (OVERHEAD)
  | 'go-bucket-tophash-wasted' // WASTED tophash [1B] (OVERHEAD)
  | 'go-bucket-keys'           // keys region container (8 × 16B = 128B)
  | 'go-bucket-vals'           // vals region container (8 × 16B = 128B)
  | 'go-bucket-overflow'       // overflow pointer [8B] (OVERHEAD)
  | 'go-bucket-key-slot'       // individual key slot [16B] (string header)
  | 'go-bucket-val-slot'       // individual val slot [16B] (interface{})
  | 'go-bucket-wasted-key'     // WASTED key slot [16B] (OVERHEAD - unused!)
  | 'go-bucket-wasted-val'     // WASTED val slot [16B] (OVERHEAD - unused!)

  // Legacy POD-level Go (kept for compatibility)
  | 'go-type-struct'           // _type metadata struct
  | 'go-type-size'             // _type.size
  | 'go-type-kind'             // _type.kind
  | 'go-type-hash'             // _type.hash
  | 'go-tophash'               // bmap.tophash[8]
  | 'go-bucket-values'         // values region in bmap

  // ========== Jansson (C) ==========
  // Reference: jansson.h, jansson_private.h, hashtable.h
  //
  // json_t base structure (16 bytes with padding)
  | 'jansson-json-t'           // json_t base (type + refcount + padding)
  | 'jansson-type-enum'        // json_type enum [4B] (OVERHEAD)
  | 'jansson-refcount'         // volatile size_t refcount [8B] (OVERHEAD)
  | 'jansson-padding'          // alignment padding [4B] (OVERHEAD)

  // json_object_t = json_t (16) + hashtable_t (56) = 72 bytes
  | 'jansson-object'           // json_object_t container

  // hashtable_t (56 bytes) - embedded in json_object_t
  | 'jansson-hashtable'        // hashtable_t container
  | 'jansson-ht-size'          // size_t size [8B] (OVERHEAD - count!)
  | 'jansson-ht-buckets-ptr'   // bucket_t* buckets [8B] (OVERHEAD - pointer)
  | 'jansson-ht-order'         // size_t order [8B] (OVERHEAD - log2)
  | 'jansson-ht-list-prev'     // list.prev [8B] (OVERHEAD)
  | 'jansson-ht-list-next'     // list.next [8B] (OVERHEAD)
  | 'jansson-ht-ordered-prev'  // ordered_list.prev [8B] (OVERHEAD)
  | 'jansson-ht-ordered-next'  // ordered_list.next [8B] (OVERHEAD)

  // hashtable_bucket (bucket_t) - 16 bytes each, 8 buckets = 128 bytes
  | 'jansson-bucket'           // bucket_t container
  | 'jansson-bucket-first'     // first pointer [8B] (OVERHEAD)
  | 'jansson-bucket-last'      // last pointer [8B] (OVERHEAD)
  | 'jansson-bucket-array'     // bucket array container

  // hashtable_pair - 56 bytes + key_len + 1 (null terminator)
  | 'jansson-pair'             // hashtable_pair container
  | 'jansson-pair-list-prev'   // list.prev [8B] (OVERHEAD - collision)
  | 'jansson-pair-list-next'   // list.next [8B] (OVERHEAD - collision)
  | 'jansson-pair-ordered-prev' // ordered_list.prev [8B] (OVERHEAD - order)
  | 'jansson-pair-ordered-next' // ordered_list.next [8B] (OVERHEAD - order)
  | 'jansson-pair-hash'        // size_t hash [8B] (OVERHEAD - cached)
  | 'jansson-pair-value-ptr'   // json_t* value [8B] (OVERHEAD - pointer)
  | 'jansson-pair-key-len'     // size_t key_len [8B] (OVERHEAD - length!)
  | 'jansson-pair-key-data'    // char key[] (DATA - flexible array)
  | 'jansson-pair-key-null'    // null terminator [1B] (OVERHEAD)

  // json_string_t = json_t (16) + char* (8) + size_t (8) = 32 bytes
  | 'jansson-string'           // json_string_t container
  | 'jansson-string-ptr'       // char* value [8B] (OVERHEAD - pointer)
  | 'jansson-string-length'    // size_t length [8B] (OVERHEAD - length!)
  | 'jansson-string-data'      // char* backing on heap (DATA)
  | 'jansson-string-null'      // null terminator [1B] (OVERHEAD)

  // json_integer_t = json_t (16) + json_int_t (8) = 24 bytes
  | 'jansson-integer'          // json_integer_t container
  | 'jansson-int-value'        // json_int_t value [8B] (DATA)

  // json_real_t = json_t (16) + double (8) = 24 bytes
  | 'jansson-real'             // json_real_t container
  | 'jansson-real-value'       // double value [8B] (DATA)

  // json_t for true/false/null (16 bytes each)
  | 'jansson-true'             // json_t for JSON_TRUE
  | 'jansson-false'            // json_t for JSON_FALSE
  | 'jansson-null'             // json_t for JSON_NULL

  // json_array_t = json_t (16) + size (8) + entries (8) + table* (8) = 40 bytes
  | 'jansson-array'            // json_array_t container
  | 'jansson-array-data'       // json_t** table heap array
  | 'jansson-array-size'       // size_t size [8B] (OVERHEAD - capacity)
  | 'jansson-array-entries'    // size_t entries [8B] (OVERHEAD - count!)
  | 'jansson-array-table'      // json_t** table [8B] (OVERHEAD - pointer)
  | 'jansson-array-slot'       // json_t* slot [8B] (OVERHEAD - pointer)

  // Legacy compatibility
  | 'jansson-ht-buckets'       // hashtable_bucket* pointer (alias)
  | 'jansson-list-prev'        // list.prev pointer (alias)
  | 'jansson-list-next'        // list.next pointer (alias)

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
  // HakkaJson relationships
  | 'registry-manages'         // Registry → Manager
  | 'manager-stores'           // Manager → handle entry
  | 'handle-resolves'          // Handle → Manager (type bits)
  | 'handle-indexes'           // Handle → entry (index bits)
  | 'value-contains'           // Array/Object → child handles
  | 'string-interned-at'       // Reference to interned string
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
  | 'array-element'            // Generic array → element relationship
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
