import type { StringTracker, StringStats, ValueTracker } from '../types';

/**
 * Calculate PicoString size based on string length.
 * HakkaJson uses tiered fixed-size storage with size encoded in pointer's lower 3 bits.
 */
function getPicoStringSize(len: number): number {
  if (len <= 1) return 1;   // PicoString1
  if (len <= 2) return 2;   // PicoString2
  if (len <= 4) return 4;   // PicoString4
  if (len <= 8) return 8;   // PicoString8
  if (len <= 16) return 16; // PicoString16
  if (len <= 32) return 32; // PicoString32
  if (len <= 64) return 64; // PicoString64
  return 24 + len;          // Fallback to std::string for len > 64
}

/**
 * Tracks strings for interning simulation
 * Used by HakkaJson to share string references
 */
export class InternedStringTracker implements StringTracker {
  private strings = new Map<string, string>();
  private accessCounts = new Map<string, number>();
  private idGenerator: () => string;

  constructor(idGenerator: () => string) {
    this.idGenerator = idGenerator;
  }

  track(value: string): { nodeId: string; isNew: boolean } {
    const existing = this.strings.get(value);

    if (existing) {
      // Increment access count for duplicate tracking
      this.accessCounts.set(value, (this.accessCounts.get(value) ?? 1) + 1);
      return { nodeId: existing, isNew: false };
    }

    // New string - generate ID and store
    const nodeId = this.idGenerator();
    this.strings.set(value, nodeId);
    this.accessCounts.set(value, 1);
    return { nodeId, isNew: true };
  }

  getStrings(): Map<string, string> {
    return new Map(this.strings);
  }

  getStats(): StringStats {
    let totalStrings = 0;
    let duplicateStrings = 0;
    let bytesSaved = 0;

    for (const [value, count] of this.accessCounts) {
      totalStrings += count;
      if (count > 1) {
        duplicateStrings += count - 1;
        // Each duplicate saves the PicoString size
        bytesSaved += (count - 1) * ((value.length + 24) - getPicoStringSize(value.length));
      }
    }

    return {
      totalStrings,
      uniqueStrings: this.strings.size,
      duplicateStrings,
      bytesSaved,
    };
  }

  reset(): void {
    this.strings.clear();
    this.accessCounts.clear();
  }
}

/**
 * Non-interning tracker - creates new ID for every string
 * Used by implementations that don't intern strings
 */
export class DuplicatingStringTracker implements StringTracker {
  private strings = new Map<string, string[]>();
  private idGenerator: () => string;

  constructor(idGenerator: () => string) {
    this.idGenerator = idGenerator;
  }

  track(value: string): { nodeId: string; isNew: boolean } {
    const nodeId = this.idGenerator();
    const existing = this.strings.get(value);

    if (existing) {
      existing.push(nodeId);
    } else {
      this.strings.set(value, [nodeId]);
    }

    // Always "new" - each string gets its own allocation
    return { nodeId, isNew: true };
  }

  getStrings(): Map<string, string> {
    // Return first ID for each string value
    const result = new Map<string, string>();
    for (const [value, ids] of this.strings) {
      if (ids.length > 0) {
        result.set(value, ids[0]);
      }
    }
    return result;
  }

  getStats(): StringStats {
    let totalStrings = 0;
    let duplicateStrings = 0;

    for (const [, ids] of this.strings) {
      totalStrings += ids.length;
      if (ids.length > 1) {
        duplicateStrings += ids.length - 1;
      }
    }

    return {
      totalStrings,
      uniqueStrings: this.strings.size,
      duplicateStrings,
      bytesSaved: 0, // No interning = no savings
    };
  }

  reset(): void {
    this.strings.clear();
  }
}

/**
 * Converts a value to a unique string key for tracking
 * Handles strings, numbers, booleans, and null
 */
function valueToKey(value: unknown): string {
  if (value === null) return 'null:null';
  if (typeof value === 'boolean') return `bool:${value}`;
  if (typeof value === 'number') return `num:${value}`;
  if (typeof value === 'string') return `str:${value}`;
  // For non-primitive values, return unique key (won't match)
  return `obj:${Math.random()}`;
}

/**
 * Tracks all immutable values (strings, numbers, booleans, null) for duplicate detection
 * Used by non-interning implementations to mark duplicate allocations
 */
export class ImmutableValueTracker implements ValueTracker {
  private seenValues = new Set<string>();

  isDuplicate(value: unknown): boolean {
    const key = valueToKey(value);
    return this.seenValues.has(key);
  }

  markSeen(value: unknown): void {
    const key = valueToKey(value);
    this.seenValues.add(key);
  }

  reset(): void {
    this.seenValues.clear();
  }
}
