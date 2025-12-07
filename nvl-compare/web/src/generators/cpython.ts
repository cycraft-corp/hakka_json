import type { ImplementationId, NvlNode } from '../types/graph';
import type { GenerationResult, StringTracker, IdGenerator } from './types';
import { estimateSize } from './types';
import { BaseGenerator } from './base';
import { SequentialIdGenerator } from './utils/id-generator';
import { DuplicatingStringTracker } from './utils/string-tracker';

/**
 * CPython json module Graph Generator
 *
 * Key characteristics:
 * - PyObject header on EVERY value (refcount + type pointer)
 * - PyDict for objects with complex hash table
 * - PyList for arrays
 * - PyUnicode for strings (complex internal structure)
 * - PyLong for integers (arbitrary precision)
 * - High overhead due to object model
 */
export class CPythonJsonGenerator extends BaseGenerator {
  readonly id: ImplementationId = 'cpython_json';
  readonly name = 'CPython json';
  protected readonly implementation = 'cpython' as const;

  protected getLanguage(): string {
    return 'Python';
  }

  protected getDescription(): string {
    return 'PyObject overhead, refcounting, no string interning';
  }

  protected createStringTracker(idGen: () => string): StringTracker {
    // CPython may intern some strings but we simulate without
    return new DuplicatingStringTracker(idGen);
  }

  protected createIdGenerator(): IdGenerator {
    return new SequentialIdGenerator('py-');
  }

  protected generateInfrastructure(): void {
    // CPython has no special infrastructure for JSON
  }

  protected generateValue(
    value: unknown,
    path: string,
    parentId?: string
  ): GenerationResult {
    const type = this.getJsonType(value);

    switch (type) {
      case 'null':
        return this.generateNone(path, parentId);
      case 'boolean':
        return this.generateBool(value as boolean, path, parentId);
      case 'number':
        return this.generateNumber(value as number, path, parentId);
      case 'string':
        return this.generateString(value as string, path, parentId);
      case 'array':
        return this.generateList(value as unknown[], path, parentId);
      case 'object':
        return this.generateDict(value as Record<string, unknown>, path, parentId);
    }
  }

  private addPyObjectOverhead(
    nodeId: string,
    parentId: string | undefined
  ): void {
    const idGen = this.context.idGenerator;

    // Type pointer (overhead)
    const typePtrId = idGen.nodeId('tp');
    this.createAndAddNode(typePtrId, 'py-type-ptr', 'type*', {
      caption: 'Type pointer',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), nodeId, typePtrId, 'has-type-ptr');

    // Refcount (overhead)
    const refcountId = idGen.nodeId('rc');
    this.createAndAddNode(refcountId, 'py-refcount', 'refcount', {
      caption: 'Reference count',
      sizeBytes: 8,
      isOverhead: true,
      refCount: 1,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), nodeId, refcountId, 'has-refcount');

    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, nodeId, 'contains');
    }
  }

  private generateNone(path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(null);
    this.context.valueTracker.markSeen(null);

    // None singleton
    const noneId = idGen.nodeId('none');
    const noneNode = this.createAndAddNode(noneId, 'py-none', 'None', {
      caption: isDup ? `None @ ${path} (DUP!)` : `None @ ${path}`,
      sizeBytes: estimateSize('py-none'),
      isDuplicate: isDup,
      implementation: this.implementation,
    });
    nodes.push(noneNode);

    this.addPyObjectOverhead(noneId, parentId);

    return { rootNodeId: noneId, nodes, relationships: [] };
  }

  private generateBool(value: boolean, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // Bool (True/False singleton-like)
    const boolId = idGen.nodeId('bool');
    const boolNode = this.createAndAddNode(boolId, 'py-bool', String(value), {
      caption: isDup ? `${value ? 'True' : 'False'} @ ${path} (DUP!)` : `${value ? 'True' : 'False'} @ ${path}`,
      sizeBytes: estimateSize('py-bool'),
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    nodes.push(boolNode);

    this.addPyObjectOverhead(boolId, parentId);

    return { rootNodeId: boolId, nodes, relationships: [] };
  }

  private generateNumber(value: number, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    if (Number.isInteger(value)) {
      // PyLong (arbitrary precision integer)
      const longId = idGen.nodeId('long');
      const longNode = this.createAndAddNode(longId, 'py-long', String(value), {
        caption: isDup ? `int @ ${path} (DUP!)` : `int @ ${path}`,
        sizeBytes: estimateSize('py-long'),
        isDuplicate: isDup,
        value,
        implementation: this.implementation,
      });
      nodes.push(longNode);

      this.addPyObjectOverhead(longId, parentId);

      return { rootNodeId: longId, nodes, relationships: [] };
    } else {
      // PyFloat
      const floatId = idGen.nodeId('float');
      const floatNode = this.createAndAddNode(floatId, 'py-float', String(value), {
        caption: isDup ? `float @ ${path} (DUP!)` : `float @ ${path}`,
        sizeBytes: estimateSize('py-float'),
        isDuplicate: isDup,
        value,
        implementation: this.implementation,
      });
      nodes.push(floatNode);

      this.addPyObjectOverhead(floatId, parentId);

      return { rootNodeId: floatId, nodes, relationships: [] };
    }
  }

  private generateString(value: string, path: string, parentId?: string): GenerationResult {
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // PyUnicode
    const { nodeId: strId } = this.context.stringTracker.track(value);
    const strNode = this.createAndAddNode(strId, 'py-unicode', this.getValueLabel(value), {
      caption: isDup ? `str @ ${path} (DUP!)` : `str @ ${path}`,
      sizeBytes: estimateSize('py-unicode', value),
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });
    nodes.push(strNode);

    this.addPyObjectOverhead(strId, parentId);

    return { rootNodeId: strId, nodes, relationships: [] };
  }

  private generateList(value: unknown[], path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // PyList
    const listId = idGen.nodeId('list');
    const listNode = this.createAndAddNode(listId, 'py-list', `list[${value.length}]`, {
      caption: `list @ ${path}`,
      sizeBytes: estimateSize('py-list'),
      implementation: this.implementation,
    });
    nodes.push(listNode);

    this.addPyObjectOverhead(listId, parentId);

    // Generate children
    value.forEach((item, index) => {
      const childPath = `${path}[${index}]`;
      const result = this.generateValue(item, childPath, listId);
      nodes.push(...result.nodes);
    });

    return { rootNodeId: listId, nodes, relationships: [] };
  }

  private generateDict(
    value: Record<string, unknown>,
    path: string,
    parentId?: string
  ): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const keys = Object.keys(value);

    // PyDict
    const dictId = idGen.nodeId('dict');
    const dictNode = this.createAndAddNode(dictId, 'py-dict', `dict{${keys.length}}`, {
      caption: `dict @ ${path}`,
      sizeBytes: estimateSize('py-dict'),
      implementation: this.implementation,
    });
    nodes.push(dictNode);

    this.addPyObjectOverhead(dictId, parentId);

    // Generate key-value pairs
    for (const key of keys) {
      const childPath = `${path}["${key}"]`;

      // Check for duplicate key string
      const isKeyDup = this.context.valueTracker.isDuplicate(key);
      this.context.valueTracker.markSeen(key);

      // Key as PyUnicode
      const { nodeId: keyId } = this.context.stringTracker.track(key);
      const keyNode = this.createAndAddNode(keyId, 'py-unicode', `"${key}"`, {
        caption: isKeyDup ? 'dict key (DUP!)' : 'dict key',
        sizeBytes: estimateSize('py-unicode', key),
        isDuplicate: isKeyDup,
        value: key,
        implementation: this.implementation,
      });
      nodes.push(keyNode);

      // Key also needs PyObject overhead
      const keyTypePtrId = idGen.nodeId('tp');
      this.createAndAddNode(keyTypePtrId, 'py-type-ptr', 'type*', {
        caption: 'Type pointer (key)',
        sizeBytes: 8,
        isOverhead: true,
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), keyId, keyTypePtrId, 'has-type-ptr');

      this.createAndAddRelationship(idGen.edgeId(), dictId, keyId, 'has-key');

      // Generate value
      const result = this.generateValue(value[key], childPath, dictId);
      nodes.push(...result.nodes);
    }

    return { rootNodeId: dictId, nodes, relationships: [] };
  }
}
