import type { ImplementationId, NvlNode } from '../types/graph';
import type { GenerationResult, StringTracker, IdGenerator } from './types';
import { estimateSize } from './types';
import { BaseGenerator } from './base';
import { SequentialIdGenerator } from './utils/id-generator';
import { DuplicatingStringTracker } from './utils/string-tracker';

/**
 * serde_json Graph Generator (Rust)
 *
 * Key characteristics:
 * - Value enum: tagged union for all JSON types
 * - Heap-allocated strings (no interning by default)
 * - IndexMap for object key ordering
 * - Vec for arrays
 */
export class SerdeJsonGenerator extends BaseGenerator {
  readonly id: ImplementationId = 'serde_json';
  readonly name = 'serde_json';
  protected readonly implementation = 'serde' as const;

  protected getLanguage(): string {
    return 'Rust';
  }

  protected getDescription(): string {
    return 'Tagged enum Value, no string interning';
  }

  protected createStringTracker(idGen: () => string): StringTracker {
    // serde_json doesn't intern strings
    return new DuplicatingStringTracker(idGen);
  }

  protected createIdGenerator(): IdGenerator {
    return new SequentialIdGenerator('serde-');
  }

  protected generateInfrastructure(): void {
    // serde_json has no central registry/infrastructure
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

  private generateNull(path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(null);
    this.context.valueTracker.markSeen(null);

    // Value::Null variant
    const valueId = idGen.nodeId('val');
    const valueNode = this.createAndAddNode(valueId, 'serde-value-enum', 'Null', {
      caption: isDup ? `Value::Null @ ${path} (DUP!)` : `Value::Null @ ${path}`,
      sizeBytes: estimateSize('serde-value-enum'),
      isOverhead: true, // Enum tag overhead
      isDuplicate: isDup,
      implementation: this.implementation,
    });
    nodes.push(valueNode);

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, valueId, 'contains');
    }

    return { rootNodeId: valueId, nodes, relationships: [] };
  }

  private generateBool(value: boolean, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // Value::Bool variant (enum wrapper overhead)
    const valueId = idGen.nodeId('val');
    const valueNode = this.createAndAddNode(valueId, 'serde-value-enum', String(value), {
      caption: isDup ? `Value::Bool @ ${path} (DUP!)` : `Value::Bool @ ${path}`,
      sizeBytes: estimateSize('serde-value-enum'),
      isOverhead: true,
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    nodes.push(valueNode);

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, valueId, 'contains');
    }

    return { rootNodeId: valueId, nodes, relationships: [] };
  }

  private generateNumber(value: number, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // Value::Number variant (enum wrapper overhead)
    const valueId = idGen.nodeId('val');
    const valueNode = this.createAndAddNode(valueId, 'serde-value-enum', String(value), {
      caption: isDup ? `Value::Number @ ${path} (DUP!)` : `Value::Number @ ${path}`,
      sizeBytes: estimateSize('serde-value-enum'),
      isOverhead: true,
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    nodes.push(valueNode);

    // Number inner representation
    const numberId = idGen.nodeId('num');
    const numberNode = this.createAndAddNode(numberId, 'serde-number', String(value), {
      caption: isDup ? 'i64/u64/f64 (DUP!)' : (Number.isInteger(value) ? 'i64/u64' : 'f64'),
      sizeBytes: estimateSize('serde-number'),
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    nodes.push(numberNode);

    this.createAndAddRelationship(idGen.edgeId(), valueId, numberId, 'contains');

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, valueId, 'contains');
    }

    return { rootNodeId: valueId, nodes, relationships: [] };
  }

  private generateString(value: string, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // Value::String variant (enum wrapper overhead)
    const valueId = idGen.nodeId('val');
    const valueNode = this.createAndAddNode(valueId, 'serde-value-enum', this.getValueLabel(value), {
      caption: isDup ? `Value::String @ ${path} (DUP!)` : `Value::String @ ${path}`,
      sizeBytes: estimateSize('serde-value-enum'),
      isOverhead: true,
      isDuplicate: isDup,
      implementation: this.implementation,
    });
    nodes.push(valueNode);

    // Heap-allocated String
    const { nodeId: stringId } = this.context.stringTracker.track(value);
    const stringNode = this.createAndAddNode(stringId, 'serde-string', this.getValueLabel(value), {
      caption: isDup ? 'Heap String (DUP!)' : 'Heap String',
      sizeBytes: estimateSize('serde-string', value),
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    nodes.push(stringNode);

    this.createAndAddRelationship(idGen.edgeId(), valueId, stringId, 'owns');

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, valueId, 'contains');
    }

    return { rootNodeId: valueId, nodes, relationships: [] };
  }

  private generateArray(value: unknown[], path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Value::Array variant (enum wrapper overhead)
    const valueId = idGen.nodeId('val');
    const valueNode = this.createAndAddNode(valueId, 'serde-value-enum', `Array[${value.length}]`, {
      caption: `Value::Array @ ${path}`,
      sizeBytes: estimateSize('serde-value-enum'),
      isOverhead: true,
      implementation: this.implementation,
    });
    nodes.push(valueNode);

    // Vec<Value>
    const vecId = idGen.nodeId('vec');
    const vecNode = this.createAndAddNode(vecId, 'serde-array', `Vec[${value.length}]`, {
      caption: 'Vec<Value>',
      sizeBytes: estimateSize('serde-array', value),
      implementation: this.implementation,
    });
    nodes.push(vecNode);

    this.createAndAddRelationship(idGen.edgeId(), valueId, vecId, 'owns');

    // Generate children
    value.forEach((item, index) => {
      const childPath = `${path}[${index}]`;
      const result = this.generateValue(item, childPath, vecId);
      nodes.push(...result.nodes);
    });

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, valueId, 'contains');
    }

    return { rootNodeId: valueId, nodes, relationships: [] };
  }

  private generateObject(
    value: Record<string, unknown>,
    path: string,
    parentId?: string
  ): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const keys = Object.keys(value);

    // Value::Object variant (enum wrapper overhead)
    const valueId = idGen.nodeId('val');
    const valueNode = this.createAndAddNode(valueId, 'serde-value-enum', `Object{${keys.length}}`, {
      caption: `Value::Object @ ${path}`,
      sizeBytes: estimateSize('serde-value-enum'),
      isOverhead: true,
      implementation: this.implementation,
    });
    nodes.push(valueNode);

    // IndexMap<String, Value>
    const mapId = idGen.nodeId('map');
    const mapNode = this.createAndAddNode(mapId, 'serde-indexmap', `IndexMap{${keys.length}}`, {
      caption: 'IndexMap<String, Value>',
      sizeBytes: estimateSize('serde-indexmap', value),
      implementation: this.implementation,
    });
    nodes.push(mapNode);

    this.createAndAddRelationship(idGen.edgeId(), valueId, mapId, 'owns');

    // Generate key-value pairs
    for (const key of keys) {
      const childPath = `${path}.${key}`;

      // Check for duplicate key string
      const isKeyDup = this.context.valueTracker.isDuplicate(key);
      this.context.valueTracker.markSeen(key);

      // Key string (heap allocated, may be duplicate)
      const { nodeId: keyId } = this.context.stringTracker.track(key);
      const keyNode = this.createAndAddNode(keyId, 'serde-string', `"${key}"`, {
        caption: isKeyDup ? 'Key String (DUP!)' : 'Key String',
        sizeBytes: estimateSize('serde-string', key),
        isDuplicate: isKeyDup,
        value: key,
        implementation: this.implementation,
      });
      nodes.push(keyNode);

      this.createAndAddRelationship(idGen.edgeId(), mapId, keyId, 'has-key');

      // Generate value
      const result = this.generateValue(value[key], childPath, mapId);
      nodes.push(...result.nodes);
    }

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, valueId, 'contains');
    }

    return { rootNodeId: valueId, nodes, relationships: [] };
  }
}
