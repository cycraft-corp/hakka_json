import type { ImplementationId, NvlNode } from '../types/graph';
import type { GenerationResult, StringTracker, IdGenerator } from './types';
import { estimateSize } from './types';
import { BaseGenerator } from './base';
import { SequentialIdGenerator } from './utils/id-generator';
import { DuplicatingStringTracker } from './utils/string-tracker';

/**
 * Jansson Graph Generator (C)
 *
 * Key characteristics:
 * - json_t union type with type tag
 * - Hashtable for objects (separate chaining)
 * - Simple array storage
 * - No string interning (duplicates)
 * - Reference counting
 */
export class JanssonGenerator extends BaseGenerator {
  readonly id: ImplementationId = 'jansson';
  readonly name = 'Jansson';
  protected readonly implementation = 'jansson' as const;

  protected getLanguage(): string {
    return 'C';
  }

  protected getDescription(): string {
    return 'json_t union, hashtable, refcounting';
  }

  protected createStringTracker(idGen: () => string): StringTracker {
    // Jansson doesn't intern strings
    return new DuplicatingStringTracker(idGen);
  }

  protected createIdGenerator(): IdGenerator {
    return new SequentialIdGenerator('jan-');
  }

  protected generateInfrastructure(): void {
    // Jansson has no special infrastructure
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

    // json_t with JSON_NULL type
    const nodeId = idGen.nodeId('null');
    const node = this.createAndAddNode(nodeId, 'jansson-null', 'null', {
      caption: isDup ? `json_t (null) @ ${path} (DUP!)` : `json_t (null) @ ${path}`,
      sizeBytes: estimateSize('jansson-null'),
      refCount: 1,
      isDuplicate: isDup,
      implementation: this.implementation,
    });
    nodes.push(node);

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, nodeId, 'contains');
    }

    return { rootNodeId: nodeId, nodes, relationships: [] };
  }

  private generateBool(value: boolean, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // json_t with JSON_TRUE or JSON_FALSE type
    const nodeId = idGen.nodeId('bool');
    const nodeType = value ? 'jansson-true' : 'jansson-false';
    const node = this.createAndAddNode(nodeId, nodeType, String(value), {
      caption: isDup ? `json_t (${value}) @ ${path} (DUP!)` : `json_t (${value}) @ ${path}`,
      sizeBytes: estimateSize(nodeType),
      refCount: 1,
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    nodes.push(node);

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, nodeId, 'contains');
    }

    return { rootNodeId: nodeId, nodes, relationships: [] };
  }

  private generateNumber(value: number, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // json_t with JSON_INTEGER or JSON_REAL
    const isInteger = Number.isInteger(value);
    const nodeId = idGen.nodeId(isInteger ? 'int' : 'real');
    const nodeType = isInteger ? 'jansson-integer' : 'jansson-real';

    const node = this.createAndAddNode(nodeId, nodeType, String(value), {
      caption: isDup ? `json_t (${isInteger ? 'integer' : 'real'}) @ ${path} (DUP!)` : `json_t (${isInteger ? 'integer' : 'real'}) @ ${path}`,
      sizeBytes: estimateSize(nodeType),
      refCount: 1,
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    nodes.push(node);

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, nodeId, 'contains');
    }

    return { rootNodeId: nodeId, nodes, relationships: [] };
  }

  private generateString(value: string, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // json_t with JSON_STRING type
    const { nodeId: strId } = this.context.stringTracker.track(value);
    const strNode = this.createAndAddNode(strId, 'jansson-string', this.getValueLabel(value), {
      caption: isDup ? `json_t (string) @ ${path} (DUP!)` : `json_t (string) @ ${path}`,
      sizeBytes: estimateSize('jansson-string', value),
      refCount: 1,
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    nodes.push(strNode);

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, strId, 'contains');
    }

    return { rootNodeId: strId, nodes, relationships: [] };
  }

  private generateArray(value: unknown[], path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // json_t with JSON_ARRAY type
    const arrayId = idGen.nodeId('arr');
    const arrayNode = this.createAndAddNode(arrayId, 'jansson-array', `array[${value.length}]`, {
      caption: `json_t (array) @ ${path}`,
      sizeBytes: estimateSize('jansson-array', value),
      refCount: 1,
      implementation: this.implementation,
    });
    nodes.push(arrayNode);

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, arrayId, 'contains');
    }

    // Generate children
    value.forEach((item, index) => {
      const childPath = `${path}[${index}]`;
      const result = this.generateValue(item, childPath, arrayId);
      nodes.push(...result.nodes);
    });

    return { rootNodeId: arrayId, nodes, relationships: [] };
  }

  private generateObject(
    value: Record<string, unknown>,
    path: string,
    parentId?: string
  ): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const keys = Object.keys(value);

    // json_t with JSON_OBJECT type
    const objId = idGen.nodeId('obj');
    const objNode = this.createAndAddNode(objId, 'jansson-object', `object{${keys.length}}`, {
      caption: `json_t (object) @ ${path}`,
      sizeBytes: estimateSize('jansson-object'),
      refCount: 1,
      implementation: this.implementation,
    });
    nodes.push(objNode);

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, objId, 'contains');
    }

    // Hashtable for object storage (structural overhead)
    const htId = idGen.nodeId('ht');
    const htNode = this.createAndAddNode(htId, 'jansson-hashtable', `hashtable[${keys.length}]`, {
      caption: 'Hashtable',
      sizeBytes: estimateSize('jansson-hashtable', value),
      isOverhead: true,
      implementation: this.implementation,
    });
    nodes.push(htNode);

    this.createAndAddRelationship(idGen.edgeId(), objId, htId, 'has-hashtable');

    // Generate key-value pairs
    for (const key of keys) {
      const childPath = `${path}.${key}`;

      // Pair structure (structural overhead)
      const pairId = idGen.nodeId('pair');
      const pairNode = this.createAndAddNode(pairId, 'jansson-pair', `"${key}":...`, {
        caption: 'Key-Value Pair',
        sizeBytes: estimateSize('jansson-pair'),
        isOverhead: true,
        implementation: this.implementation,
      });
      nodes.push(pairNode);

      this.createAndAddRelationship(idGen.edgeId(), htId, pairId, 'has-pair');

      // Check for duplicate key string
      const isKeyDup = this.context.valueTracker.isDuplicate(key);
      this.context.valueTracker.markSeen(key);

      // Key string (duplicated, not interned)
      const { nodeId: keyId } = this.context.stringTracker.track(key);
      const keyNode = this.createAndAddNode(keyId, 'jansson-string', `"${key}"`, {
        caption: isKeyDup ? 'Pair key (DUP!)' : 'Pair key',
        sizeBytes: estimateSize('jansson-string', key),
        isDuplicate: isKeyDup,
        value: key,
        implementation: this.implementation,
      });
      nodes.push(keyNode);

      this.createAndAddRelationship(idGen.edgeId(), pairId, keyId, 'has-key');

      // Generate value
      const result = this.generateValue(value[key], childPath, pairId);
      nodes.push(...result.nodes);
    }

    return { rootNodeId: objId, nodes, relationships: [] };
  }
}
