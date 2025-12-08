import type { ImplementationId, NvlNode } from '../types/graph';
import type { GenerationResult, StringTracker, IdGenerator } from './types';
import { BaseGenerator } from './base';
import { SequentialIdGenerator } from './utils/id-generator';
import { DuplicatingStringTracker } from './utils/string-tracker';
import { SIZE_ESTIMATES } from './types';

/**
 * Jansson Graph Generator - Scientific Memory Visualization
 *
 * Reference: jansson.h, jansson_private.h, hashtable.h, hashtable.c
 *
 * Key structures:
 * - json_t base: type [4B] + refcount [8B] + padding [4B] = 16 bytes OVERHEAD per value
 * - json_object_t: json_t [16B] + hashtable_t [56B] = 72 bytes
 * - hashtable_t: size + buckets* + order + 2×list (56 bytes)
 * - hashtable_bucket: first* + last* = 16 bytes × 8 buckets = 128 bytes
 * - hashtable_pair: 2×list + hash + value* + key_len + key[] = 56 + key_len + 1 bytes
 * - json_string_t: json_t [16B] + char* [8B] + length [8B] = 32 bytes + heap data
 *
 * Memory Classification:
 * - TRUE DATA: actual string bytes, integer values, float values
 * - OVERHEAD: type enums, refcounts, pointers, lengths, hash values, linked list ptrs
 */
export class JanssonGenerator extends BaseGenerator {
  readonly id: ImplementationId = 'jansson';
  readonly name = 'Jansson';
  protected readonly implementation = 'jansson' as const;

  protected getLanguage(): string {
    return 'C';
  }

  protected getDescription(): string {
    return 'json_t base, hashtable_pair, 8 buckets × 16B, no string interning';
  }

  protected createStringTracker(idGen: () => string): StringTracker {
    // Jansson doesn't intern strings
    return new DuplicatingStringTracker(idGen);
  }

  protected createIdGenerator(): IdGenerator {
    return new SequentialIdGenerator('jan-');
  }

  protected generateInfrastructure(): void {
    // Jansson has no special infrastructure (no type objects like Python)
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
   * Generate json_t base header with field-level detail
   * Reference: jansson.h - 16 bytes with padding
   */
  private generateJsonTBase(
    containerId: string,
    typeName: string,
  ): NvlNode[] {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // type enum - 4 bytes OVERHEAD
    const typeId = idGen.nodeId('type');
    nodes.push(this.createAndAddNode(typeId, 'jansson-type-enum', typeName, {
      caption: `json_type [4B] = ${typeName}`,
      sizeBytes: SIZE_ESTIMATES['jansson-type-enum'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), containerId, typeId, 'json-t-to-type');

    // refcount - 8 bytes OVERHEAD
    const refId = idGen.nodeId('ref');
    nodes.push(this.createAndAddNode(refId, 'jansson-refcount', 'refcount=1', {
      caption: 'refcount [8B] = 1',
      sizeBytes: SIZE_ESTIMATES['jansson-refcount'],
      isOverhead: true,
      refCount: 1,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), containerId, refId, 'json-t-to-refcount');

    // padding - 4 bytes OVERHEAD
    const padId = idGen.nodeId('pad');
    nodes.push(this.createAndAddNode(padId, 'jansson-padding', 'padding', {
      caption: 'padding [4B]',
      sizeBytes: SIZE_ESTIMATES['jansson-padding'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), containerId, padId, 'struct-field');

    return nodes;
  }

  private generateNull(path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate
    const isDup = this.context.valueTracker.isDuplicate(null);
    this.context.valueTracker.markSeen(null);

    // json_t container for NULL (16 bytes)
    const nodeId = idGen.nodeId('null');
    nodes.push(this.createAndAddNode(nodeId, 'jansson-null', 'null', {
      caption: isDup ? `json_t (null) @ ${path} (DUP!)` : `json_t (null) @ ${path}`,
      sizeBytes: 0, // Size from children
      isDuplicate: isDup,
      implementation: this.implementation,
    }));

    // json_t base fields
    nodes.push(...this.generateJsonTBase(nodeId, 'JSON_NULL'));

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, nodeId, 'contains');
    }

    return { rootNodeId: nodeId, nodes, relationships: [] };
  }

  private generateBool(value: boolean, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // json_t container for TRUE/FALSE (16 bytes)
    const nodeId = idGen.nodeId('bool');
    const nodeType = value ? 'jansson-true' : 'jansson-false';
    nodes.push(this.createAndAddNode(nodeId, nodeType, String(value), {
      caption: isDup ? `json_t (${value}) @ ${path} (DUP!)` : `json_t (${value}) @ ${path}`,
      sizeBytes: 0, // Size from children
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    }));

    // json_t base fields
    nodes.push(...this.generateJsonTBase(nodeId, value ? 'JSON_TRUE' : 'JSON_FALSE'));

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, nodeId, 'contains');
    }

    return { rootNodeId: nodeId, nodes, relationships: [] };
  }

  /**
   * Generate json_integer_t or json_real_t with field-level detail
   * Reference: jansson_private.h
   */
  private generateNumber(value: number, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    const isInteger = Number.isInteger(value);
    const nodeId = idGen.nodeId(isInteger ? 'int' : 'real');
    const nodeType = isInteger ? 'jansson-integer' : 'jansson-real';
    const typeName = isInteger ? 'JSON_INTEGER' : 'JSON_REAL';

    // json_integer_t or json_real_t container (24 bytes)
    nodes.push(this.createAndAddNode(nodeId, nodeType, String(value), {
      caption: isDup ? `${nodeType} @ ${path} (DUP!)` : `${nodeType} @ ${path}`,
      sizeBytes: 0, // Size from children
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    }));

    // json_t base fields (16 bytes)
    nodes.push(...this.generateJsonTBase(nodeId, typeName));

    // Value field - 8 bytes DATA
    const valId = idGen.nodeId('val');
    const valType = isInteger ? 'jansson-int-value' : 'jansson-real-value';
    nodes.push(this.createAndAddNode(valId, valType, String(value), {
      caption: isInteger ? `json_int_t [8B] = ${value}` : `double [8B] = ${value}`,
      sizeBytes: SIZE_ESTIMATES[valType],
      isOverhead: false, // THIS IS DATA!
      value,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(
      idGen.edgeId(),
      nodeId,
      valId,
      isInteger ? 'integer-to-value' : 'real-to-value'
    );

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, nodeId, 'contains');
    }

    return { rootNodeId: nodeId, nodes, relationships: [] };
  }

  /**
   * Generate json_string_t with field-level detail
   * Reference: jansson_private.h - 32 bytes + heap data
   */
  private generateString(value: string, _path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // json_string_t container (32 bytes header)
    const { nodeId: strId } = this.context.stringTracker.track(value);
    nodes.push(this.createAndAddNode(strId, 'jansson-string', this.getValueLabel(value), {
      caption: isDup ? `json_string_t (DUP!) "${value}"` : `json_string_t "${value}"`,
      sizeBytes: 0, // Size from children
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    }));

    // json_t base fields (16 bytes)
    nodes.push(...this.generateJsonTBase(strId, 'JSON_STRING'));

    // value pointer - 8 bytes OVERHEAD
    const ptrId = idGen.nodeId('sptr');
    nodes.push(this.createAndAddNode(ptrId, 'jansson-string-ptr', 'value', {
      caption: 'char* value [8B]',
      sizeBytes: SIZE_ESTIMATES['jansson-string-ptr'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), strId, ptrId, 'jstring-to-ptr');

    // length field - 8 bytes OVERHEAD
    const lenId = idGen.nodeId('slen');
    nodes.push(this.createAndAddNode(lenId, 'jansson-string-length', `length=${value.length}`, {
      caption: `size_t length [8B] = ${value.length}`,
      sizeBytes: SIZE_ESTIMATES['jansson-string-length'],
      isOverhead: true, // LENGTH IS OVERHEAD!
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), strId, lenId, 'jstring-to-len');

    // Heap data - actual DATA bytes
    const dataId = idGen.nodeId('sdata');
    const dataSize = (SIZE_ESTIMATES['jansson-string-data'] as (len: number) => number)(value.length);
    nodes.push(this.createAndAddNode(dataId, 'jansson-string-data', `"${value}"`, {
      caption: `heap char[${value.length}] DATA`,
      sizeBytes: dataSize,
      isOverhead: false, // THIS IS DATA!
      value,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), ptrId, dataId, 'jstring-ptr-to-data');

    // Null terminator - 1 byte OVERHEAD
    const nullId = idGen.nodeId('snul');
    nodes.push(this.createAndAddNode(nullId, 'jansson-string-null', '\\0', {
      caption: 'null terminator [1B]',
      sizeBytes: SIZE_ESTIMATES['jansson-string-null'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), dataId, nullId, 'struct-field');

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, strId, 'contains');
    }

    return { rootNodeId: strId, nodes, relationships: [] };
  }

  /**
   * Generate json_array_t with field-level detail
   * Reference: jansson_private.h - 40 bytes + table
   */
  private generateArray(value: unknown[], path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // json_array_t container (40 bytes)
    const arrayId = idGen.nodeId('arr');
    nodes.push(this.createAndAddNode(arrayId, 'jansson-array', `array[${value.length}]`, {
      caption: `json_array_t @ ${path}`,
      sizeBytes: 0, // Size from children
      implementation: this.implementation,
    }));

    // json_t base fields (16 bytes)
    nodes.push(...this.generateJsonTBase(arrayId, 'JSON_ARRAY'));

    // size field - 8 bytes OVERHEAD (capacity)
    const sizeId = idGen.nodeId('asize');
    nodes.push(this.createAndAddNode(sizeId, 'jansson-array-size', `size=${value.length}`, {
      caption: `size_t size [8B] = ${value.length}`,
      sizeBytes: SIZE_ESTIMATES['jansson-array-size'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), arrayId, sizeId, 'jarray-to-size');

    // entries field - 8 bytes OVERHEAD (count)
    const entId = idGen.nodeId('aent');
    nodes.push(this.createAndAddNode(entId, 'jansson-array-entries', `entries=${value.length}`, {
      caption: `size_t entries [8B] = ${value.length}`,
      sizeBytes: SIZE_ESTIMATES['jansson-array-entries'],
      isOverhead: true, // COUNT IS OVERHEAD!
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), arrayId, entId, 'jarray-to-entries');

    // table pointer - 8 bytes OVERHEAD
    const tblId = idGen.nodeId('atbl');
    nodes.push(this.createAndAddNode(tblId, 'jansson-array-table', 'table', {
      caption: 'json_t** table [8B]',
      sizeBytes: SIZE_ESTIMATES['jansson-array-table-ptr'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), arrayId, tblId, 'jarray-to-table');

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, arrayId, 'contains');
    }

    // Generate children with pointer slots
    value.forEach((item, index) => {
      // Slot pointer - 8 bytes OVERHEAD per element
      const slotId = idGen.nodeId('slot');
      nodes.push(this.createAndAddNode(slotId, 'jansson-array-slot', `[${index}]`, {
        caption: `json_t* slot [8B]`,
        sizeBytes: SIZE_ESTIMATES['jansson-array-slot'],
        isOverhead: true, // POINTER IS OVERHEAD!
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), tblId, slotId, 'jarray-table-to-element');

      const childPath = `${path}[${index}]`;
      const result = this.generateValue(item, childPath, slotId);
      nodes.push(...result.nodes);
    });

    return { rootNodeId: arrayId, nodes, relationships: [] };
  }

  /**
   * Generate json_object_t with full hashtable detail
   * Reference: jansson_private.h, hashtable.h, hashtable.c
   *
   * Shows:
   * - json_object_t (72 bytes = json_t + hashtable_t)
   * - hashtable_t (56 bytes)
   * - bucket array (8 × 16 bytes = 128 bytes)
   * - hashtable_pair per entry (56 + key_len + 1 bytes)
   */
  private generateObject(
    value: Record<string, unknown>,
    path: string,
    parentId?: string
  ): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const keys = Object.keys(value);
    const entryCount = keys.length;

    // ==================== json_object_t (72 bytes) ====================
    const objId = idGen.nodeId('obj');
    nodes.push(this.createAndAddNode(objId, 'jansson-object', `object{${entryCount}}`, {
      caption: `json_object_t @ ${path}`,
      sizeBytes: 0, // Size from children
      implementation: this.implementation,
    }));

    // json_t base fields (16 bytes)
    nodes.push(...this.generateJsonTBase(objId, 'JSON_OBJECT'));

    // ==================== hashtable_t (56 bytes) ====================
    const htId = idGen.nodeId('ht');
    nodes.push(this.createAndAddNode(htId, 'jansson-hashtable', `hashtable[${entryCount}]`, {
      caption: 'hashtable_t [56B]',
      sizeBytes: 0, // Size from children
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), objId, htId, 'object-to-hashtable');

    // size - 8 bytes OVERHEAD (entry count!)
    const sizeId = idGen.nodeId('htsize');
    nodes.push(this.createAndAddNode(sizeId, 'jansson-ht-size', `size=${entryCount}`, {
      caption: `size_t size [8B] = ${entryCount}`,
      sizeBytes: SIZE_ESTIMATES['jansson-ht-size'],
      isOverhead: true, // COUNT IS OVERHEAD!
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), htId, sizeId, 'ht-to-size');

    // buckets pointer - 8 bytes OVERHEAD
    const bktPtrId = idGen.nodeId('htbkt');
    nodes.push(this.createAndAddNode(bktPtrId, 'jansson-ht-buckets-ptr', 'buckets', {
      caption: 'bucket_t* buckets [8B]',
      sizeBytes: SIZE_ESTIMATES['jansson-ht-buckets-ptr'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), htId, bktPtrId, 'ht-to-buckets');

    // order - 8 bytes OVERHEAD
    const orderId = idGen.nodeId('htord');
    nodes.push(this.createAndAddNode(orderId, 'jansson-ht-order', 'order=3', {
      caption: 'size_t order [8B] = 3 (8 buckets)',
      sizeBytes: SIZE_ESTIMATES['jansson-ht-order'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), htId, orderId, 'struct-field');

    // list (collision chain head) - 16 bytes OVERHEAD
    const listPrevId = idGen.nodeId('htlp');
    nodes.push(this.createAndAddNode(listPrevId, 'jansson-ht-list-prev', 'list.prev', {
      caption: 'list.prev [8B]',
      sizeBytes: SIZE_ESTIMATES['jansson-ht-list-prev'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), htId, listPrevId, 'struct-field');

    const listNextId = idGen.nodeId('htln');
    nodes.push(this.createAndAddNode(listNextId, 'jansson-ht-list-next', 'list.next', {
      caption: 'list.next [8B]',
      sizeBytes: SIZE_ESTIMATES['jansson-ht-list-next'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), htId, listNextId, 'struct-field');

    // ordered_list (insertion order head) - 16 bytes OVERHEAD
    const ordPrevId = idGen.nodeId('htop');
    nodes.push(this.createAndAddNode(ordPrevId, 'jansson-ht-ordered-prev', 'ordered.prev', {
      caption: 'ordered_list.prev [8B]',
      sizeBytes: SIZE_ESTIMATES['jansson-ht-ordered-prev'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), htId, ordPrevId, 'struct-field');

    const ordNextId = idGen.nodeId('hton');
    nodes.push(this.createAndAddNode(ordNextId, 'jansson-ht-ordered-next', 'ordered.next', {
      caption: 'ordered_list.next [8B]',
      sizeBytes: SIZE_ESTIMATES['jansson-ht-ordered-next'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), htId, ordNextId, 'struct-field');

    // ==================== Bucket Array (8 × 16 = 128 bytes) ====================
    const BUCKET_COUNT = 8; // INITIAL_HASHTABLE_ORDER = 3 means 2^3 = 8 buckets
    const bucketArrayId = idGen.nodeId('bktarr');
    nodes.push(this.createAndAddNode(bucketArrayId, 'jansson-bucket-array', `buckets[${BUCKET_COUNT}]`, {
      caption: `bucket array [${BUCKET_COUNT} × 16B = 128B]`,
      sizeBytes: 0, // Size from children
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), bktPtrId, bucketArrayId, 'points-to');

    // Generate all 8 buckets (each 16 bytes)
    for (let i = 0; i < BUCKET_COUNT; i++) {
      const bucketId = idGen.nodeId('bkt');
      nodes.push(this.createAndAddNode(bucketId, 'jansson-bucket', `bucket[${i}]`, {
        caption: `bucket_t [16B]`,
        sizeBytes: 0, // Size from children
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), bucketArrayId, bucketId, 'array-element');

      // first pointer - 8 bytes OVERHEAD
      const firstId = idGen.nodeId('bfst');
      nodes.push(this.createAndAddNode(firstId, 'jansson-bucket-first', 'first', {
        caption: 'first* [8B]',
        sizeBytes: SIZE_ESTIMATES['jansson-bucket-first'],
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), bucketId, firstId, 'struct-field');

      // last pointer - 8 bytes OVERHEAD
      const lastId = idGen.nodeId('blst');
      nodes.push(this.createAndAddNode(lastId, 'jansson-bucket-last', 'last', {
        caption: 'last* [8B]',
        sizeBytes: SIZE_ESTIMATES['jansson-bucket-last'],
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), bucketId, lastId, 'struct-field');
    }

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, objId, 'contains');
    }

    // ==================== Generate hashtable_pair for each entry ====================
    for (const key of keys) {
      const childPath = `${path}["${key}"]`;

      // Check for duplicate key
      const isKeyDup = this.context.valueTracker.isDuplicate(key);
      this.context.valueTracker.markSeen(key);

      // hashtable_pair container (56 + key_len + 1 bytes)
      const pairId = idGen.nodeId('pair');
      const keyDataSize = (SIZE_ESTIMATES['jansson-pair-key-data'] as (len: number) => number)(key.length);
      nodes.push(this.createAndAddNode(pairId, 'jansson-pair', `"${key}":...`, {
        caption: isKeyDup ? `hashtable_pair (DUP key!)` : `hashtable_pair`,
        sizeBytes: 0, // Size from children
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), htId, pairId, 'has-pair');

      // list.prev - 8 bytes OVERHEAD (collision chain)
      const lpId = idGen.nodeId('plp');
      nodes.push(this.createAndAddNode(lpId, 'jansson-pair-list-prev', 'list.prev', {
        caption: 'list.prev [8B]',
        sizeBytes: SIZE_ESTIMATES['jansson-pair-list-prev'],
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), pairId, lpId, 'pair-to-list-prev');

      // list.next - 8 bytes OVERHEAD
      const lnId = idGen.nodeId('pln');
      nodes.push(this.createAndAddNode(lnId, 'jansson-pair-list-next', 'list.next', {
        caption: 'list.next [8B]',
        sizeBytes: SIZE_ESTIMATES['jansson-pair-list-next'],
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), pairId, lnId, 'pair-to-list-next');

      // ordered_list.prev - 8 bytes OVERHEAD (insertion order)
      const opId = idGen.nodeId('pop');
      nodes.push(this.createAndAddNode(opId, 'jansson-pair-ordered-prev', 'ordered.prev', {
        caption: 'ordered_list.prev [8B]',
        sizeBytes: SIZE_ESTIMATES['jansson-pair-ordered-prev'],
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), pairId, opId, 'pair-to-ordered-prev');

      // ordered_list.next - 8 bytes OVERHEAD
      const onId = idGen.nodeId('pon');
      nodes.push(this.createAndAddNode(onId, 'jansson-pair-ordered-next', 'ordered.next', {
        caption: 'ordered_list.next [8B]',
        sizeBytes: SIZE_ESTIMATES['jansson-pair-ordered-next'],
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), pairId, onId, 'pair-to-ordered-next');

      // hash - 8 bytes OVERHEAD
      const hashId = idGen.nodeId('phash');
      nodes.push(this.createAndAddNode(hashId, 'jansson-pair-hash', 'hash', {
        caption: 'size_t hash [8B]',
        sizeBytes: SIZE_ESTIMATES['jansson-pair-hash'],
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), pairId, hashId, 'pair-to-hash');

      // value pointer - 8 bytes OVERHEAD
      const valPtrId = idGen.nodeId('pvptr');
      nodes.push(this.createAndAddNode(valPtrId, 'jansson-pair-value-ptr', 'value', {
        caption: 'json_t* value [8B]',
        sizeBytes: SIZE_ESTIMATES['jansson-pair-value-ptr'],
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), pairId, valPtrId, 'struct-field');

      // key_len - 8 bytes OVERHEAD
      const keyLenId = idGen.nodeId('pklen');
      nodes.push(this.createAndAddNode(keyLenId, 'jansson-pair-key-len', `key_len=${key.length}`, {
        caption: `size_t key_len [8B] = ${key.length}`,
        sizeBytes: SIZE_ESTIMATES['jansson-pair-key-len'],
        isOverhead: true, // LENGTH IS OVERHEAD!
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), pairId, keyLenId, 'pair-to-key-len');

      // key[] flexible array - DATA!
      const keyDataId = idGen.nodeId('pkdata');
      nodes.push(this.createAndAddNode(keyDataId, 'jansson-pair-key-data', `"${key}"`, {
        caption: `char key[${key.length}] DATA`,
        sizeBytes: keyDataSize,
        isOverhead: false, // THIS IS DATA! (key stored inline)
        isDuplicate: isKeyDup,
        value: key,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), pairId, keyDataId, 'pair-to-key-data');

      // null terminator - 1 byte OVERHEAD
      const keyNullId = idGen.nodeId('pknul');
      nodes.push(this.createAndAddNode(keyNullId, 'jansson-pair-key-null', '\\0', {
        caption: 'null terminator [1B]',
        sizeBytes: SIZE_ESTIMATES['jansson-pair-key-null'],
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), keyDataId, keyNullId, 'struct-field');

      // Generate value (pointed by value pointer)
      const result = this.generateValue(value[key], childPath, valPtrId);
      nodes.push(...result.nodes);
    }

    return { rootNodeId: objId, nodes, relationships: [] };
  }
}
