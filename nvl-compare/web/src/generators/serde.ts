import type { ImplementationId, NvlNode } from '../types/graph';
import type { GenerationResult, StringTracker, IdGenerator } from './types';
import { BaseGenerator } from './base';
import { SequentialIdGenerator } from './utils/id-generator';
import { DuplicatingStringTracker } from './utils/string-tracker';

/**
 * serde_json Graph Generator (Rust) - Field-Level Scientific Breakdown
 *
 * Memory Layout (64-bit, serde_json without preserve_order feature):
 *
 * Value enum (32 bytes):
 * - discriminant [8B] - OVERHEAD (type tag, aligned)
 * - payload [24B max] - varies by variant
 *
 * String (24 bytes stack + heap):
 * - ptr [8B] - OVERHEAD (pointer to heap)
 * - len [8B] - OVERHEAD (metadata)
 * - cap [8B] - OVERHEAD (over-allocation)
 * - heap: [u8; len] - DATA (actual UTF-8 bytes)
 *
 * Number (16 bytes):
 * - N discriminant [8B] - OVERHEAD (PosInt/NegInt/Float tag)
 * - value [8B] - DATA (u64/i64/f64)
 *
 * Vec<Value> (24 bytes stack + heap):
 * - ptr [8B] - OVERHEAD (pointer to heap)
 * - len [8B] - OVERHEAD (metadata)
 * - cap [8B] - OVERHEAD (over-allocation)
 * - heap: [Value; cap] - each Value is 32B OVERHEAD (slots)
 *
 * BTreeMap<String, Value> (16 bytes stack + heap nodes):
 * - root [8B] - OVERHEAD (pointer to root node)
 * - length [8B] - OVERHEAD (metadata)
 *
 * ONLY actual DATA (not overhead):
 * - String bytes on heap
 * - Numeric values (8B u64/i64/f64)
 * - Boolean (1 bit conceptually)
 */
export class SerdeJsonGenerator extends BaseGenerator {
  readonly id: ImplementationId = 'serde_json';
  readonly name = 'serde_json';
  protected readonly implementation = 'serde' as const;

  protected getLanguage(): string {
    return 'Rust';
  }

  protected getDescription(): string {
    return 'Value enum, ownership model, no GC';
  }

  protected createStringTracker(idGen: () => string): StringTracker {
    // serde_json doesn't intern strings
    return new DuplicatingStringTracker(idGen);
  }

  protected createIdGenerator(): IdGenerator {
    return new SequentialIdGenerator('serde-');
  }

  protected generateInfrastructure(): void {
    // Rust has no runtime infrastructure like Python's type objects
    // The Value enum discriminant IS the type information
  }

  protected generateValue(
    value: unknown,
    path: string,
    parentId?: string
  ): GenerationResult {
    const type = this.getJsonType(value);

    switch (type) {
      case 'null':
        return this.generateNull(path, parentId);
      case 'boolean':
        return this.generateBool(value as boolean, path, parentId);
      case 'number':
        return this.generateNumber(value as number, path, parentId);
      case 'string':
        return this.generateString(value as string, path, parentId);
      case 'array':
        return this.generateArray(value as unknown[], path, parentId);
      case 'object':
        return this.generateObject(value as Record<string, unknown>, path, parentId);
    }
  }

  /**
   * Value::Null (32 bytes, all overhead)
   * - discriminant [8B] - tag = 0
   * - padding [24B] - unused payload space
   */
  private generateNull(path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate
    const isDup = this.context.valueTracker.isDuplicate(null);
    this.context.valueTracker.markSeen(null);

    // Value::Null container
    const valueId = idGen.nodeId('val');
    this.createAndAddNode(valueId, 'serde-value-enum', 'Null', {
      caption: isDup ? `Value::Null @ ${path} (DUP!)` : `Value::Null @ ${path}`,
      sizeBytes: 0,  // Size in children
      isDuplicate: isDup,
      implementation: this.implementation,
    });

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, valueId, 'contains');
    }

    // discriminant [8B] - OVERHEAD
    const discId = idGen.nodeId('disc');
    this.createAndAddNode(discId, 'serde-discriminant', 'discriminant', {
      caption: 'discriminant = 0 (8B, type overhead)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, discId, 'struct-field');

    // padding [24B] - OVERHEAD (unused payload space)
    const padId = idGen.nodeId('pad');
    this.createAndAddNode(padId, 'serde-padding', 'padding', {
      caption: 'padding (24B, alignment overhead)',
      sizeBytes: 24,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, padId, 'struct-field');

    return { rootNodeId: valueId, nodes, relationships: [] };
  }

  /**
   * Value::Bool (32 bytes)
   * - discriminant [8B] - tag = 1 (OVERHEAD)
   * - bool [1B] - DATA (but marked overhead due to enum wrapper waste)
   * - padding [23B] - OVERHEAD
   */
  private generateBool(value: boolean, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // Value::Bool container
    const valueId = idGen.nodeId('val');
    this.createAndAddNode(valueId, 'serde-value-enum', String(value), {
      caption: isDup ? `Value::Bool @ ${path} (DUP!)` : `Value::Bool @ ${path}`,
      sizeBytes: 0,
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, valueId, 'contains');
    }

    // discriminant [8B] - OVERHEAD
    const discId = idGen.nodeId('disc');
    this.createAndAddNode(discId, 'serde-discriminant', 'discriminant', {
      caption: 'discriminant = 1 (8B, type overhead)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, discId, 'struct-field');

    // bool value [1B] - This 1 byte IS data, but 23 bytes are wasted padding
    // For simplicity, we show it as 1B data + 23B padding
    const boolId = idGen.nodeId('bool');
    this.createAndAddNode(boolId, 'serde-n-value', 'bool', {
      caption: `${value} (1B data)`,
      sizeBytes: 1,
      isOverhead: false,  // This IS the data
      value,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, boolId, 'struct-field');

    // padding [23B] - OVERHEAD
    const padId = idGen.nodeId('pad');
    this.createAndAddNode(padId, 'serde-padding', 'padding', {
      caption: 'padding (23B, alignment overhead)',
      sizeBytes: 23,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, padId, 'struct-field');

    return { rootNodeId: valueId, nodes, relationships: [] };
  }

  /**
   * Value::Number (32 bytes)
   * - discriminant [8B] - tag = 2 (OVERHEAD)
   * - Number [16B]:
   *   - N discriminant [8B] - PosInt/NegInt/Float (OVERHEAD)
   *   - value [8B] - u64/i64/f64 (DATA)
   * - padding [8B] - OVERHEAD
   */
  private generateNumber(value: number, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // Value::Number container
    const valueId = idGen.nodeId('val');
    this.createAndAddNode(valueId, 'serde-value-enum', String(value), {
      caption: isDup ? `Value::Number @ ${path} (DUP!)` : `Value::Number @ ${path}`,
      sizeBytes: 0,
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, valueId, 'contains');
    }

    // Value discriminant [8B] - OVERHEAD
    const discId = idGen.nodeId('disc');
    this.createAndAddNode(discId, 'serde-discriminant', 'discriminant', {
      caption: 'discriminant = 2 (8B, type overhead)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, discId, 'struct-field');

    // Number container
    const numId = idGen.nodeId('num');
    const numType = Number.isInteger(value) ? (value >= 0 ? 'PosInt' : 'NegInt') : 'Float';
    this.createAndAddNode(numId, 'serde-number', 'Number', {
      caption: `Number::${numType}`,
      sizeBytes: 0,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, numId, 'struct-field');

    // N discriminant [8B] - OVERHEAD
    const nDiscId = idGen.nodeId('n-disc');
    this.createAndAddNode(nDiscId, 'serde-n-discriminant', 'N discriminant', {
      caption: `N::${numType} tag (8B, type overhead)`,
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), numId, nDiscId, 'struct-field');

    // Actual value [8B] - DATA!
    const valId = idGen.nodeId('n-val');
    const valType = Number.isInteger(value) ? (value >= 0 ? 'u64' : 'i64') : 'f64';
    this.createAndAddNode(valId, 'serde-n-value', valType, {
      caption: `${value} (8B ${valType}, DATA)`,
      sizeBytes: 8,
      isOverhead: false,  // This IS the data!
      value,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), numId, valId, 'struct-field');

    // Padding [8B] to fill Value enum to 32B - OVERHEAD
    const padId = idGen.nodeId('pad');
    this.createAndAddNode(padId, 'serde-padding', 'padding', {
      caption: 'padding (8B, alignment overhead)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, padId, 'struct-field');

    return { rootNodeId: valueId, nodes, relationships: [] };
  }

  /**
   * Value::String (32 bytes stack + heap)
   * - discriminant [8B] - tag = 3 (OVERHEAD)
   * - String [24B]:
   *   - ptr [8B] - pointer to heap (OVERHEAD)
   *   - len [8B] - length (OVERHEAD)
   *   - cap [8B] - capacity (OVERHEAD)
   * - heap: [u8; len] - actual UTF-8 bytes (DATA!)
   */
  private generateString(value: string, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const len = value.length;

    // Check for duplicate
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // Value::String container
    const valueId = idGen.nodeId('val');
    this.createAndAddNode(valueId, 'serde-value-enum', this.getValueLabel(value), {
      caption: isDup ? `Value::String @ ${path} (DUP!)` : `Value::String @ ${path}`,
      sizeBytes: 0,
      isDuplicate: isDup,
      implementation: this.implementation,
    });

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, valueId, 'contains');
    }

    // Value discriminant [8B] - OVERHEAD
    const discId = idGen.nodeId('disc');
    this.createAndAddNode(discId, 'serde-discriminant', 'discriminant', {
      caption: 'discriminant = 3 (8B, type overhead)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, discId, 'struct-field');

    // String container
    const { nodeId: strId } = this.context.stringTracker.track(value);
    this.createAndAddNode(strId, 'serde-string', 'String', {
      caption: isDup ? 'String (DUP!)' : 'String',
      sizeBytes: 0,
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, strId, 'struct-field');

    // ptr [8B] - OVERHEAD
    const ptrId = idGen.nodeId('ptr');
    this.createAndAddNode(ptrId, 'serde-string-ptr', 'ptr', {
      caption: 'ptr (8B, pointer overhead)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), strId, ptrId, 'struct-field');

    // len [8B] - OVERHEAD
    const lenId = idGen.nodeId('len');
    this.createAndAddNode(lenId, 'serde-string-len', 'len', {
      caption: `len = ${len} (8B, metadata overhead)`,
      sizeBytes: 8,
      isOverhead: true,
      value: String(len),
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), strId, lenId, 'struct-field');

    // cap [8B] - OVERHEAD
    const capId = idGen.nodeId('cap');
    this.createAndAddNode(capId, 'serde-string-cap', 'cap', {
      caption: `cap = ${len} (8B, capacity overhead)`,
      sizeBytes: 8,
      isOverhead: true,
      value: String(len),
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), strId, capId, 'struct-field');

    // Heap data [len bytes] - DATA!
    const dataId = idGen.nodeId('data');
    const displayValue = len <= 20 ? `"${value}"` : `"${value.slice(0, 17)}..."`;
    this.createAndAddNode(dataId, 'serde-string-data', 'heap data', {
      caption: `[u8; ${len}] = ${displayValue} (DATA)`,
      sizeBytes: len,
      isOverhead: false,  // This IS the data!
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), ptrId, dataId, 'points-to');

    return { rootNodeId: valueId, nodes, relationships: [] };
  }

  /**
   * Value::Array (32 bytes stack + heap)
   * - discriminant [8B] - tag = 4 (OVERHEAD)
   * - Vec<Value> [24B]:
   *   - ptr [8B] - pointer to heap (OVERHEAD)
   *   - len [8B] - length (OVERHEAD)
   *   - cap [8B] - capacity (OVERHEAD)
   * - heap: [Value; cap] - array of 32B Values (each Value is recursively expanded)
   */
  private generateArray(value: unknown[], path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const count = value.length;

    // Value::Array container
    const valueId = idGen.nodeId('val');
    this.createAndAddNode(valueId, 'serde-value-enum', `Array[${count}]`, {
      caption: `Value::Array @ ${path}`,
      sizeBytes: 0,
      implementation: this.implementation,
    });

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, valueId, 'contains');
    }

    // Value discriminant [8B] - OVERHEAD
    const discId = idGen.nodeId('disc');
    this.createAndAddNode(discId, 'serde-discriminant', 'discriminant', {
      caption: 'discriminant = 4 (8B, type overhead)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, discId, 'struct-field');

    // Vec<Value> container
    const vecId = idGen.nodeId('vec');
    this.createAndAddNode(vecId, 'serde-vec', 'Vec<Value>', {
      caption: `Vec<Value>[${count}]`,
      sizeBytes: 0,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, vecId, 'struct-field');

    // ptr [8B] - OVERHEAD
    const ptrId = idGen.nodeId('ptr');
    this.createAndAddNode(ptrId, 'serde-vec-ptr', 'ptr', {
      caption: 'ptr (8B, pointer overhead)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), vecId, ptrId, 'struct-field');

    // len [8B] - OVERHEAD
    const lenId = idGen.nodeId('len');
    this.createAndAddNode(lenId, 'serde-vec-len', 'len', {
      caption: `len = ${count} (8B, metadata overhead)`,
      sizeBytes: 8,
      isOverhead: true,
      value: String(count),
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), vecId, lenId, 'struct-field');

    // cap [8B] - OVERHEAD
    const capId = idGen.nodeId('cap');
    this.createAndAddNode(capId, 'serde-vec-cap', 'cap', {
      caption: `cap = ${count} (8B, capacity overhead)`,
      sizeBytes: 8,
      isOverhead: true,
      value: String(count),
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), vecId, capId, 'struct-field');

    // Heap array container
    if (count > 0) {
      const heapId = idGen.nodeId('heap');
      this.createAndAddNode(heapId, 'serde-vec-data', '[Value]', {
        caption: `[Value; ${count}] on heap`,
        sizeBytes: 0,
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), ptrId, heapId, 'points-to');

      // Generate each child Value
      value.forEach((item, index) => {
        const childPath = `${path}[${index}]`;
        const result = this.generateValue(item, childPath, heapId);
        nodes.push(...result.nodes);
      });
    }

    return { rootNodeId: valueId, nodes, relationships: [] };
  }

  /**
   * Value::Object (32 bytes stack + ~628 bytes LeafNode heap!)
   *
   * Stack layout (32 bytes):
   * - discriminant [8B] - tag = 5 (OVERHEAD)
   * - BTreeMap<String, Value> [16B]:
   *   - root [8B] - pointer to LeafNode (OVERHEAD)
   *   - length [8B] - entry count (OVERHEAD)
   * - padding [8B] - OVERHEAD
   *
   * LeafNode<String, Value> on heap (~628 bytes!):
   * Reference: rust/library/alloc/src/collections/btree/node.rs
   * B = 6, CAPACITY = 11
   *
   * - parent: Option<NonNull<InternalNode>> [8B] (OVERHEAD)
   * - parent_idx: MaybeUninit<u16> [2B] (OVERHEAD)
   * - len: u16 [2B] (OVERHEAD)
   * - keys: [MaybeUninit<String>; 11] [264B] (11 slots, most WASTED!)
   * - vals: [MaybeUninit<Value>; 11] [352B] (11 slots, most WASTED!)
   *
   * For a 1-entry object: 10 key slots + 10 val slots = 560 bytes WASTED!
   */
  private generateObject(
    value: Record<string, unknown>,
    path: string,
    parentId?: string
  ): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const keys = Object.keys(value);
    const count = keys.length;
    const BTREE_CAPACITY = 11;  // Rust BTreeMap B=6, CAPACITY=2*B-1=11

    // Value::Object container
    const valueId = idGen.nodeId('val');
    this.createAndAddNode(valueId, 'serde-value-enum', `Object{${count}}`, {
      caption: `Value::Object @ ${path}`,
      sizeBytes: 0,
      implementation: this.implementation,
    });

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, valueId, 'contains');
    }

    // Value discriminant [8B] - OVERHEAD
    const discId = idGen.nodeId('disc');
    this.createAndAddNode(discId, 'serde-discriminant', 'discriminant', {
      caption: 'discriminant = 5 (8B, type overhead)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, discId, 'struct-field');

    // BTreeMap<String, Value> container
    const mapId = idGen.nodeId('map');
    this.createAndAddNode(mapId, 'serde-btreemap', 'BTreeMap', {
      caption: `BTreeMap<String, Value>{${count}}`,
      sizeBytes: 0,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, mapId, 'struct-field');

    // root [8B] - OVERHEAD
    const rootId = idGen.nodeId('root');
    this.createAndAddNode(rootId, 'serde-btree-root', 'root', {
      caption: 'root ptr (8B, pointer overhead)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), mapId, rootId, 'struct-field');

    // length [8B] - OVERHEAD
    const lenId = idGen.nodeId('len');
    this.createAndAddNode(lenId, 'serde-btree-len', 'length', {
      caption: `length = ${count} (8B, metadata overhead)`,
      sizeBytes: 8,
      isOverhead: true,
      value: String(count),
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), mapId, lenId, 'struct-field');

    // Padding [8B] - OVERHEAD
    const padId = idGen.nodeId('pad');
    this.createAndAddNode(padId, 'serde-padding', 'padding', {
      caption: 'padding (8B, alignment overhead)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), valueId, padId, 'struct-field');

    // ========== LeafNode<String, Value> on heap (~628 bytes!) ==========
    const leafId = idGen.nodeId('leaf');
    this.createAndAddNode(leafId, 'serde-btree-leaf', 'LeafNode', {
      caption: `LeafNode<String, Value> (~628B heap!)`,
      sizeBytes: 0,  // Size in children
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), rootId, leafId, 'points-to');

    // parent: Option<NonNull<InternalNode>> [8B] - OVERHEAD
    const parentPtrId = idGen.nodeId('parent');
    this.createAndAddNode(parentPtrId, 'serde-leafnode-parent', 'parent', {
      caption: 'parent = None (8B, pointer overhead)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), leafId, parentPtrId, 'struct-field');

    // parent_idx: MaybeUninit<u16> [2B] - OVERHEAD
    const parentIdxId = idGen.nodeId('parent-idx');
    this.createAndAddNode(parentIdxId, 'serde-leafnode-parent-idx', 'parent_idx', {
      caption: 'parent_idx (2B, overhead)',
      sizeBytes: 2,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), leafId, parentIdxId, 'struct-field');

    // len: u16 [2B] - OVERHEAD
    const nodeLen = idGen.nodeId('node-len');
    this.createAndAddNode(nodeLen, 'serde-leafnode-len', 'len', {
      caption: `len = ${count} (2B, metadata overhead)`,
      sizeBytes: 2,
      isOverhead: true,
      value: String(count),
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), leafId, nodeLen, 'struct-field');

    // ========== keys: [MaybeUninit<String>; 11] array (264 bytes) ==========
    const keysArrayId = idGen.nodeId('keys-arr');
    this.createAndAddNode(keysArrayId, 'serde-leafnode-keys', 'keys[11]', {
      caption: `keys: [String; 11] (264B, ${count} used, ${BTREE_CAPACITY - count} WASTED)`,
      sizeBytes: 0,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), leafId, keysArrayId, 'struct-field');

    // ========== vals: [MaybeUninit<Value>; 11] array (352 bytes) ==========
    const valsArrayId = idGen.nodeId('vals-arr');
    this.createAndAddNode(valsArrayId, 'serde-leafnode-vals', 'vals[11]', {
      caption: `vals: [Value; 11] (352B, ${count} used, ${BTREE_CAPACITY - count} WASTED)`,
      sizeBytes: 0,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), leafId, valsArrayId, 'struct-field');

    // Generate WASTED key slots first (to show the waste prominently)
    const wastedKeySlots = BTREE_CAPACITY - count;
    for (let i = 0; i < wastedKeySlots; i++) {
      const wastedKeyId = idGen.nodeId(`wasted-key-${i}`);
      this.createAndAddNode(wastedKeyId, 'serde-leafnode-wasted-key', `[${count + i}]`, {
        caption: `WASTED key slot (24B overhead!)`,
        sizeBytes: 24,
        isOverhead: true,  // WASTED = pure overhead!
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), keysArrayId, wastedKeyId, 'array-element');
    }

    // Generate WASTED val slots
    const wastedValSlots = BTREE_CAPACITY - count;
    for (let i = 0; i < wastedValSlots; i++) {
      const wastedValId = idGen.nodeId(`wasted-val-${i}`);
      this.createAndAddNode(wastedValId, 'serde-leafnode-wasted-val', `[${count + i}]`, {
        caption: `WASTED val slot (32B overhead!)`,
        sizeBytes: 32,
        isOverhead: true,  // WASTED = pure overhead!
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), valsArrayId, wastedValId, 'array-element');
    }

    // Generate USED key-value pairs
    keys.forEach((key, index) => {
      const childPath = `${path}.${key}`;

      // Used key slot [24B] - contains String header
      const keySlotId = idGen.nodeId(`key-slot-${index}`);
      this.createAndAddNode(keySlotId, 'serde-leafnode-key-slot', `[${index}]`, {
        caption: `key slot[${index}] (24B String header)`,
        sizeBytes: 0,  // Size in String children
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), keysArrayId, keySlotId, 'array-element');

      // Key String fields inside the slot
      // ptr [8B] - OVERHEAD
      const keyPtrId = idGen.nodeId(`key-ptr-${index}`);
      this.createAndAddNode(keyPtrId, 'serde-string-ptr', 'ptr', {
        caption: 'ptr (8B, pointer overhead)',
        sizeBytes: 8,
        isOverhead: true,
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), keySlotId, keyPtrId, 'struct-field');

      // len [8B] - OVERHEAD
      const keyLenId = idGen.nodeId(`key-len-${index}`);
      this.createAndAddNode(keyLenId, 'serde-string-len', 'len', {
        caption: `len = ${key.length} (8B, metadata overhead)`,
        sizeBytes: 8,
        isOverhead: true,
        value: String(key.length),
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), keySlotId, keyLenId, 'struct-field');

      // cap [8B] - OVERHEAD
      const keyCapId = idGen.nodeId(`key-cap-${index}`);
      this.createAndAddNode(keyCapId, 'serde-string-cap', 'cap', {
        caption: `cap = ${key.length} (8B, capacity overhead)`,
        sizeBytes: 8,
        isOverhead: true,
        value: String(key.length),
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), keySlotId, keyCapId, 'struct-field');

      // Key heap data - DATA!
      const keyDataId = idGen.nodeId(`key-data-${index}`);
      this.createAndAddNode(keyDataId, 'serde-string-data', `"${key}"`, {
        caption: `[u8; ${key.length}] = "${key}" (DATA)`,
        sizeBytes: key.length,
        isOverhead: false,  // This IS the data!
        value: key,
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), keyPtrId, keyDataId, 'points-to');

      // Used val slot [32B] - contains Value
      const valSlotId = idGen.nodeId(`val-slot-${index}`);
      this.createAndAddNode(valSlotId, 'serde-leafnode-val-slot', `[${index}]`, {
        caption: `val slot[${index}] (32B Value)`,
        sizeBytes: 0,  // Size in Value children
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), valsArrayId, valSlotId, 'array-element');

      // Generate Value (recursively) - connects to val slot
      const valueResult = this.generateValue(value[key], childPath);
      this.createAndAddRelationship(idGen.edgeId(), valSlotId, valueResult.rootNodeId, 'contains');
    });

    return { rootNodeId: valueId, nodes, relationships: [] };
  }
}
