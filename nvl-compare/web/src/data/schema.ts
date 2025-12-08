import type { NodeType, RelationshipType } from '../types/graph';

/**
 * Color mapping for node types
 *
 * Design principle:
 * - HakkaJson: Cool greens/teals (efficient, modern)
 * - CPython: Blues (established, reliable)
 * - Rust: Oranges (performance, safety)
 * - Go: Cyans (simplicity, concurrency)
 * - Jansson: Purples (C heritage)
 * - POD types: Greys/Neutral (fundamental, universal)
 * - Overhead indicators: Reds/Pinks (warning)
 */
export const NODE_COLORS: Record<NodeType, string> = {
  // ========== HakkaJson - Greens/Teals ==========
  'hakka-registry': '#059669',
  'hakka-scalar-manager': '#10B981',
  'hakka-string-manager': '#14B8A6',
  'hakka-array-manager': '#0D9488',
  'hakka-object-manager': '#0F766E',
  'hakka-handle': '#34D399',
  'hakka-nan-boxed': '#FCD34D',        // Amber - NaN-boxed scalars
  'hakka-interned-string': '#A78BFA',  // Violet - interned PicoStrings
  'hakka-array': '#2DD4BF',            // JsonArrayCompact
  'hakka-object': '#5EEAD4',           // JsonObjectCompact

  // ========== CPython - Blues ==========
  // Infrastructure (type objects) - all marked as overhead (red tones)
  'py-type-pool': '#1E3A8A',          // Dark blue - container
  'py-type-object': '#F87171',        // Red - overhead (PyTypeObject base struct)
  'py-number-methods': '#EF4444',     // Red - overhead (PyNumberMethods)
  'py-sequence-methods': '#DC2626',   // Red - overhead (PySequenceMethods)
  'py-mapping-methods': '#B91C1C',    // Red - overhead (PyMappingMethods)
  // Container types
  'py-dict': '#3B82F6',
  'py-dict-keys': '#60A5FA',
  'py-dict-entry': '#93C5FD',
  'py-list': '#2563EB',
  'py-list-items': '#3B82F6',
  // Value types
  'py-unicode': '#1D4ED8',
  'py-long': '#1E40AF',
  'py-float': '#1E3A8A',
  'py-bool-singleton': '#60A5FA',
  'py-none-singleton': '#94A3B8',
  'py-none': '#94A3B8',               // Slate grey for None
  'py-bool': '#60A5FA',               // Same as py-bool-singleton
  // Overhead nodes
  'py-type-ptr': '#F87171',           // Red - overhead
  'py-refcount': '#FCA5A5',           // Red - overhead
  // POD-level CPython - Field-level breakdown
  // PyObject base fields
  'py-ob-refcnt': '#FCA5A5',           // Red (overhead)
  'py-ob-type': '#FCA5A5',             // Red (overhead)
  'py-ob-size': '#93C5FD',             // Light blue (metadata)

  // PyDictObject fields
  'py-dict-ma-used': '#60A5FA',        // Blue - count field
  'py-dict-ma-version': '#93C5FD',     // Light blue - metadata
  'py-dict-ma-keys-ptr': '#3B82F6',    // Blue - pointer
  'py-dict-ma-values-ptr': '#3B82F6',  // Blue - pointer

  // PyDictKeysObject fields
  'py-dict-keys-refcnt': '#FCA5A5',    // Red (overhead - refcount)
  'py-dict-keys-log2': '#BFDBFE',      // Very light blue - metadata
  'py-dict-keys-version': '#BFDBFE',   // Very light blue - metadata
  'py-dict-keys-usable': '#93C5FD',    // Light blue - metadata
  'py-dict-keys-nentries': '#60A5FA',  // Blue - count field
  'py-dict-keys-indices': '#DBEAFE',   // Very light blue - hash table
  'py-dict-keys-entries': '#3B82F6',   // Blue - entries container

  // PyDictKeyEntry fields
  'py-dict-entry-hash': '#BFDBFE',     // Light blue - hash
  'py-dict-entry-key-ptr': '#F87171',  // Red - pointer (overhead)
  'py-dict-entry-value-ptr': '#F87171', // Red - pointer (overhead)

  // PyListObject fields
  'py-list-ob-size': '#93C5FD',        // Light blue - item count
  'py-list-ob-item-ptr': '#2563EB',    // Blue - pointer
  'py-list-allocated': '#93C5FD',      // Light blue - metadata
  'py-list-items-array': '#60A5FA',    // Blue - array container
  'py-list-item-slot': '#F87171',      // Red - pointer (overhead)

  // PyLongObject fields
  'py-long-ob-size': '#93C5FD',        // Light blue - metadata
  'py-long-digit': '#60A5FA',          // Blue - actual data
  'py-lv-tag': '#93C5FD',              // Light blue - Python 3.12+

  // PyFloatObject fields
  'py-float-fval': '#FBBF24',          // Amber - numeric data

  // PyUnicodeObject fields
  'py-unicode-length': '#93C5FD',      // Light blue - metadata
  'py-unicode-hash': '#BFDBFE',        // Very light blue - cached hash
  'py-unicode-state': '#DBEAFE',       // Very light blue - flags
  'py-unicode-wstr': '#F87171',        // Red - deprecated pointer (overhead)
  'py-unicode-data': '#A78BFA',        // Violet - actual string data

  // ========== serde_json - Oranges ==========
  // Container types
  'serde-value-enum': '#F97316',       // Orange - Value enum container
  'serde-string': '#FB923C',           // Light orange - String container
  'serde-string-data': '#A78BFA',      // Violet - actual string DATA (not overhead!)
  'serde-vec': '#EA580C',
  'serde-vec-data': '#F97316',
  'serde-indexmap': '#C2410C',
  'serde-btreemap': '#C2410C',         // Same as indexmap
  'serde-map-entry': '#FB923C',
  'serde-number': '#FBBF24',           // Amber - Number container
  'serde-array': '#EA580C',            // Same as serde-vec

  // Value enum fields
  'serde-discriminant': '#F87171',     // Red - overhead (type tag)
  'serde-padding': '#FCA5A5',          // Light red - overhead (alignment)

  // String fields
  'serde-string-ptr': '#F87171',       // Red - overhead (pointer)
  'serde-string-len': '#FCA5A5',       // Light red - overhead (metadata)
  'serde-string-cap': '#FECACA',       // Very light red - overhead (capacity)

  // Number fields
  'serde-n-discriminant': '#F87171',   // Red - overhead (inner type tag)
  'serde-n-value': '#FBBF24',          // Amber - actual DATA

  // Vec fields
  'serde-vec-ptr': '#F87171',          // Red - overhead (pointer)
  'serde-vec-len': '#FCA5A5',          // Light red - overhead (metadata)
  'serde-vec-cap': '#FECACA',          // Very light red - overhead (capacity)
  'serde-vec-slot': '#FB923C',         // Orange - slot container

  // BTreeMap fields
  'serde-btree-root': '#F87171',       // Red - overhead (pointer)
  'serde-btree-len': '#FCA5A5',        // Light red - overhead (metadata)
  'serde-btree-node': '#EA580C',       // Dark orange - tree structure
  'serde-btree-leaf': '#FB923C',       // Orange - leaf container

  // LeafNode internals (~628 bytes overhead per node!)
  'serde-leafnode-parent': '#F87171',     // Red - overhead (pointer)
  'serde-leafnode-parent-idx': '#FCA5A5', // Light red - overhead
  'serde-leafnode-len': '#FCA5A5',        // Light red - overhead (metadata)
  'serde-leafnode-keys': '#FB923C',       // Orange - keys array container
  'serde-leafnode-vals': '#FB923C',       // Orange - vals array container
  'serde-leafnode-key-slot': '#FDBA74',   // Light orange - used key slot
  'serde-leafnode-val-slot': '#FDBA74',   // Light orange - used val slot
  'serde-leafnode-wasted-key': '#DC2626', // Bright red - WASTED overhead!
  'serde-leafnode-wasted-val': '#DC2626', // Bright red - WASTED overhead!

  // Legacy/POD-level Rust
  'serde-n-variant': '#FB923C',
  'rust-ptr': '#F87171',               // Red - pointer overhead
  'rust-usize': '#FDBA74',
  'rust-hash-seed': '#FCD34D',

  // ========== Go - Cyans ==========
  // Reference: runtime/runtime2.go, runtime/string.go, runtime/map.go
  //
  // Container types
  'go-interface': '#06B6D4',          // interface{} (eface) - 16 bytes
  'go-string': '#22D3EE',             // string header - 16 bytes
  'go-string-data': '#A78BFA',        // Violet - actual string DATA (not overhead!)
  'go-slice': '#0891B2',              // []interface{} header - 24 bytes
  'go-slice-data': '#06B6D4',         // backing array
  'go-hmap': '#0E7490',               // hmap header - 48 bytes
  'go-bucket': '#155E75',             // bmap - 272 bytes
  'go-float64': '#FBBF24',            // Amber - float64 DATA
  'go-bool': '#67E8F9',               // Cyan - bool DATA

  // interface{} (eface) internal fields
  'go-iface-type': '#F87171',         // Red - _type pointer (OVERHEAD)
  'go-iface-data': '#FCA5A5',         // Light red - data pointer (OVERHEAD)

  // string (stringStruct) internal fields
  'go-string-ptr': '#F87171',         // Red - str pointer (OVERHEAD)
  'go-string-len': '#FCA5A5',         // Light red - len (OVERHEAD - metadata!)

  // []interface{} slice internal fields
  'go-slice-ptr': '#F87171',          // Red - array pointer (OVERHEAD)
  'go-slice-len': '#FCA5A5',          // Light red - length (OVERHEAD - metadata!)
  'go-slice-cap': '#FECACA',          // Very light red - capacity (OVERHEAD)
  'go-slice-slot': '#22D3EE',         // Cyan - used slot
  'go-slice-wasted-slot': '#DC2626',  // Bright red - WASTED slot (OVERHEAD)

  // hmap internal fields
  'go-hmap-count': '#FCA5A5',         // Light red - count (OVERHEAD - length!)
  'go-hmap-flags': '#FECACA',         // Very light red (OVERHEAD)
  'go-hmap-b': '#FECACA',             // Very light red - B (OVERHEAD)
  'go-hmap-noverflow': '#FECACA',     // Very light red (OVERHEAD)
  'go-hmap-hash0': '#FCD34D',         // Amber - hash seed (OVERHEAD)
  'go-hmap-buckets': '#F87171',       // Red - pointer (OVERHEAD)
  'go-hmap-oldbuckets': '#F87171',    // Red - pointer (OVERHEAD)
  'go-hmap-nevacuate': '#FCA5A5',     // Light red (OVERHEAD)
  'go-hmap-extra': '#F87171',         // Red - pointer (OVERHEAD)

  // bucket (bmap) internal fields
  'go-bucket-tophash': '#67E8F9',     // Cyan - tophash container
  'go-bucket-tophash-slot': '#A5F3FC', // Light cyan - used tophash
  'go-bucket-tophash-wasted': '#DC2626', // Bright red - WASTED tophash
  'go-bucket-keys': '#A5F3FC',        // Light cyan - keys region
  'go-bucket-vals': '#CFFAFE',        // Very light cyan - vals region
  'go-bucket-overflow': '#F87171',    // Red - overflow pointer (OVERHEAD)
  'go-bucket-key-slot': '#22D3EE',    // Cyan - used key slot
  'go-bucket-val-slot': '#06B6D4',    // Cyan - used val slot
  'go-bucket-wasted-key': '#DC2626',  // Bright red - WASTED key (OVERHEAD!)
  'go-bucket-wasted-val': '#DC2626',  // Bright red - WASTED val (OVERHEAD!)

  // Legacy POD-level Go (kept for compatibility)
  'go-type-struct': '#0E7490',
  'go-type-size': '#22D3EE',
  'go-type-kind': '#67E8F9',
  'go-type-hash': '#A5F3FC',
  'go-tophash': '#67E8F9',
  'go-bucket-values': '#CFFAFE',

  // ========== Jansson - Purples ==========
  // Reference: jansson.h, jansson_private.h, hashtable.h
  //
  // json_t base (16 bytes)
  'jansson-json-t': '#8B5CF6',          // Purple - json_t container
  'jansson-type-enum': '#FCA5A5',       // Light red - OVERHEAD (type tag)
  'jansson-refcount': '#F87171',        // Red - OVERHEAD (refcount)
  'jansson-padding': '#FECACA',         // Very light red - OVERHEAD (padding)

  // json_object_t (72 bytes)
  'jansson-object': '#8B5CF6',          // Purple - container

  // hashtable_t (56 bytes)
  'jansson-hashtable': '#A78BFA',       // Light purple - container
  'jansson-ht-size': '#FCA5A5',         // Light red - OVERHEAD (count!)
  'jansson-ht-buckets-ptr': '#F87171',  // Red - OVERHEAD (pointer)
  'jansson-ht-order': '#FECACA',        // Very light red - OVERHEAD
  'jansson-ht-list-prev': '#FCA5A5',    // Light red - OVERHEAD
  'jansson-ht-list-next': '#FCA5A5',    // Light red - OVERHEAD
  'jansson-ht-ordered-prev': '#FCA5A5', // Light red - OVERHEAD
  'jansson-ht-ordered-next': '#FCA5A5', // Light red - OVERHEAD

  // hashtable_bucket (16 bytes each, 8 buckets = 128 bytes)
  'jansson-bucket': '#C4B5FD',          // Light purple - container
  'jansson-bucket-first': '#F87171',    // Red - OVERHEAD (pointer)
  'jansson-bucket-last': '#F87171',     // Red - OVERHEAD (pointer)
  'jansson-bucket-array': '#C4B5FD',    // Light purple - container

  // hashtable_pair (56+ bytes)
  'jansson-pair': '#A78BFA',            // Light purple - container
  'jansson-pair-list-prev': '#F87171',  // Red - OVERHEAD (collision)
  'jansson-pair-list-next': '#F87171',  // Red - OVERHEAD (collision)
  'jansson-pair-ordered-prev': '#FCA5A5', // Light red - OVERHEAD (order)
  'jansson-pair-ordered-next': '#FCA5A5', // Light red - OVERHEAD (order)
  'jansson-pair-hash': '#FECACA',       // Very light red - OVERHEAD (cached hash)
  'jansson-pair-value-ptr': '#F87171',  // Red - OVERHEAD (pointer)
  'jansson-pair-key-len': '#FCA5A5',    // Light red - OVERHEAD (length!)
  'jansson-pair-key-data': '#A78BFA',   // Violet - DATA (actual key bytes!)
  'jansson-pair-key-null': '#FECACA',   // Very light red - OVERHEAD (null term)

  // json_string_t (32 bytes)
  'jansson-string': '#6D28D9',          // Dark purple - container
  'jansson-string-ptr': '#F87171',      // Red - OVERHEAD (pointer)
  'jansson-string-length': '#FCA5A5',   // Light red - OVERHEAD (length!)
  'jansson-string-data': '#A78BFA',     // Violet - DATA (actual string bytes!)
  'jansson-string-null': '#FECACA',     // Very light red - OVERHEAD (null term)

  // json_integer_t (24 bytes)
  'jansson-integer': '#5B21B6',         // Dark purple - container
  'jansson-int-value': '#FBBF24',       // Amber - DATA (actual value!)

  // json_real_t (24 bytes)
  'jansson-real': '#4C1D95',            // Very dark purple - container
  'jansson-real-value': '#FBBF24',      // Amber - DATA (actual value!)

  // json_t for true/false/null (16 bytes each)
  'jansson-true': '#A78BFA',            // Light purple
  'jansson-false': '#A78BFA',           // Light purple
  'jansson-null': '#94A3B8',            // Slate grey

  // json_array_t (40 bytes)
  'jansson-array': '#7C3AED',           // Purple - container
  'jansson-array-data': '#8B5CF6',      // Purple - table container
  'jansson-array-size': '#FECACA',      // Very light red - OVERHEAD (capacity)
  'jansson-array-entries': '#FCA5A5',   // Light red - OVERHEAD (count!)
  'jansson-array-table': '#F87171',     // Red - OVERHEAD (pointer)
  'jansson-array-slot': '#F87171',      // Red - OVERHEAD (pointer per element)

  // Legacy compatibility
  'jansson-ht-buckets': '#E9D5FF',
  'jansson-list-prev': '#F5F3FF',
  'jansson-list-next': '#F5F3FF',

  // ========== POD Types - Greys/Neutral ==========
  'pod-int8': '#9CA3AF',
  'pod-uint8': '#9CA3AF',
  'pod-int16': '#6B7280',
  'pod-uint16': '#6B7280',
  'pod-int32': '#4B5563',
  'pod-uint32': '#4B5563',
  'pod-int64': '#374151',
  'pod-uint64': '#374151',
  'pod-float32': '#FCD34D',            // Amber
  'pod-float64': '#FBBF24',            // Amber
  'pod-ptr': '#F87171',                // Red (overhead)
  'pod-char-array': '#A78BFA',         // Violet
  'pod-byte-array': '#C4B5FD',
  'pod-bitfield': '#FB923C',           // Orange
};

/**
 * Node size by type (relative sizing)
 */
export const NODE_SIZES: Record<NodeType, number> = {
  // HakkaJson - Managers are large, handles are small
  'hakka-registry': 50,
  'hakka-scalar-manager': 45,
  'hakka-string-manager': 45,
  'hakka-array-manager': 45,
  'hakka-object-manager': 45,
  'hakka-handle': 12,
  'hakka-nan-boxed': 14,
  'hakka-interned-string': 18,
  'hakka-array': 22,                   // JsonArrayCompact
  'hakka-object': 20,                  // JsonObjectCompact (2 handles + refcount)

  // CPython
  // Infrastructure
  'py-type-pool': 50,         // Container for type objects (like hakka-registry)
  'py-type-object': 45,       // PyTypeObject base struct (424 bytes)
  'py-number-methods': 35,    // PyNumberMethods (280 bytes)
  'py-sequence-methods': 25,  // PySequenceMethods (80 bytes)
  'py-mapping-methods': 20,   // PyMappingMethods (24 bytes)
  // Container types
  'py-dict': 30,
  'py-dict-keys': 25,
  'py-dict-entry': 15,
  'py-list': 28,
  'py-list-items': 20,
  // Value types
  'py-unicode': 18,
  'py-long': 16,
  'py-float': 16,
  'py-bool-singleton': 14,
  'py-none-singleton': 14,
  'py-none': 14,
  'py-bool': 14,
  // Overhead nodes
  'py-type-ptr': 8,
  'py-refcount': 10,
  // POD-level CPython - Field-level nodes
  // PyObject base fields
  'py-ob-refcnt': 10,
  'py-ob-type': 10,
  'py-ob-size': 10,

  // PyDictObject fields
  'py-dict-ma-used': 10,
  'py-dict-ma-version': 10,
  'py-dict-ma-keys-ptr': 10,
  'py-dict-ma-values-ptr': 10,

  // PyDictKeysObject fields
  'py-dict-keys-refcnt': 10,
  'py-dict-keys-log2': 6,
  'py-dict-keys-version': 8,
  'py-dict-keys-usable': 10,
  'py-dict-keys-nentries': 10,
  'py-dict-keys-indices': 12,
  'py-dict-keys-entries': 14,

  // PyDictKeyEntry fields
  'py-dict-entry-hash': 10,
  'py-dict-entry-key-ptr': 10,
  'py-dict-entry-value-ptr': 10,

  // PyListObject fields
  'py-list-ob-size': 10,
  'py-list-ob-item-ptr': 10,
  'py-list-allocated': 10,
  'py-list-items-array': 14,
  'py-list-item-slot': 8,

  // PyLongObject fields
  'py-long-ob-size': 10,
  'py-long-digit': 10,
  'py-lv-tag': 8,

  // PyFloatObject fields
  'py-float-fval': 12,

  // PyUnicodeObject fields
  'py-unicode-length': 8,
  'py-unicode-hash': 10,
  'py-unicode-state': 6,
  'py-unicode-wstr': 10,
  'py-unicode-data': 14,

  // serde_json containers
  'serde-value-enum': 20,
  'serde-string': 18,
  'serde-string-data': 14,         // Actual DATA
  'serde-vec': 22,
  'serde-vec-data': 18,
  'serde-indexmap': 25,
  'serde-btreemap': 18,
  'serde-map-entry': 16,
  'serde-number': 16,
  'serde-array': 22,

  // Value enum fields
  'serde-discriminant': 10,        // Overhead
  'serde-padding': 8,              // Overhead

  // String fields
  'serde-string-ptr': 10,          // Overhead
  'serde-string-len': 10,          // Overhead
  'serde-string-cap': 10,          // Overhead

  // Number fields
  'serde-n-discriminant': 10,      // Overhead
  'serde-n-value': 12,             // DATA

  // Vec fields
  'serde-vec-ptr': 10,             // Overhead
  'serde-vec-len': 10,             // Overhead
  'serde-vec-cap': 10,             // Overhead
  'serde-vec-slot': 10,            // Overhead

  // BTreeMap fields
  'serde-btree-root': 10,          // Overhead
  'serde-btree-len': 10,           // Overhead
  'serde-btree-node': 16,
  'serde-btree-leaf': 14,

  // LeafNode internals
  'serde-leafnode-parent': 10,        // Overhead
  'serde-leafnode-parent-idx': 6,     // Overhead
  'serde-leafnode-len': 6,            // Overhead
  'serde-leafnode-keys': 16,          // Container
  'serde-leafnode-vals': 16,          // Container
  'serde-leafnode-key-slot': 10,      // Used slot
  'serde-leafnode-val-slot': 10,      // Used slot
  'serde-leafnode-wasted-key': 12,    // WASTED - bright red!
  'serde-leafnode-wasted-val': 14,    // WASTED - bright red!

  // Legacy POD-level Rust
  'serde-n-variant': 12,
  'rust-ptr': 10,
  'rust-usize': 10,
  'rust-hash-seed': 12,

  // Go - Container types
  'go-interface': 16,
  'go-string': 16,
  'go-string-data': 14,            // DATA
  'go-slice': 20,
  'go-slice-data': 16,
  'go-hmap': 30,
  'go-bucket': 35,
  'go-float64': 14,                // DATA
  'go-bool': 14,                   // DATA

  // interface{} (eface) fields
  'go-iface-type': 10,             // Overhead
  'go-iface-data': 10,             // Overhead

  // string (stringStruct) fields
  'go-string-ptr': 10,             // Overhead
  'go-string-len': 10,             // Overhead

  // []interface{} slice fields
  'go-slice-ptr': 10,              // Overhead
  'go-slice-len': 10,              // Overhead
  'go-slice-cap': 10,              // Overhead
  'go-slice-slot': 10,             // Used slot
  'go-slice-wasted-slot': 12,      // WASTED - bright red!

  // hmap fields
  'go-hmap-count': 10,             // Overhead - length!
  'go-hmap-flags': 6,              // Overhead
  'go-hmap-b': 6,                  // Overhead
  'go-hmap-noverflow': 6,          // Overhead
  'go-hmap-hash0': 8,              // Overhead
  'go-hmap-buckets': 10,           // Overhead - pointer
  'go-hmap-oldbuckets': 10,        // Overhead - pointer
  'go-hmap-nevacuate': 10,         // Overhead
  'go-hmap-extra': 10,             // Overhead - pointer

  // bucket (bmap) fields
  'go-bucket-tophash': 12,         // Container
  'go-bucket-tophash-slot': 6,     // Used tophash
  'go-bucket-tophash-wasted': 6,   // WASTED tophash
  'go-bucket-keys': 16,            // Container
  'go-bucket-vals': 16,            // Container
  'go-bucket-overflow': 10,        // Overhead - pointer
  'go-bucket-key-slot': 10,        // Used key slot
  'go-bucket-val-slot': 10,        // Used val slot
  'go-bucket-wasted-key': 12,      // WASTED - bright red!
  'go-bucket-wasted-val': 14,      // WASTED - bright red!

  // Legacy POD-level Go
  'go-type-struct': 20,
  'go-type-size': 10,
  'go-type-kind': 8,
  'go-type-hash': 10,
  'go-tophash': 12,
  'go-bucket-values': 16,

  // Jansson - Container types
  'jansson-json-t': 16,              // json_t base
  'jansson-type-enum': 8,            // Overhead
  'jansson-refcount': 10,            // Overhead
  'jansson-padding': 6,              // Overhead

  'jansson-object': 28,              // json_object_t
  'jansson-hashtable': 25,           // hashtable_t
  'jansson-ht-size': 10,             // Overhead - count!
  'jansson-ht-buckets-ptr': 10,      // Overhead - pointer
  'jansson-ht-order': 8,             // Overhead
  'jansson-ht-list-prev': 10,        // Overhead
  'jansson-ht-list-next': 10,        // Overhead
  'jansson-ht-ordered-prev': 10,     // Overhead
  'jansson-ht-ordered-next': 10,     // Overhead

  'jansson-bucket': 12,              // bucket_t
  'jansson-bucket-first': 10,        // Overhead - pointer
  'jansson-bucket-last': 10,         // Overhead - pointer
  'jansson-bucket-array': 20,        // Container (128 bytes!)

  'jansson-pair': 18,                // hashtable_pair
  'jansson-pair-list-prev': 10,      // Overhead
  'jansson-pair-list-next': 10,      // Overhead
  'jansson-pair-ordered-prev': 10,   // Overhead
  'jansson-pair-ordered-next': 10,   // Overhead
  'jansson-pair-hash': 10,           // Overhead
  'jansson-pair-value-ptr': 10,      // Overhead - pointer
  'jansson-pair-key-len': 10,        // Overhead - length!
  'jansson-pair-key-data': 12,       // DATA
  'jansson-pair-key-null': 6,        // Overhead

  'jansson-string': 18,              // json_string_t
  'jansson-string-ptr': 10,          // Overhead - pointer
  'jansson-string-length': 10,       // Overhead - length!
  'jansson-string-data': 14,         // DATA
  'jansson-string-null': 6,          // Overhead

  'jansson-integer': 16,             // json_integer_t
  'jansson-int-value': 12,           // DATA

  'jansson-real': 16,                // json_real_t
  'jansson-real-value': 12,          // DATA

  'jansson-true': 14,                // json_t
  'jansson-false': 14,               // json_t
  'jansson-null': 14,                // json_t

  'jansson-array': 26,               // json_array_t
  'jansson-array-data': 18,          // table container
  'jansson-array-size': 8,           // Overhead - capacity
  'jansson-array-entries': 10,       // Overhead - count!
  'jansson-array-table': 10,         // Overhead - pointer
  'jansson-array-slot': 8,           // Overhead - pointer

  // Legacy compatibility
  'jansson-ht-buckets': 10,
  'jansson-list-prev': 8,
  'jansson-list-next': 8,

  // POD Types
  'pod-int8': 6,
  'pod-uint8': 6,
  'pod-int16': 8,
  'pod-uint16': 8,
  'pod-int32': 10,
  'pod-uint32': 10,
  'pod-int64': 12,
  'pod-uint64': 12,
  'pod-float32': 10,
  'pod-float64': 12,
  'pod-ptr': 10,
  'pod-char-array': 14,
  'pod-byte-array': 12,
  'pod-bitfield': 8,
};

/**
 * Relationship colors
 */
export const RELATIONSHIP_COLORS: Partial<Record<RelationshipType, string>> = {
  // HakkaJson - Green tones
  'registry-manages': '#059669',
  'manager-stores': '#10B981',
  'handle-resolves': '#34D399',
  'handle-indexes': '#6EE7B7',
  'value-contains': '#14B8A6',
  'string-interned-at': '#A78BFA',

  // CPython - Blue tones
  'dict-to-keys': '#3B82F6',
  'keys-to-entry': '#60A5FA',
  'entry-key': '#93C5FD',
  'entry-value': '#93C5FD',
  'list-to-items': '#2563EB',
  'items-element': '#60A5FA',
  'ob-type': '#F87171',  // Red - overhead
  // CPython deep
  'pyobj-refcnt': '#FCA5A5',
  'pyobj-type': '#FCA5A5',
  'pylong-lvtag': '#93C5FD',
  'pyobj-size': '#93C5FD',
  'pylong-digits': '#60A5FA',
  'digit-value': '#60A5FA',
  'pyfloat-fval': '#FBBF24',
  'pyunicode-length': '#93C5FD',
  'pyunicode-hash': '#BFDBFE',
  'pyunicode-state': '#DBEAFE',
  'pyunicode-data': '#A78BFA',
  'char-at': '#C4B5FD',

  // Rust - Orange tones
  'enum-contains': '#F97316',
  'string-to-heap': '#FB923C',
  'vec-to-data': '#EA580C',
  'vec-element': '#FB923C',
  'map-to-entry': '#C2410C',
  'map-entry-key': '#FDBA74',
  'map-entry-value': '#FDBA74',
  // Rust deep
  'enum-discriminant': '#FDBA74',
  'number-to-n': '#FB923C',
  'n-to-payload': '#FBBF24',
  'string-ptr': '#FB923C',
  'string-len': '#FDBA74',
  'string-cap': '#FDBA74',
  'ptr-to-bytes': '#FCD34D',
  'indexmap-indices': '#C2410C',
  'indexmap-entries': '#EA580C',
  'bucket-hash': '#FDBA74',
  'bucket-key': '#FB923C',
  'bucket-value': '#FB923C',

  // Go - Cyan tones
  'iface-type': '#F87171',  // Red - overhead
  'iface-data': '#06B6D4',
  'string-to-bytes': '#22D3EE',
  'slice-to-data': '#0891B2',
  'slice-element': '#22D3EE',
  'hmap-to-buckets': '#0E7490',
  'bucket-overflow': '#155E75',
  'bucket-entry': '#67E8F9',
  // Go deep
  'iface-type-ptr': '#F87171',
  'iface-data-ptr': '#06B6D4',
  'type-to-size': '#22D3EE',
  'type-to-kind': '#67E8F9',
  'type-to-hash': '#A5F3FC',
  'hmap-count': '#22D3EE',
  'hmap-b': '#67E8F9',
  'hmap-hash0': '#FCD34D',
  'hmap-buckets-ptr': '#0E7490',
  'bmap-tophash': '#67E8F9',
  'bmap-keys': '#A5F3FC',
  'bmap-values': '#CFFAFE',
  'bmap-overflow': '#155E75',
  'stringhdr-ptr': '#22D3EE',
  'stringhdr-len': '#67E8F9',

  // Jansson - Purple tones
  'object-to-hashtable': '#8B5CF6',
  'hashtable-to-pair': '#A78BFA',
  'pair-chain': '#C4B5FD',
  'pair-to-key': '#DDD6FE',
  'pair-to-value': '#DDD6FE',
  'array-to-elements': '#7C3AED',
  'element-ptr': '#A78BFA',
  // Jansson deep
  'json-t-to-type': '#C4B5FD',
  'json-t-to-refcount': '#FCA5A5',
  'integer-to-value': '#DDD6FE',
  'real-to-value': '#FBBF24',
  'jstring-to-ptr': '#C4B5FD',
  'jstring-to-len': '#DDD6FE',
  'jstring-ptr-to-data': '#A78BFA',
  'jarray-to-size': '#C4B5FD',
  'jarray-to-entries': '#DDD6FE',
  'jarray-to-table': '#E9D5FF',
  'jarray-table-to-element': '#A78BFA',
  'ht-to-size': '#C4B5FD',
  'ht-to-order': '#DDD6FE',
  'ht-to-buckets': '#E9D5FF',
  'pair-to-hash': '#DDD6FE',
  'pair-to-key-len': '#DDD6FE',
  'pair-to-key-data': '#A78BFA',
  'pair-to-list-prev': '#F5F3FF',
  'pair-to-list-next': '#F5F3FF',
  'pair-to-ordered-prev': '#F5F3FF',
  'pair-to-ordered-next': '#F5F3FF',

  // Generic
  'struct-field': '#6B7280',
  'field-value': '#9CA3AF',
  'array-index': '#4B5563',
  'pointer-deref': '#F87171',
};

/**
 * Default NVL layout options
 */
export const DEFAULT_LAYOUT_OPTIONS = {
  layout: 'forceDirected' as const,
  initialZoom: 0.8,
  nodeSpacing: 50,
};

/**
 * Legend categories for display
 */
export const LEGEND_CATEGORIES = {
  hakka: {
    label: 'HakkaJson (Efficient)',
    color: '#10B981',
    items: [
      { type: 'hakka-registry', label: 'Registry' },
      { type: 'hakka-handle', label: 'Handle (4 bytes)' },
      { type: 'hakka-nan-boxed', label: 'NaN-boxed (8 bytes)' },
      { type: 'hakka-interned-string', label: 'Interned String' },
    ],
  },
  traditional: {
    label: 'Traditional (Overhead)',
    color: '#EF4444',
    items: [
      { type: 'py-type-ptr', label: 'Type Pointer' },
      { type: 'go-interface', label: 'interface{} wrapper' },
      { type: 'jansson-pair', label: 'Hash Pair (56 bytes)' },
    ],
  },
};

/**
 * CSS custom properties for theming
 */
export const CSS_VARIABLES = {
  // HakkaJson
  '--hakka-primary': '#10B981',
  '--hakka-secondary': '#14B8A6',
  '--hakka-accent': '#FCD34D',

  // CPython
  '--cpython-primary': '#3B82F6',
  '--cpython-secondary': '#60A5FA',

  // serde_json
  '--serde-primary': '#F97316',
  '--serde-secondary': '#FB923C',

  // Go
  '--go-primary': '#06B6D4',
  '--go-secondary': '#22D3EE',

  // Jansson
  '--jansson-primary': '#8B5CF6',
  '--jansson-secondary': '#A78BFA',

  // Overhead
  '--overhead-primary': '#F87171',
  '--overhead-secondary': '#FCA5A5',

  // POD
  '--pod-int': '#4B5563',
  '--pod-float': '#FBBF24',
  '--pod-ptr': '#F87171',
};
