import type { ImplementationId, NvlNode } from '../types/graph';
import type { GenerationResult, StringTracker, IdGenerator } from './types';
import { estimateSize } from './types';
import { BaseGenerator } from './base';
import { SequentialIdGenerator } from './utils/id-generator';
import { DuplicatingStringTracker } from './utils/string-tracker';

/**
 * Go encoding/json Graph Generator
 *
 * Key characteristics:
 * - interface{} wrapper for EVERY value (type + data pointers = 16 bytes)
 * - map[string]interface{} with hmap + buckets
 * - []interface{} slices
 * - string headers (pointer + length)
 * - High overhead from interface{} boxing
 */
export class GoJsonGenerator extends BaseGenerator {
  readonly id: ImplementationId = 'go_json';
  readonly name = 'Go encoding/json';
  protected readonly implementation = 'go' as const;

  protected getLanguage(): string {
    return 'Go';
  }

  protected getDescription(): string {
    return 'interface{} boxing, hmap, no string interning';
  }

  protected createStringTracker(idGen: () => string): StringTracker {
    // Go doesn't intern strings
    return new DuplicatingStringTracker(idGen);
  }

  protected createIdGenerator(): IdGenerator {
    return new SequentialIdGenerator('go-');
  }

  protected generateInfrastructure(): void {
    // Go has no special infrastructure
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

  private wrapInInterface(
    valueNodeId: string,
    path: string,
    parentId?: string
  ): string {
    const idGen = this.context.idGenerator;

    // interface{} wrapper (16 bytes overhead)
    const interfaceId = idGen.nodeId('iface');
    this.createAndAddNode(interfaceId, 'go-interface', 'interface{}', {
      caption: `interface{} @ ${path}`,
      sizeBytes: 16,
      isOverhead: true,
      implementation: this.implementation,
    });

    // Link interface to value
    this.createAndAddRelationship(idGen.edgeId(), interfaceId, valueNodeId, 'wraps');

    // Link from parent
    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, interfaceId, 'contains');
    }

    return interfaceId;
  }

  private generateNil(path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // nil interface{} - just the wrapper, no underlying value
    const interfaceId = idGen.nodeId('iface');
    const interfaceNode = this.createAndAddNode(interfaceId, 'go-interface', 'nil', {
      caption: `nil interface{} @ ${path}`,
      sizeBytes: 16,
      isOverhead: true,
      implementation: this.implementation,
    });
    nodes.push(interfaceNode);

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, interfaceId, 'contains');
    }

    return { rootNodeId: interfaceId, nodes, relationships: [] };
  }

  private generateBool(value: boolean, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // bool value
    const boolId = idGen.nodeId('bool');
    const boolNode = this.createAndAddNode(boolId, 'go-bool', String(value), {
      caption: isDup ? 'bool (DUP!)' : 'bool',
      sizeBytes: 1,
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    nodes.push(boolNode);

    // Wrap in interface{}
    const interfaceId = this.wrapInInterface(boolId, path, parentId);

    return { rootNodeId: interfaceId, nodes, relationships: [] };
  }

  private generateFloat64(value: number, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // float64 (Go's default for JSON numbers)
    const floatId = idGen.nodeId('f64');
    const floatNode = this.createAndAddNode(floatId, 'go-float64', String(value), {
      caption: isDup ? 'float64 (DUP!)' : 'float64',
      sizeBytes: 8,
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    nodes.push(floatNode);

    // Wrap in interface{}
    const interfaceId = this.wrapInInterface(floatId, path, parentId);

    return { rootNodeId: interfaceId, nodes, relationships: [] };
  }

  private generateString(value: string, path: string, parentId?: string): GenerationResult {
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // string (header + data)
    const { nodeId: strId } = this.context.stringTracker.track(value);
    const strNode = this.createAndAddNode(strId, 'go-string', this.getValueLabel(value), {
      caption: isDup ? 'string (DUP!)' : 'string',
      sizeBytes: estimateSize('go-string', value),
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    nodes.push(strNode);

    // Wrap in interface{}
    const interfaceId = this.wrapInInterface(strId, path, parentId);

    return { rootNodeId: interfaceId, nodes, relationships: [] };
  }

  private generateSlice(value: unknown[], path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // []interface{} slice header
    const sliceId = idGen.nodeId('slice');
    const sliceNode = this.createAndAddNode(sliceId, 'go-slice', `[]interface{}[${value.length}]`, {
      caption: '[]interface{}',
      sizeBytes: estimateSize('go-slice', value),
      implementation: this.implementation,
    });
    nodes.push(sliceNode);

    // Wrap slice in interface{}
    const interfaceId = this.wrapInInterface(sliceId, path, parentId);

    // Generate children (each wrapped in interface{})
    value.forEach((item, index) => {
      const childPath = `${path}[${index}]`;
      const result = this.generateValue(item, childPath, sliceId);
      nodes.push(...result.nodes);
    });

    return { rootNodeId: interfaceId, nodes, relationships: [] };
  }

  private generateMap(
    value: Record<string, unknown>,
    path: string,
    parentId?: string
  ): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const keys = Object.keys(value);

    // map[string]interface{} - hmap structure
    const hmapId = idGen.nodeId('hmap');
    const hmapNode = this.createAndAddNode(hmapId, 'go-hmap', `map[${keys.length}]`, {
      caption: 'map[string]interface{}',
      sizeBytes: estimateSize('go-hmap'),
      implementation: this.implementation,
    });
    nodes.push(hmapNode);

    // Wrap hmap in interface{}
    const interfaceId = this.wrapInInterface(hmapId, path, parentId);

    // Create buckets (simplified - one bucket for small maps)
    const bucketId = idGen.nodeId('bucket');
    const bucketNode = this.createAndAddNode(bucketId, 'go-bucket', `bucket[${keys.length}]`, {
      caption: 'map bucket',
      sizeBytes: estimateSize('go-bucket', value),
      implementation: this.implementation,
    });
    nodes.push(bucketNode);

    this.createAndAddRelationship(idGen.edgeId(), hmapId, bucketId, 'has-bucket');

    // Generate key-value pairs
    for (const key of keys) {
      const childPath = `${path}["${key}"]`;

      // Check for duplicate key string
      const isKeyDup = this.context.valueTracker.isDuplicate(key);
      this.context.valueTracker.markSeen(key);

      // Key string
      const { nodeId: keyId } = this.context.stringTracker.track(key);
      const keyNode = this.createAndAddNode(keyId, 'go-string', `"${key}"`, {
        caption: isKeyDup ? 'map key (DUP!)' : 'map key',
        sizeBytes: estimateSize('go-string', key),
        isDuplicate: isKeyDup,
        value: key,
        implementation: this.implementation,
      });
      nodes.push(keyNode);

      this.createAndAddRelationship(idGen.edgeId(), bucketId, keyId, 'has-key');

      // Generate value (wrapped in interface{})
      const result = this.generateValue(value[key], childPath, bucketId);
      nodes.push(...result.nodes);
    }

    return { rootNodeId: interfaceId, nodes, relationships: [] };
  }
}
