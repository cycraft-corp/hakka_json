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
  'hakka-owned-pointer': '#6EE7B7',
  'hakka-freelist': '#6B7280',
  'hakka-nan-boxed': '#FCD34D',        // Amber - special
  'hakka-interned-string': '#A78BFA',  // Violet - shared
  'hakka-array-value': '#2DD4BF',
  'hakka-object-value': '#5EEAD4',
  'hakka-array-storage': '#2DD4BF',
  'hakka-object-storage': '#5EEAD4',
  // POD-level HakkaJson
  'hakka-handle-type-bits': '#34D399',
  'hakka-handle-index-bits': '#6EE7B7',
  'hakka-ptr-type-tag': '#10B981',
  'hakka-ptr-address': '#6EE7B7',
  'hakka-primitive-value': '#FCD34D',
  'hakka-primitive-refcount': '#FCA5A5',  // Red (overhead)
  'hakka-int-data': '#FCD34D',
  'hakka-float-data': '#FBBF24',
  'hakka-nan-pattern': '#F59E0B',
  'hakka-string-proxy': '#A78BFA',
  'hakka-string-ptr': '#C4B5FD',
  'hakka-string-len': '#DDD6FE',

  // ========== CPython - Blues ==========
  'py-dict': '#3B82F6',
  'py-dict-keys': '#60A5FA',
  'py-dict-entry': '#93C5FD',
  'py-list': '#2563EB',
  'py-list-items': '#3B82F6',
  'py-unicode': '#1D4ED8',
  'py-long': '#1E40AF',
  'py-float': '#1E3A8A',
  'py-bool-singleton': '#60A5FA',
  'py-none-singleton': '#94A3B8',
  'py-type-ptr': '#F87171',            // Red - overhead
  'py-none': '#94A3B8',               // Slate grey for None
  'py-bool': '#60A5FA',               // Same as py-bool-singleton
  'py-refcount': '#FCA5A5',           // Red - overhead
  // POD-level CPython
  'py-ob-refcnt': '#FCA5A5',           // Red (overhead)
  'py-ob-type': '#FCA5A5',             // Red (overhead)
  'py-lv-tag': '#93C5FD',
  'py-ob-size': '#93C5FD',
  'py-long-digit': '#60A5FA',
  'py-float-fval': '#FBBF24',          // Amber
  'py-unicode-length': '#93C5FD',
  'py-unicode-hash': '#BFDBFE',
  'py-unicode-state': '#DBEAFE',
  'py-unicode-data': '#A78BFA',        // Violet (string data)

  // ========== serde_json - Oranges ==========
  'serde-value-enum': '#F97316',
  'serde-string': '#FB923C',
  'serde-string-data': '#FDBA74',
  'serde-vec': '#EA580C',
  'serde-vec-data': '#F97316',
  'serde-indexmap': '#C2410C',
  'serde-map-entry': '#FB923C',
  'serde-number': '#FBBF24',
  'serde-array': '#EA580C',           // Same as serde-vec
  // POD-level Rust
  'serde-discriminant': '#FDBA74',
  'serde-n-variant': '#FB923C',
  'rust-ptr': '#F87171',               // Red
  'rust-usize': '#FDBA74',
  'rust-hash-seed': '#FCD34D',

  // ========== Go - Cyans ==========
  'go-interface': '#06B6D4',
  'go-string': '#22D3EE',
  'go-string-data': '#67E8F9',
  'go-slice': '#0891B2',
  'go-slice-data': '#06B6D4',
  'go-hmap': '#0E7490',
  'go-bucket': '#155E75',
  'go-float64': '#FBBF24',             // Amber - all numbers
  'go-bool': '#67E8F9',               // Cyan - boolean
  // POD-level Go
  'go-type-struct': '#0E7490',
  'go-type-size': '#22D3EE',
  'go-type-kind': '#67E8F9',
  'go-type-hash': '#A5F3FC',
  'go-hmap-count': '#22D3EE',
  'go-hmap-b': '#67E8F9',
  'go-hmap-hash0': '#FCD34D',
  'go-tophash': '#67E8F9',
  'go-bucket-keys': '#A5F3FC',
  'go-bucket-values': '#CFFAFE',
  'go-string-len': '#67E8F9',

  // ========== Jansson - Purples ==========
  'jansson-object': '#8B5CF6',
  'jansson-hashtable': '#A78BFA',
  'jansson-pair': '#C4B5FD',
  'jansson-array': '#7C3AED',
  'jansson-array-data': '#8B5CF6',
  'jansson-string': '#6D28D9',
  'jansson-string-data': '#7C3AED',
  'jansson-integer': '#5B21B6',
  'jansson-real': '#4C1D95',
  'jansson-true': '#A78BFA',
  'jansson-false': '#A78BFA',
  'jansson-null': '#94A3B8',
  // POD-level Jansson
  'jansson-type-enum': '#C4B5FD',
  'jansson-refcount': '#FCA5A5',       // Red (overhead)
  'jansson-int-value': '#DDD6FE',
  'jansson-real-value': '#FBBF24',     // Amber
  'jansson-string-ptr': '#C4B5FD',
  'jansson-string-length': '#DDD6FE',
  'jansson-array-size': '#C4B5FD',
  'jansson-array-entries': '#DDD6FE',
  'jansson-array-table': '#E9D5FF',
  'jansson-ht-size': '#C4B5FD',
  'jansson-ht-order': '#DDD6FE',
  'jansson-ht-buckets': '#E9D5FF',
  'jansson-pair-hash': '#DDD6FE',
  'jansson-pair-value-ptr': '#E9D5FF',
  'jansson-pair-key-len': '#DDD6FE',
  'jansson-pair-key-data': '#A78BFA',
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
  'hakka-scalar-manager': 40,
  'hakka-string-manager': 40,
  'hakka-array-manager': 40,
  'hakka-object-manager': 40,
  'hakka-handle': 12,
  'hakka-owned-pointer': 15,
  'hakka-freelist': 10,
  'hakka-nan-boxed': 14,
  'hakka-interned-string': 18,
  'hakka-array-value': 25,
  'hakka-object-value': 25,
  'hakka-array-storage': 22,
  'hakka-object-storage': 22,
  // POD-level HakkaJson
  'hakka-handle-type-bits': 8,
  'hakka-handle-index-bits': 10,
  'hakka-ptr-type-tag': 8,
  'hakka-ptr-address': 10,
  'hakka-primitive-value': 12,
  'hakka-primitive-refcount': 10,
  'hakka-int-data': 12,
  'hakka-float-data': 12,
  'hakka-nan-pattern': 10,
  'hakka-string-proxy': 14,
  'hakka-string-ptr': 10,
  'hakka-string-len': 8,

  // CPython
  'py-dict': 30,
  'py-dict-keys': 25,
  'py-dict-entry': 15,
  'py-list': 28,
  'py-list-items': 20,
  'py-unicode': 18,
  'py-long': 16,
  'py-float': 16,
  'py-bool-singleton': 14,
  'py-none-singleton': 14,
  'py-type-ptr': 8,
  'py-none': 14,
  'py-bool': 14,
  'py-refcount': 10,
  // POD-level CPython
  'py-ob-refcnt': 10,
  'py-ob-type': 10,
  'py-lv-tag': 8,
  'py-ob-size': 8,
  'py-long-digit': 10,
  'py-float-fval': 12,
  'py-unicode-length': 8,
  'py-unicode-hash': 10,
  'py-unicode-state': 6,
  'py-unicode-data': 14,

  // serde_json
  'serde-value-enum': 20,
  'serde-string': 18,
  'serde-string-data': 15,
  'serde-vec': 22,
  'serde-vec-data': 18,
  'serde-indexmap': 25,
  'serde-map-entry': 16,
  'serde-number': 16,
  'serde-array': 22,
  // POD-level Rust
  'serde-discriminant': 8,
  'serde-n-variant': 12,
  'rust-ptr': 10,
  'rust-usize': 10,
  'rust-hash-seed': 12,

  // Go
  'go-interface': 16,
  'go-string': 16,
  'go-string-data': 14,
  'go-slice': 20,
  'go-slice-data': 16,
  'go-hmap': 30,
  'go-bucket': 35,
  'go-float64': 14,
  'go-bool': 14,
  // POD-level Go
  'go-type-struct': 20,
  'go-type-size': 10,
  'go-type-kind': 8,
  'go-type-hash': 10,
  'go-hmap-count': 10,
  'go-hmap-b': 8,
  'go-hmap-hash0': 10,
  'go-tophash': 12,
  'go-bucket-keys': 16,
  'go-bucket-values': 16,
  'go-string-len': 8,

  // Jansson
  'jansson-object': 28,
  'jansson-hashtable': 25,
  'jansson-pair': 18,
  'jansson-array': 26,
  'jansson-array-data': 18,
  'jansson-string': 18,
  'jansson-string-data': 14,
  'jansson-integer': 16,
  'jansson-real': 16,
  'jansson-true': 14,
  'jansson-false': 14,
  'jansson-null': 14,
  // POD-level Jansson
  'jansson-type-enum': 8,
  'jansson-refcount': 10,
  'jansson-int-value': 12,
  'jansson-real-value': 12,
  'jansson-string-ptr': 10,
  'jansson-string-length': 8,
  'jansson-array-size': 8,
  'jansson-array-entries': 8,
  'jansson-array-table': 10,
  'jansson-ht-size': 8,
  'jansson-ht-order': 6,
  'jansson-ht-buckets': 10,
  'jansson-pair-hash': 10,
  'jansson-pair-value-ptr': 10,
  'jansson-pair-key-len': 8,
  'jansson-pair-key-data': 12,
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
  'pointer-wraps': '#A7F3D0',
  'value-contains': '#14B8A6',
  'string-interned-at': '#A78BFA',
  // HakkaJson deep
  'handle-to-type-bits': '#34D399',
  'handle-to-index-bits': '#6EE7B7',
  'ptr-to-type-tag': '#10B981',
  'ptr-to-address': '#6EE7B7',
  'primitive-to-refcount': '#FCA5A5',
  'primitive-to-value': '#FCD34D',
  'int-to-data': '#FCD34D',
  'float-to-data': '#FBBF24',
  'float-to-nan-bits': '#F59E0B',
  'string-to-proxy': '#A78BFA',
  'proxy-to-ptr': '#C4B5FD',
  'proxy-to-len': '#DDD6FE',

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
