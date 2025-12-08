import type { ImplementationId, NvlNode } from '../types/graph';
import type { GenerationResult, StringTracker, IdGenerator } from './types';
import { BaseGenerator } from './base';
import { SequentialIdGenerator } from './utils/id-generator';
import { DuplicatingStringTracker } from './utils/string-tracker';
import { SIZE_ESTIMATES } from './types';

/**
 * Go encoding/json Graph Generator - Scientific Memory Visualization
 *
 * Reference: runtime/runtime2.go, runtime/string.go, runtime/map.go
 *
 * Key structures:
 * - interface{} (eface): _type [8B] + data [8B] = 16 bytes OVERHEAD per value
 * - string: str ptr [8B] + len [8B] = 16 bytes header + heap data
 * - hmap: 48 bytes header (count, flags, B, noverflow, hash0, buckets, oldbuckets, nevacuate, extra)
 * - bucket (bmap): 272 bytes (tophash[8] + keys[8]×16 + vals[8]×16 + overflow)
 *
 * Memory Classification:
 * - TRUE DATA: actual string bytes, float64 values
 * - OVERHEAD: all pointers, lengths, capacities, interface{} wrappers
 * - WASTED: unused bucket slots (8-slot allocation for any size)
 *
 * Update: Modeled Runtime Singletons (static addresses) for:
 * - bool (true/false)
 * - empty string
 * - zero value (nil)
 */
export class GoJsonGenerator extends BaseGenerator {
  readonly id: ImplementationId = 'go_json';
  readonly name = 'Go encoding/json';
  protected readonly implementation = 'go' as const;

  // Runtime static singleton IDs
  private trueNodeId?: string;
  private falseNodeId?: string;
  private emptyStringNodeId?: string;
  private zeroValueNodeId?: string; // For nil interface data

  protected getLanguage(): string {
    return 'Go';
  }

  protected getDescription(): string {
    return 'interface{} boxing, hmap, 8-slot buckets, no string interning';
  }

  protected createStringTracker(idGen: () => string): StringTracker {
    // Go doesn't intern strings
    return new DuplicatingStringTracker(idGen);
  }

  protected createIdGenerator(): IdGenerator {
    return new SequentialIdGenerator('go-');
  }

  protected generateInfrastructure(): void {
    const idGen = this.context.idGenerator;

    // 1. Static Boolean Singletons
    // Go runtime has static addresses for true/false when used in interfaces
    this.trueNodeId = idGen.nodeId('true');
    this.createAndAddNode(this.trueNodeId, 'go-bool', 'true', {
      caption: 'static true [1B]',
      sizeBytes: 1,
      isOverhead: false, // Shared data
      value: true,
      implementation: this.implementation,
    });

    this.falseNodeId = idGen.nodeId('false');
    this.createAndAddNode(this.falseNodeId, 'go-bool', 'false', {
      caption: 'static false [1B]',
      sizeBytes: 1,
      isOverhead: false, // Shared data
      value: false,
      implementation: this.implementation,
    });

    // 2. Static Empty String Singleton
    // Go strings with length 0 often point to zerobase
    this.emptyStringNodeId = idGen.nodeId('empty-str');
    this.createAndAddNode(this.emptyStringNodeId, 'go-string-data', '""', {
      caption: 'static zerobase [0B]',
      sizeBytes: 0,
      isOverhead: false,
      value: '',
      implementation: this.implementation,
    });

    // 3. Zero Value Singleton (for nil interfaces data pointer)
    // Represents runtime.zeroval or null pointer
    this.zeroValueNodeId = idGen.nodeId('zeroval');
    this.createAndAddNode(this.zeroValueNodeId, 'go-iface-data', 'nil-addr', {
      caption: 'static zeroval',
      sizeBytes: 0,
      isOverhead: true,
      implementation: this.implementation,
    });
  }

  protected generateValue(
    value: unknown,
    path: string,
    parentId?: string
  ): GenerationResult {
    const type = this.getJsonType(value);

    switch (type) {
      case 'null':
        return this.generateNil(path, parentId);
      case 'boolean':
        return this.generateBool(value as boolean, path, parentId);
      case 'number':
        return this.generateFloat64(value as number, path, parentId);
      case 'string':
        return this.generateString(value as string, path, parentId);
      case 'array':
        return this.generateSlice(value as unknown[], path, parentId);
      case 'object':
        return this.generateMap(value as Record<string, unknown>, path, parentId);
    }
  }

  /**
   * Generate interface{} wrapper with field-level detail
   * Reference: runtime/runtime2.go - eface struct
   */
  private generateInterfaceWrapper(
    valueNodeId: string,
    path: string,
    parentId?: string,
    isNil = false
  ): { interfaceId: string; nodes: NvlNode[] } {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // interface{} (eface) container - 16 bytes total
    const interfaceId = idGen.nodeId('iface');
    nodes.push(this.createAndAddNode(interfaceId, 'go-interface', isNil ? 'nil' : 'interface{}', {
      caption: isNil ? `nil interface{} @ ${path}` : `eface @ ${path}`,
      sizeBytes: 0, // Size comes from children
      isOverhead: true,
      implementation: this.implementation,
    }));

    // _type pointer - 8 bytes OVERHEAD
    const typeId = idGen.nodeId('itype');
    nodes.push(this.createAndAddNode(typeId, 'go-iface-type', isNil ? '_type=nil' : '_type', {
      caption: isNil ? '_type pointer [8B] = nil' : '_type pointer [8B]',
      sizeBytes: SIZE_ESTIMATES['go-iface-type'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), interfaceId, typeId, 'iface-type-ptr');

    // data pointer - 8 bytes OVERHEAD
    const dataId = idGen.nodeId('idata');
    nodes.push(this.createAndAddNode(dataId, 'go-iface-data', isNil ? 'data=nil' : 'data', {
      caption: isNil ? 'data pointer [8B] = nil' : 'data pointer [8B]',
      sizeBytes: SIZE_ESTIMATES['go-iface-data'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), interfaceId, dataId, 'iface-data-ptr');

    // Link data pointer to actual value
    // For nil, valueNodeId should be the static zeroValueNodeId
    this.createAndAddRelationship(idGen.edgeId(), dataId, valueNodeId, 'points-to');

    // Link from parent
    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, interfaceId, 'contains');
    }

    return { interfaceId, nodes };
  }

  private generateNil(path: string, parentId?: string): GenerationResult {
    // Reuse the generic interface wrapper logic, pointing to zeroValueNodeId
    const { interfaceId, nodes } = this.generateInterfaceWrapper(
      this.zeroValueNodeId!, 
      path, 
      parentId, 
      true
    );
    return { rootNodeId: interfaceId, nodes, relationships: [] };
  }

  private generateBool(value: boolean, path: string, parentId?: string): GenerationResult {
    // Don't create new bool nodes. Use the static singletons.
    // The "value" is carried by the static node.
    const targetNodeId = value ? this.trueNodeId! : this.falseNodeId!;

    // Wrap in interface{} - 16 bytes OVERHEAD
    // This creates the per-instance overhead (the interface box)
    // but points to the shared static data
    const { interfaceId, nodes: ifaceNodes } = this.generateInterfaceWrapper(targetNodeId, path, parentId);

    return { rootNodeId: interfaceId, nodes: ifaceNodes, relationships: [] };
  }

  private generateFloat64(value: number, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // float64 - 8 bytes DATA (Go's default for JSON numbers)
    // Floats are usually allocated on heap when boxed
    const floatId = idGen.nodeId('f64');
    nodes.push(this.createAndAddNode(floatId, 'go-float64', String(value), {
      caption: isDup ? 'float64 (DUP!) [8B]' : 'float64 [8B]',
      sizeBytes: SIZE_ESTIMATES['go-float64'],
      isDuplicate: isDup,
      isOverhead: false, // DATA!
      value,
      implementation: this.implementation,
    }));

    // Wrap in interface{} - 16 bytes OVERHEAD
    const { interfaceId, nodes: ifaceNodes } = this.generateInterfaceWrapper(floatId, path, parentId);
    nodes.push(...ifaceNodes);

    return { rootNodeId: interfaceId, nodes, relationships: [] };
  }

  /**
   * Generate string with field-level detail
   * Reference: runtime/string.go - stringStruct
   */
  private generateString(value: string, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // string header container - 16 bytes
    const { nodeId: strId } = this.context.stringTracker.track(value);
    nodes.push(this.createAndAddNode(strId, 'go-string', this.getValueLabel(value), {
      caption: isDup ? `string (DUP!) "${value}"` : `string "${value}"`,
      sizeBytes: 0, // Size comes from children
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    }));

    // str pointer - 8 bytes OVERHEAD
    const ptrId = idGen.nodeId('sptr');
    nodes.push(this.createAndAddNode(ptrId, 'go-string-ptr', 'str', {
      caption: 'str pointer [8B]',
      sizeBytes: SIZE_ESTIMATES['go-string-ptr'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), strId, ptrId, 'stringhdr-ptr');

    // len field - 8 bytes OVERHEAD (metadata, not data!)
    const lenId = idGen.nodeId('slen');
    nodes.push(this.createAndAddNode(lenId, 'go-string-len', `len=${value.length}`, {
      caption: `len [8B] = ${value.length}`,
      sizeBytes: SIZE_ESTIMATES['go-string-len'],
      isOverhead: true, // LENGTH IS OVERHEAD!
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), strId, lenId, 'stringhdr-len');

    // Heap data - actual DATA bytes
    if (value.length === 0) {
        // Point to static zerobase for empty string
        this.createAndAddRelationship(idGen.edgeId(), ptrId, this.emptyStringNodeId!, 'points-to');
    } else {
        // Allocated heap data
        const dataId = idGen.nodeId('sdata');
        const dataSize = (SIZE_ESTIMATES['go-string-data'] as (len: number) => number)(value.length);
        nodes.push(this.createAndAddNode(dataId, 'go-string-data', `"${value}"`, {
          caption: `heap data [${dataSize}B] DATA`,
          sizeBytes: dataSize,
          isOverhead: false, // THIS IS DATA!
          value,
          implementation: this.implementation,
        }));
        this.createAndAddRelationship(idGen.edgeId(), ptrId, dataId, 'points-to');
    }

    // Wrap in interface{} - 16 bytes OVERHEAD
    const { interfaceId, nodes: ifaceNodes } = this.generateInterfaceWrapper(strId, path, parentId);
    nodes.push(...ifaceNodes);

    return { rootNodeId: interfaceId, nodes, relationships: [] };
  }

  /**
   * Generate []interface{} slice with field-level detail
   * Reference: runtime/slice.go
   */
  private generateSlice(value: unknown[], path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Slice header container - 24 bytes
    const sliceId = idGen.nodeId('slice');
    nodes.push(this.createAndAddNode(sliceId, 'go-slice', `[]interface{}[${value.length}]`, {
      caption: '[]interface{} slice',
      sizeBytes: 0, // Size comes from children
      implementation: this.implementation,
    }));

    // array pointer - 8 bytes OVERHEAD
    const ptrId = idGen.nodeId('slptr');
    nodes.push(this.createAndAddNode(ptrId, 'go-slice-ptr', 'ptr', {
      caption: 'array pointer [8B]',
      sizeBytes: SIZE_ESTIMATES['go-slice-ptr'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), sliceId, ptrId, 'struct-field');

    // len field - 8 bytes OVERHEAD
    const lenId = idGen.nodeId('sllen');
    nodes.push(this.createAndAddNode(lenId, 'go-slice-len', `len=${value.length}`, {
      caption: `len [8B] = ${value.length}`,
      sizeBytes: SIZE_ESTIMATES['go-slice-len'],
      isOverhead: true, // LENGTH IS OVERHEAD!
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), sliceId, lenId, 'struct-field');

    // cap field - 8 bytes OVERHEAD
    const capId = idGen.nodeId('slcap');
    nodes.push(this.createAndAddNode(capId, 'go-slice-cap', `cap=${value.length}`, {
      caption: `cap [8B] = ${value.length}`,
      sizeBytes: SIZE_ESTIMATES['go-slice-cap'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), sliceId, capId, 'struct-field');

    // Wrap slice in interface{} - 16 bytes OVERHEAD
    const { interfaceId, nodes: ifaceNodes } = this.generateInterfaceWrapper(sliceId, path, parentId);
    nodes.push(...ifaceNodes);

    // Generate children (each wrapped in interface{})
    value.forEach((item, index) => {
      const childPath = `${path}[${index}]`;
      const result = this.generateValue(item, childPath, sliceId);
      nodes.push(...result.nodes);
    });

    return { rootNodeId: interfaceId, nodes, relationships: [] };
  }

  /**
   * Generate map[string]interface{} with full hmap + bucket detail
   * Reference: runtime/map.go
   *
   * Shows:
   * - hmap header (48 bytes, all fields)
   * - bucket structure (272 bytes fixed!)
   * - WASTED slots (8-slot allocation regardless of entry count)
   */
  private generateMap(
    value: Record<string, unknown>,
    path: string,
    parentId?: string
  ): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const keys = Object.keys(value);
    const entryCount = keys.length;

    // ==================== hmap structure (48 bytes) ====================
    const hmapId = idGen.nodeId('hmap');
    nodes.push(this.createAndAddNode(hmapId, 'go-hmap', `hmap[${entryCount}]`, {
      caption: 'map[string]interface{} hmap',
      sizeBytes: 0, // Size comes from children
      implementation: this.implementation,
    }));

    // count - 8 bytes OVERHEAD (this is the length field!)
    const countId = idGen.nodeId('hmcnt');
    nodes.push(this.createAndAddNode(countId, 'go-hmap-count', `count=${entryCount}`, {
      caption: `count [8B] = ${entryCount}`,
      sizeBytes: SIZE_ESTIMATES['go-hmap-count'],
      isOverhead: true, // LENGTH IS OVERHEAD!
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), hmapId, countId, 'hmap-count');

    // flags - 1 byte OVERHEAD
    const flagsId = idGen.nodeId('hmflg');
    nodes.push(this.createAndAddNode(flagsId, 'go-hmap-flags', 'flags', {
      caption: 'flags [1B]',
      sizeBytes: SIZE_ESTIMATES['go-hmap-flags'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), hmapId, flagsId, 'struct-field');

    // B - 1 byte OVERHEAD (log2 of bucket count)
    const bId = idGen.nodeId('hmb');
    nodes.push(this.createAndAddNode(bId, 'go-hmap-b', 'B=0', {
      caption: 'B [1B] = 0 (1 bucket)',
      sizeBytes: SIZE_ESTIMATES['go-hmap-b'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), hmapId, bId, 'hmap-b');

    // noverflow - 2 bytes OVERHEAD
    const novId = idGen.nodeId('hmnov');
    nodes.push(this.createAndAddNode(novId, 'go-hmap-noverflow', 'noverflow=0', {
      caption: 'noverflow [2B]',
      sizeBytes: SIZE_ESTIMATES['go-hmap-noverflow'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), hmapId, novId, 'struct-field');

    // hash0 - 4 bytes OVERHEAD (hash seed)
    const h0Id = idGen.nodeId('hmh0');
    nodes.push(this.createAndAddNode(h0Id, 'go-hmap-hash0', 'hash0', {
      caption: 'hash0 [4B] seed',
      sizeBytes: SIZE_ESTIMATES['go-hmap-hash0'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), hmapId, h0Id, 'hmap-hash0');

    // buckets pointer - 8 bytes OVERHEAD
    const bucketsId = idGen.nodeId('hmbkt');
    nodes.push(this.createAndAddNode(bucketsId, 'go-hmap-buckets', 'buckets', {
      caption: 'buckets pointer [8B]',
      sizeBytes: SIZE_ESTIMATES['go-hmap-buckets'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), hmapId, bucketsId, 'hmap-buckets-ptr');

    // oldbuckets pointer - 8 bytes OVERHEAD
    const oldId = idGen.nodeId('hmold');
    nodes.push(this.createAndAddNode(oldId, 'go-hmap-oldbuckets', 'oldbuckets', {
      caption: 'oldbuckets pointer [8B]',
      sizeBytes: SIZE_ESTIMATES['go-hmap-oldbuckets'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), hmapId, oldId, 'struct-field');

    // nevacuate - 8 bytes OVERHEAD
    const nevacId = idGen.nodeId('hmnev');
    nodes.push(this.createAndAddNode(nevacId, 'go-hmap-nevacuate', 'nevacuate', {
      caption: 'nevacuate [8B]',
      sizeBytes: SIZE_ESTIMATES['go-hmap-nevacuate'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), hmapId, nevacId, 'struct-field');

    // extra pointer - 8 bytes OVERHEAD
    const extraId = idGen.nodeId('hmext');
    nodes.push(this.createAndAddNode(extraId, 'go-hmap-extra', 'extra', {
      caption: 'extra pointer [8B]',
      sizeBytes: SIZE_ESTIMATES['go-hmap-extra'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), hmapId, extraId, 'struct-field');

    // ==================== bucket (bmap) structure (272 bytes) ====================
    // bucketCnt = 8 (hardcoded in Go runtime)
    const BUCKET_CNT = 8;
    const wastedSlots = BUCKET_CNT - entryCount;

    const bucketId = idGen.nodeId('bucket');
    nodes.push(this.createAndAddNode(bucketId, 'go-bucket', `bmap[${entryCount}/${BUCKET_CNT}]`, {
      caption: `bucket (${wastedSlots} WASTED slots!)`,
      sizeBytes: 0, // Size comes from children
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), bucketsId, bucketId, 'points-to');

    // ========== tophash[8] - 8 bytes ==========
    const tophashId = idGen.nodeId('toph');
    nodes.push(this.createAndAddNode(tophashId, 'go-bucket-tophash', 'tophash[8]', {
      caption: 'tophash[8] container',
      sizeBytes: 0,
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), bucketId, tophashId, 'bmap-tophash');

    // Used tophash slots
    for (let i = 0; i < entryCount; i++) {
      const thId = idGen.nodeId('th');
      nodes.push(this.createAndAddNode(thId, 'go-bucket-tophash-slot', `tophash[${i}]`, {
        caption: `tophash[${i}] [1B]`,
        sizeBytes: SIZE_ESTIMATES['go-bucket-tophash-slot'],
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), tophashId, thId, 'array-element');
    }

    // WASTED tophash slots
    for (let i = entryCount; i < BUCKET_CNT; i++) {
      const thId = idGen.nodeId('thw');
      nodes.push(this.createAndAddNode(thId, 'go-bucket-tophash-wasted', `tophash[${i}]`, {
        caption: `tophash[${i}] [1B] WASTED`,
        sizeBytes: SIZE_ESTIMATES['go-bucket-tophash-wasted'],
        isOverhead: true, // WASTED = OVERHEAD
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), tophashId, thId, 'array-element');
    }

    // ========== keys[8] region - 128 bytes ==========
    const keysId = idGen.nodeId('keys');
    nodes.push(this.createAndAddNode(keysId, 'go-bucket-keys', 'keys[8]', {
      caption: 'keys region (8×16B)',
      sizeBytes: 0,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), bucketId, keysId, 'bmap-keys');

    // ========== vals[8] region - 128 bytes ==========
    const valsId = idGen.nodeId('vals');
    nodes.push(this.createAndAddNode(valsId, 'go-bucket-vals', 'vals[8]', {
      caption: 'vals region (8×16B)',
      sizeBytes: 0,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), bucketId, valsId, 'bmap-values');

    // ========== Generate USED key-value pairs ==========
    let slotIndex = 0;
    for (const key of keys) {
      const childPath = `${path}["${key}"]`;

      // Check for duplicate key
      const isKeyDup = this.context.valueTracker.isDuplicate(key);
      this.context.valueTracker.markSeen(key);

      // Key slot in bucket (string header = 16 bytes) - OVERHEAD
      const keySlotId = idGen.nodeId('kslot');
      nodes.push(this.createAndAddNode(keySlotId, 'go-bucket-key-slot', `keys[${slotIndex}]`, {
        caption: `keys[${slotIndex}] [16B] string header`,
        sizeBytes: SIZE_ESTIMATES['go-bucket-key-slot'],
        isOverhead: true, // String header is overhead
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), keysId, keySlotId, 'array-element');

      // String for key - with ptr/len/data breakdown
      const { nodeId: keyStrId } = this.context.stringTracker.track(key);
      nodes.push(this.createAndAddNode(keyStrId, 'go-string', `"${key}"`, {
        caption: isKeyDup ? `key "${key}" (DUP!)` : `key "${key}"`,
        sizeBytes: 0,
        isDuplicate: isKeyDup,
        value: key,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), keySlotId, keyStrId, 'points-to');

      // Key string fields
      const kptrId = idGen.nodeId('kptr');
      nodes.push(this.createAndAddNode(kptrId, 'go-string-ptr', 'str', {
        caption: 'str pointer [8B]',
        sizeBytes: SIZE_ESTIMATES['go-string-ptr'],
        isOverhead: true,
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), keyStrId, kptrId, 'stringhdr-ptr');

      const klenId = idGen.nodeId('klen');
      nodes.push(this.createAndAddNode(klenId, 'go-string-len', `len=${key.length}`, {
        caption: `len [8B] = ${key.length}`,
        sizeBytes: SIZE_ESTIMATES['go-string-len'],
        isOverhead: true, // LENGTH IS OVERHEAD!
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), keyStrId, klenId, 'stringhdr-len');

      // Key heap data - DATA!
      if (key.length === 0) {
        this.createAndAddRelationship(idGen.edgeId(), kptrId, this.emptyStringNodeId!, 'points-to');
      } else {
        const kdataId = idGen.nodeId('kdata');
        const keyDataSize = (SIZE_ESTIMATES['go-string-data'] as (len: number) => number)(key.length);
        nodes.push(this.createAndAddNode(kdataId, 'go-string-data', `"${key}"`, {
            caption: `heap [${keyDataSize}B] DATA`,
            sizeBytes: keyDataSize,
            isOverhead: false, // THIS IS DATA!
            value: key,
            implementation: this.implementation,
        }));
        this.createAndAddRelationship(idGen.edgeId(), kptrId, kdataId, 'points-to');
      }

      // Value slot in bucket (interface{} = 16 bytes) - OVERHEAD
      const valSlotId = idGen.nodeId('vslot');
      nodes.push(this.createAndAddNode(valSlotId, 'go-bucket-val-slot', `vals[${slotIndex}]`, {
        caption: `vals[${slotIndex}] [16B] interface{}`,
        sizeBytes: SIZE_ESTIMATES['go-bucket-val-slot'],
        isOverhead: true, // interface{} slot is overhead
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), valsId, valSlotId, 'array-element');

      // Generate value (wrapped in interface{})
      const result = this.generateValue(value[key], childPath, valSlotId);
      nodes.push(...result.nodes);

      slotIndex++;
    }

    // ========== Generate WASTED slots ==========
    for (let i = entryCount; i < BUCKET_CNT; i++) {
      // WASTED key slot - 16 bytes OVERHEAD
      const wkId = idGen.nodeId('wk');
      nodes.push(this.createAndAddNode(wkId, 'go-bucket-wasted-key', `keys[${i}]`, {
        caption: `keys[${i}] [16B] WASTED`,
        sizeBytes: SIZE_ESTIMATES['go-bucket-wasted-key'],
        isOverhead: true, // WASTED = OVERHEAD!
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), keysId, wkId, 'array-element');

      // WASTED val slot - 16 bytes OVERHEAD
      const wvId = idGen.nodeId('wv');
      nodes.push(this.createAndAddNode(wvId, 'go-bucket-wasted-val', `vals[${i}]`, {
        caption: `vals[${i}] [16B] WASTED`,
        sizeBytes: SIZE_ESTIMATES['go-bucket-wasted-val'],
        isOverhead: true, // WASTED = OVERHEAD!
        implementation: this.implementation,
      }));
      this.createAndAddRelationship(idGen.edgeId(), valsId, wvId, 'array-element');
    }

    // overflow pointer - 8 bytes OVERHEAD
    const ovfId = idGen.nodeId('ovf');
    nodes.push(this.createAndAddNode(ovfId, 'go-bucket-overflow', 'overflow', {
      caption: 'overflow pointer [8B]',
      sizeBytes: SIZE_ESTIMATES['go-bucket-overflow'],
      isOverhead: true,
      implementation: this.implementation,
    }));
    this.createAndAddRelationship(idGen.edgeId(), bucketId, ovfId, 'bmap-overflow');

    // Wrap hmap in interface{} - 16 bytes OVERHEAD
    const { interfaceId, nodes: ifaceNodes } = this.generateInterfaceWrapper(hmapId, path, parentId);
    nodes.push(...ifaceNodes);

    return { rootNodeId: interfaceId, nodes, relationships: [] };
  }
}
