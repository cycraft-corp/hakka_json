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
  // HakkaJson
  'hakka-registry': 64,
  'hakka-scalar-manager': 32,
  'hakka-string-manager': 48,
  'hakka-array-manager': 40,
  'hakka-object-manager': 56,
  'hakka-handle': 4,  // 4-byte token (type bits + index bits), NOT a pointer
  'hakka-nan-boxed': 8,
  // PicoString: tiered fixed-size storage with size encoded in pointer's lower 3 bits
  // No separate length field - extremely compact for short strings
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
  'hakka-array-storage': (count: number) => 16 + count * 8,
  'hakka-object-storage': (count: number) => 32 + count * 16,

  // serde_json
  'serde-value-enum': 32,
  'serde-string': (len: number) => 24 + len,
  'serde-number': 16,
  'serde-bool': 1,
  'serde-null': 0,
  'serde-array': (count: number) => 24 + count * 8,
  'serde-indexmap': (count: number) => 48 + count * 32,

  // CPython
  'py-dict': 232,
  'py-list': 56,
  'py-unicode': (len: number) => 48 + len,
  'py-long': 28,
  'py-float': 24,
  'py-bool': 28,
  'py-none': 16,
  'py-type-ptr': 8,
  'py-refcount': 8,

  // Go
  'go-interface': 16,
  'go-string': (len: number) => 16 + len,
  'go-float64': 8,
  'go-bool': 1,
  'go-slice': (count: number) => 24 + count * 16,
  'go-hmap': 48,
  'go-bucket': (count: number) => 8 + count * 24,

  // Jansson
  'jansson-object': 32,
  'jansson-array': (count: number) => 24 + count * 8,
  'jansson-string': (len: number) => 24 + len,
  'jansson-integer': 16,
  'jansson-real': 16,
  'jansson-true': 8,
  'jansson-false': 8,
  'jansson-null': 8,
  'jansson-hashtable': (count: number) => 40 + count * 24,
  'jansson-pair': 24,
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
