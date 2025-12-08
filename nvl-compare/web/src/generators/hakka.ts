import type { ImplementationId, NvlNode } from '../types/graph';
import type { GenerationResult, StringTracker, IdGenerator } from './types';
import { estimateSize } from './types';
import { BaseGenerator } from './base';
import { SequentialIdGenerator } from './utils/id-generator';
import { InternedStringTracker } from './utils/string-tracker';

/**
 * HakkaJson Graph Generator
 *
 * Key characteristics:
 * - String interning: identical strings share a single allocation
 * - NaN-boxing: scalars stored in 8-byte handles without heap allocation
 * - Central registry with typed managers
 * - Minimal overhead compared to other implementations
 */
export class HakkaJsonGenerator extends BaseGenerator {
  readonly id: ImplementationId = 'hakka_json';
  readonly name = 'HakkaJson';
  protected readonly implementation = 'hakka' as const;

  protected getLanguage(): string {
    return 'C++';
  }

  protected getDescription(): string {
    return 'String interning, NaN-boxing, minimal overhead';
  }

  private registryId = '';
  private scalarManagerId = '';
  private stringManagerId = '';
  private arrayManagerId = '';
  private objectManagerId = '';

  protected createStringTracker(idGen: () => string): StringTracker {
    // HakkaJson uses string interning
    return new InternedStringTracker(idGen);
  }

  protected createIdGenerator(): IdGenerator {
    return new SequentialIdGenerator('hakka-');
  }

  protected generateInfrastructure(): void {
    const idGen = this.context.idGenerator;

    // Create central registry (infrastructure overhead)
    this.registryId = idGen.nodeId('reg');
    this.createAndAddNode(this.registryId, 'hakka-registry', 'Registry', {
      caption: 'Central Value Registry',
      sizeBytes: estimateSize('hakka-registry'),
      isOverhead: true,
      implementation: this.implementation,
    });

    // Create typed managers (infrastructure overhead)
    this.scalarManagerId = idGen.nodeId('smgr');
    this.createAndAddNode(this.scalarManagerId, 'hakka-scalar-manager', 'ScalarMgr', {
      caption: 'Scalar Manager (NaN-boxing)',
      sizeBytes: estimateSize('hakka-scalar-manager'),
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(
      idGen.edgeId(),
      this.registryId,
      this.scalarManagerId,
      'manages'
    );

    this.stringManagerId = idGen.nodeId('strmgr');
    this.createAndAddNode(this.stringManagerId, 'hakka-string-manager', 'StringMgr', {
      caption: 'String Manager (Interning)',
      sizeBytes: estimateSize('hakka-string-manager'),
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(
      idGen.edgeId(),
      this.registryId,
      this.stringManagerId,
      'manages'
    );

    this.arrayManagerId = idGen.nodeId('arrmgr');
    this.createAndAddNode(this.arrayManagerId, 'hakka-array-manager', 'ArrayMgr', {
      caption: 'Array Manager',
      sizeBytes: estimateSize('hakka-array-manager'),
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(
      idGen.edgeId(),
      this.registryId,
      this.arrayManagerId,
      'manages'
    );

    this.objectManagerId = idGen.nodeId('objmgr');
    this.createAndAddNode(this.objectManagerId, 'hakka-object-manager', 'ObjectMgr', {
      caption: 'Object Manager',
      sizeBytes: estimateSize('hakka-object-manager'),
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(
      idGen.edgeId(),
      this.registryId,
      this.objectManagerId,
      'manages'
    );
  }

  protected generateValue(
    value: unknown,
    path: string,
    parentId?: string
  ): GenerationResult {
    const type = this.getJsonType(value);

    switch (type) {
      case 'null':
      case 'boolean':
      case 'number':
        return this.generateScalar(value, path, parentId);
      case 'string':
        return this.generateString(value as string, path, parentId);
      case 'array':
        return this.generateArray(value as unknown[], path, parentId);
      case 'object':
        return this.generateObject(value as Record<string, unknown>, path, parentId);
    }
  }

  private generateScalar(
    value: unknown,
    path: string,
    parentId?: string
  ): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Create handle (4 bytes token: type bits + index bits)
    const handleId = idGen.nodeId('h');
    const handleNode = this.createAndAddNode(handleId, 'hakka-handle', this.getValueLabel(value), {
      caption: `Handle @ ${path}`,
      sizeBytes: 4,
      value,
      implementation: this.implementation,
    });
    nodes.push(handleNode);

    // Create NaN-boxed value node
    const nanBoxedId = idGen.nodeId('nb');
    const nanBoxedNode = this.createAndAddNode(nanBoxedId, 'hakka-nan-boxed', String(value), {
      caption: 'NaN-boxed scalar',
      sizeBytes: 8,
      value,
      implementation: this.implementation,
    });
    nodes.push(nanBoxedNode);

    // Link handle to NaN-boxed value
    this.createAndAddRelationship(idGen.edgeId(), handleId, nanBoxedId, 'contains');

    // Link to scalar manager
    this.createAndAddRelationship(idGen.edgeId(), this.scalarManagerId, handleId, 'owns');

    // Link from parent if exists
    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, handleId, 'contains');
    }

    return { rootNodeId: handleId, nodes, relationships: [] };
  }

  private generateString(
    value: string,
    path: string,
    parentId?: string
  ): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Create handle (4 bytes token: type bits + index bits)
    const handleId = idGen.nodeId('h');
    const handleNode = this.createAndAddNode(handleId, 'hakka-handle', this.getValueLabel(value), {
      caption: `Handle @ ${path}`,
      sizeBytes: 4,
      implementation: this.implementation,
    });
    nodes.push(handleNode);

    // Track string for interning
    const { nodeId: stringNodeId, isNew } = this.context.stringTracker.track(value);

    if (isNew) {
      // Create new interned string node
      const stringNode = this.createAndAddNode(stringNodeId, 'hakka-interned-string', this.getValueLabel(value), {
        caption: 'Interned string',
        sizeBytes: estimateSize('hakka-interned-string', value),
        isInterned: true,
        value,
        implementation: this.implementation,
      });
      nodes.push(stringNode);

      // Link to string manager
      this.createAndAddRelationship(idGen.edgeId(), this.stringManagerId, stringNodeId, 'interns');
    }

    // Link handle to string (shared if duplicate)
    this.createAndAddRelationship(idGen.edgeId(), handleId, stringNodeId, 'references');

    // Link from parent if exists
    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, handleId, 'contains');
    }

    return { rootNodeId: handleId, nodes, relationships: [] };
  }

  private generateArray(
    value: unknown[],
    path: string,
    parentId?: string
  ): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Create handle (4 bytes token: type bits + index bits)
    const handleId = idGen.nodeId('h');
    const handleNode = this.createAndAddNode(handleId, 'hakka-handle', `Array[${value.length}]`, {
      caption: `Handle @ ${path}`,
      sizeBytes: 4,
      implementation: this.implementation,
    });
    nodes.push(handleNode);

    // JsonArrayCompact: vector<JsonHandleCompact> + refcount
    // No separate storage - the array IS the object containing handles directly
    const arrayId = idGen.nodeId('arr');
    const arrayNode = this.createAndAddNode(arrayId, 'hakka-array', `[${value.length}]`, {
      caption: `JsonArrayCompact (vec<handle> + refcount)`,
      sizeBytes: estimateSize('hakka-array', value),
      implementation: this.implementation,
    });
    nodes.push(arrayNode);

    // Link handle to array object
    this.createAndAddRelationship(idGen.edgeId(), handleId, arrayId, 'points-to');

    // Link to array manager
    this.createAndAddRelationship(idGen.edgeId(), this.arrayManagerId, handleId, 'owns');

    // Generate children - parent is the array itself
    value.forEach((item, index) => {
      const childPath = `${path}[${index}]`;
      const result = this.generateValue(item, childPath, arrayId);
      nodes.push(...result.nodes);
    });

    // Link from parent if exists
    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, handleId, 'contains');
    }

    return { rootNodeId: handleId, nodes, relationships: [] };
  }

  private generateObject(
    value: Record<string, unknown>,
    path: string,
    parentId?: string
  ): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const keys = Object.keys(value);

    // Create handle (4 bytes token: type bits + index bits)
    const handleId = idGen.nodeId('h');
    const handleNode = this.createAndAddNode(handleId, 'hakka-handle', `Object{${keys.length}}`, {
      caption: `Handle @ ${path}`,
      sizeBytes: 4,
      implementation: this.implementation,
    });
    nodes.push(handleNode);

    // JsonObjectCompact: just 2 handles (keys array, values array) + refcount
    // The actual keys and values are stored in separate JsonArrayCompact instances
    const objectId = idGen.nodeId('obj');
    const objectNode = this.createAndAddNode(objectId, 'hakka-object', `{${keys.length}}`, {
      caption: `JsonObjectCompact (keys + values handles)`,
      sizeBytes: estimateSize('hakka-object'),
      implementation: this.implementation,
    });
    nodes.push(objectNode);

    // Link handle to object
    this.createAndAddRelationship(idGen.edgeId(), handleId, objectId, 'points-to');

    // Link to object manager
    this.createAndAddRelationship(idGen.edgeId(), this.objectManagerId, handleId, 'owns');

    // Create keys array (JsonArrayCompact storing string handles)
    const keysArrayId = idGen.nodeId('keys');
    const keysArrayNode = this.createAndAddNode(keysArrayId, 'hakka-array', `keys[${keys.length}]`, {
      caption: 'Keys array',
      sizeBytes: estimateSize('hakka-array', keys),
      implementation: this.implementation,
    });
    nodes.push(keysArrayNode);
    this.createAndAddRelationship(idGen.edgeId(), objectId, keysArrayId, 'contains');

    // Create values array (JsonArrayCompact storing value handles)
    const valuesArrayId = idGen.nodeId('vals');
    const valuesArrayNode = this.createAndAddNode(valuesArrayId, 'hakka-array', `values[${keys.length}]`, {
      caption: 'Values array',
      sizeBytes: estimateSize('hakka-array', keys),
      implementation: this.implementation,
    });
    nodes.push(valuesArrayNode);
    this.createAndAddRelationship(idGen.edgeId(), objectId, valuesArrayId, 'contains');

    // Generate key-value pairs
    for (const key of keys) {
      const childPath = `${path}.${key}`;

      // Key is an interned string
      const { nodeId: keyNodeId, isNew: keyIsNew } = this.context.stringTracker.track(key);

      if (keyIsNew) {
        const keyNode = this.createAndAddNode(keyNodeId, 'hakka-interned-string', `"${key}"`, {
          caption: 'Interned key',
          sizeBytes: estimateSize('hakka-interned-string', key),
          isInterned: true,
          value: key,
          implementation: this.implementation,
        });
        nodes.push(keyNode);
        this.createAndAddRelationship(idGen.edgeId(), this.stringManagerId, keyNodeId, 'interns');
      }

      // Link keys array to key
      this.createAndAddRelationship(idGen.edgeId(), keysArrayId, keyNodeId, 'contains');

      // Generate value and link to values array
      const result = this.generateValue(value[key], childPath, valuesArrayId);
      nodes.push(...result.nodes);
    }

    // Link from parent if exists
    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, handleId, 'contains');
    }

    return { rootNodeId: handleId, nodes, relationships: [] };
  }
}
