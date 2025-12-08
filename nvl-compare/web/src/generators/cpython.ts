import type { ImplementationId, NvlNode } from '../types/graph';
import type { GenerationResult, StringTracker, IdGenerator } from './types';
import { estimateSize } from './types';
import { BaseGenerator } from './base';
import { SequentialIdGenerator } from './utils/id-generator';
import { DuplicatingStringTracker } from './utils/string-tracker';

/**
 * CPython json module Graph Generator
 *
 * Reference: CPython 3.11/3.12 source code
 * - Include/cpython/object.h (PyTypeObject, protocol structs)
 * - Include/object.h (PyObject, PyVarObject)
 * - Objects/longobject.c, Objects/floatobject.c, etc.
 *
 * Memory Overhead Categories:
 * 1. STRUCTURAL OVERHEAD (marked isOverhead: true):
 *    - Per-object: ob_refcnt (8B) + ob_type (8B) = 16 bytes per object
 *    - Infrastructure: PyTypeObject + protocol method structs
 *
 * 2. WASTED MEMORY (marked isDuplicate: true):
 *    - Duplicate immutable values (strings, numbers, booleans, null)
 *    - CPython's json module does NOT intern strings by default
 *
 * Key characteristics:
 * - PyObject header on EVERY value (ob_refcnt: 8B + ob_type: 8B = 16 bytes)
 * - ob_type pointer connects to shared PyTypeObject infrastructure
 * - PyTypeObject includes protocol method structs (PyNumberMethods, etc.)
 *
 * Infrastructure breakdown (from CPython 3.11 source):
 * - PyTypeObject base: 424 bytes (53 fields × 8 bytes on 64-bit)
 * - PyNumberMethods: 280 bytes (35 function pointers × 8)
 * - PySequenceMethods: 80 bytes (10 function pointers × 8)
 * - PyMappingMethods: 24 bytes (3 function pointers × 8)
 *
 * Type Object Totals:
 * - PyLong_Type:    424 + 280 = 704 bytes
 * - PyFloat_Type:   424 + 280 = 704 bytes
 * - PyUnicode_Type: 424 + 280 + 80 + 24 = 808 bytes
 * - PyList_Type:    424 + 80 + 24 = 528 bytes
 * - PyDict_Type:    424 + 80 + 24 = 528 bytes
 * - PyBool_Type:    424 + 280 = 704 bytes
 * - PyNone_Type:    424 = 424 bytes
 * - TOTAL INFRASTRUCTURE: 4,400 bytes
 */
export class CPythonJsonGenerator extends BaseGenerator {
  readonly id: ImplementationId = 'cpython_json';
  readonly name = 'CPython json';
  protected readonly implementation = 'cpython' as const;

  // Store type object IDs for linking ob_type pointers
  private typeObjectIds: Record<string, string> = {};

  protected getLanguage(): string {
    return 'Python';
  }

  protected getDescription(): string {
    return 'PyObject overhead, refcounting, type objects';
  }

  protected createStringTracker(idGen: () => string): StringTracker {
    // CPython may intern some strings but we simulate without
    return new DuplicatingStringTracker(idGen);
  }

  protected createIdGenerator(): IdGenerator {
    return new SequentialIdGenerator('py-');
  }

  /**
   * Generate CPython type object infrastructure (CPython 3.11/3.12)
   *
   * Each type object consists of:
   * 1. PyTypeObject base struct (424 bytes = 53 fields × 8 bytes)
   * 2. Protocol method structs pointed to by tp_as_* fields
   *
   * The ob_type pointer in every PyObject points to one of these.
   *
   * Note: Unlike HakkaJson's ManagerRegistry (32 bytes), CPython type objects
   * are global static variables compiled into the Python binary. There's no
   * single "registry" allocation - we use TypePool as a conceptual grouping
   * node with 0 bytes, since the actual overhead is in the type objects themselves.
   */
  protected generateInfrastructure(): void {
    const idGen = this.context.idGenerator;

    // Type Objects Pool - conceptual grouping node (0 bytes)
    // Unlike HakkaJson's Registry, CPython type objects are global statics,
    // not dynamically allocated from a central registry.
    // The actual overhead is in the individual PyTypeObject structs below.
    const typePoolId = idGen.nodeId('types');
    this.createAndAddNode(typePoolId, 'py-type-pool', 'TypePool', {
      caption: 'Python Type Objects (grouping, 0B)',
      sizeBytes: 0,  // Conceptual grouping only - real overhead in type objects
      isOverhead: true,
      implementation: this.implementation,
    });

    // Define JSON-relevant types with their protocol method requirements
    // Sizes from CPython source: Include/cpython/object.h
    const jsonTypes: Array<{
      pyName: string;          // Python type name
      cName: string;           // C type object name
      protocols: Array<'number' | 'sequence' | 'mapping'>;
    }> = [
      { pyName: 'int', cName: 'PyLong_Type', protocols: ['number'] },
      { pyName: 'float', cName: 'PyFloat_Type', protocols: ['number'] },
      { pyName: 'str', cName: 'PyUnicode_Type', protocols: ['number', 'sequence', 'mapping'] },
      { pyName: 'list', cName: 'PyList_Type', protocols: ['sequence', 'mapping'] },
      { pyName: 'dict', cName: 'PyDict_Type', protocols: ['sequence', 'mapping'] },
      { pyName: 'bool', cName: 'PyBool_Type', protocols: ['number'] },
      { pyName: 'NoneType', cName: 'PyNone_Type', protocols: [] },
    ];

    for (const { pyName, cName, protocols } of jsonTypes) {
      // Create base PyTypeObject node (424 bytes)
      const typeId = idGen.nodeId(`type-${pyName}`);
      this.typeObjectIds[pyName] = typeId;

      // Calculate total size including protocols
      let totalSize = estimateSize('py-type-object'); // 424 bytes base
      const protocolSizes: string[] = [];

      this.createAndAddNode(typeId, 'py-type-object', pyName, {
        caption: `${cName} (424B base)`,
        sizeBytes: estimateSize('py-type-object'),
        isOverhead: true,
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), typePoolId, typeId, 'contains');

      // Create protocol method struct nodes
      if (protocols.includes('number')) {
        const numberId = idGen.nodeId(`${pyName}-nb`);
        this.createAndAddNode(numberId, 'py-number-methods', 'tp_as_number', {
          caption: `PyNumberMethods (280B = 35 ptrs)`,
          sizeBytes: estimateSize('py-number-methods'),
          isOverhead: true,
          implementation: this.implementation,
        });
        this.createAndAddRelationship(idGen.edgeId(), typeId, numberId, 'contains');
        totalSize += estimateSize('py-number-methods');
        protocolSizes.push('nb:280');
      }

      if (protocols.includes('sequence')) {
        const seqId = idGen.nodeId(`${pyName}-sq`);
        this.createAndAddNode(seqId, 'py-sequence-methods', 'tp_as_sequence', {
          caption: `PySequenceMethods (80B = 10 ptrs)`,
          sizeBytes: estimateSize('py-sequence-methods'),
          isOverhead: true,
          implementation: this.implementation,
        });
        this.createAndAddRelationship(idGen.edgeId(), typeId, seqId, 'contains');
        totalSize += estimateSize('py-sequence-methods');
        protocolSizes.push('sq:80');
      }

      if (protocols.includes('mapping')) {
        const mapId = idGen.nodeId(`${pyName}-mp`);
        this.createAndAddNode(mapId, 'py-mapping-methods', 'tp_as_mapping', {
          caption: `PyMappingMethods (24B = 3 ptrs)`,
          sizeBytes: estimateSize('py-mapping-methods'),
          isOverhead: true,
          implementation: this.implementation,
        });
        this.createAndAddRelationship(idGen.edgeId(), typeId, mapId, 'contains');
        totalSize += estimateSize('py-mapping-methods');
        protocolSizes.push('mp:24');
      }
    }
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

  /**
   * Add PyObject header overhead (ob_refcnt + ob_type)
   *
   * Every CPython object has this 16-byte header:
   * - ob_refcnt: Py_ssize_t (8 bytes) - reference count for GC
   * - ob_type: PyTypeObject* (8 bytes) - pointer to type object
   *
   * The ob_type pointer CONNECTS to the shared type object infrastructure!
   */
  private addPyObjectOverhead(
    nodeId: string,
    parentId: string | undefined,
    typeName: string  // 'int', 'float', 'str', 'list', 'dict', 'bool', 'NoneType'
  ): void {
    const idGen = this.context.idGenerator;

    // ob_type pointer (8 bytes overhead)
    const typePtrId = idGen.nodeId('tp');
    this.createAndAddNode(typePtrId, 'py-type-ptr', 'ob_type', {
      caption: `ob_type → ${typeName}`,
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), nodeId, typePtrId, 'has-type-ptr');

    // Link ob_type to the actual PyTypeObject in infrastructure
    const typeObjectId = this.typeObjectIds[typeName];
    if (typeObjectId) {
      this.createAndAddRelationship(idGen.edgeId(), typePtrId, typeObjectId, 'points-to');
    }

    // ob_refcnt (8 bytes overhead)
    const refcountId = idGen.nodeId('rc');
    this.createAndAddNode(refcountId, 'py-refcount', 'ob_refcnt', {
      caption: 'Reference count (8B)',
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

    this.addPyObjectOverhead(noneId, parentId, 'NoneType');

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

    this.addPyObjectOverhead(boolId, parentId, 'bool');

    return { rootNodeId: boolId, nodes, relationships: [] };
  }

  /**
   * Generate PyLongObject or PyFloatObject with FULL field-level breakdown
   *
   * PyLongObject layout (CPython 3.11, arbitrary precision integer):
   * - ob_refcnt [8B] - reference count (overhead)
   * - ob_type [8B] - pointer to PyLong_Type (overhead)
   * - ob_size [8B] - digit count + sign (negative = negative number)
   * - ob_digit[] [4B × n] - 30-bit digits in little-endian order
   *
   * For small integers (-5 to 256): Python uses singleton cache
   * Total: 24 bytes header + 4 bytes per digit (minimum 1 digit)
   *
   * PyFloatObject layout (CPython 3.11):
   * - ob_refcnt [8B] - reference count (overhead)
   * - ob_type [8B] - pointer to PyFloat_Type (overhead)
   * - ob_fval [8B] - IEEE 754 double-precision float
   *
   * Total: 24 bytes (16B header + 8B data)
   */
  private generateNumber(value: number, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    if (Number.isInteger(value)) {
      // ========== PyLongObject (arbitrary precision integer) ==========
      const longId = idGen.nodeId('long');
      this.createAndAddNode(longId, 'py-long', String(value), {
        caption: isDup ? `PyLongObject @ ${path} (DUP!)` : `PyLongObject @ ${path}`,
        sizeBytes: 0,  // Size is in child fields
        isDuplicate: isDup,
        value,
        implementation: this.implementation,
      });

      // Connect to parent
      if (parentId) {
        this.createAndAddRelationship(idGen.edgeId(), parentId, longId, 'contains');
      }

      // ob_refcnt [8B] - overhead
      const refcntId = idGen.nodeId('rc');
      this.createAndAddNode(refcntId, 'py-refcount', 'ob_refcnt', {
        caption: 'ob_refcnt (8B)',
        sizeBytes: 8,
        isOverhead: true,
        refCount: 1,
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), longId, refcntId, 'has-refcount');

      // ob_type [8B] - overhead, points to PyLong_Type
      const typeId = idGen.nodeId('tp');
      this.createAndAddNode(typeId, 'py-type-ptr', 'ob_type', {
        caption: 'ob_type → int (8B)',
        sizeBytes: 8,
        isOverhead: true,
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), longId, typeId, 'has-type-ptr');
      const intTypeId = this.typeObjectIds['int'];
      if (intTypeId) {
        this.createAndAddRelationship(idGen.edgeId(), typeId, intTypeId, 'points-to');
      }

      // ob_size [8B] - digit count + sign
      // Calculate number of 30-bit digits needed
      const absValue = Math.abs(value);
      const digitCount = absValue === 0 ? 0 : Math.max(1, Math.ceil(Math.log2(absValue + 1) / 30));
      const signedDigitCount = value < 0 ? -digitCount : digitCount;

      // ob_size [8B] - OVERHEAD (metadata about digit count/sign)
      const obSizeId = idGen.nodeId('ob-size');
      this.createAndAddNode(obSizeId, 'py-long-ob-size', 'ob_size', {
        caption: `ob_size = ${signedDigitCount} (8B, metadata overhead)`,
        sizeBytes: 8,
        isOverhead: true,  // Metadata about data, not the data itself
        value: String(signedDigitCount),
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), longId, obSizeId, 'struct-field');

      // ob_digit[] [4B × n] - array of 30-bit digits
      const actualDigits = Math.max(1, digitCount);  // At least 1 digit for storage
      const digitId = idGen.nodeId('digit');
      this.createAndAddNode(digitId, 'py-long-digit', 'ob_digit[]', {
        caption: `ob_digit[${actualDigits}] = ${value} (${actualDigits * 4}B)`,
        sizeBytes: actualDigits * 4,
        value: String(value),
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), longId, digitId, 'struct-field');

      return { rootNodeId: longId, nodes, relationships: [] };
    } else {
      // ========== PyFloatObject ==========
      const floatId = idGen.nodeId('float');
      this.createAndAddNode(floatId, 'py-float', String(value), {
        caption: isDup ? `PyFloatObject @ ${path} (DUP!)` : `PyFloatObject @ ${path}`,
        sizeBytes: 0,  // Size is in child fields
        isDuplicate: isDup,
        value,
        implementation: this.implementation,
      });

      // Connect to parent
      if (parentId) {
        this.createAndAddRelationship(idGen.edgeId(), parentId, floatId, 'contains');
      }

      // ob_refcnt [8B] - overhead
      const refcntId = idGen.nodeId('rc');
      this.createAndAddNode(refcntId, 'py-refcount', 'ob_refcnt', {
        caption: 'ob_refcnt (8B)',
        sizeBytes: 8,
        isOverhead: true,
        refCount: 1,
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), floatId, refcntId, 'has-refcount');

      // ob_type [8B] - overhead, points to PyFloat_Type
      const typeId = idGen.nodeId('tp');
      this.createAndAddNode(typeId, 'py-type-ptr', 'ob_type', {
        caption: 'ob_type → float (8B)',
        sizeBytes: 8,
        isOverhead: true,
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), floatId, typeId, 'has-type-ptr');
      const floatTypeId = this.typeObjectIds['float'];
      if (floatTypeId) {
        this.createAndAddRelationship(idGen.edgeId(), typeId, floatTypeId, 'points-to');
      }

      // ob_fval [8B] - IEEE 754 double
      const fvalId = idGen.nodeId('fval');
      this.createAndAddNode(fvalId, 'py-float-fval', 'ob_fval', {
        caption: `ob_fval = ${value} (8B)`,
        sizeBytes: 8,
        value: String(value),
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), floatId, fvalId, 'struct-field');

      return { rootNodeId: floatId, nodes, relationships: [] };
    }
  }

  /**
   * Generate PyUnicodeObject (PyASCIIObject for ASCII strings) with FULL field-level breakdown
   *
   * PyASCIIObject layout (CPython 3.11, compact ASCII string):
   * - ob_refcnt [8B] - reference count (overhead)
   * - ob_type [8B] - pointer to PyUnicode_Type (overhead)
   * - length [8B] - string length
   * - hash [8B] - cached hash (-1 if not computed)
   * - state [4B] - bitfield (interned:2, kind:3, compact:1, ascii:1, ready:1)
   * - wstr [8B] - deprecated legacy pointer (NULL for compact, overhead)
   * - data [len+1] - inline character data + null terminator
   *
   * Total: 48 bytes header + (length + 1) bytes data
   */
  private generateString(value: string, path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const len = value.length;

    // Check for duplicate immutable value
    const isDup = this.context.valueTracker.isDuplicate(value);
    this.context.valueTracker.markSeen(value);

    // ========== PyASCIIObject (container) ==========
    const { nodeId: strId } = this.context.stringTracker.track(value);
    this.createAndAddNode(strId, 'py-unicode', this.getValueLabel(value), {
      caption: isDup ? `PyASCIIObject @ ${path} (DUP!)` : `PyASCIIObject @ ${path}`,
      sizeBytes: 0,  // Size is in child fields
      isDuplicate: isDup,
      value,
      implementation: this.implementation,
    });

    // Connect to parent
    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, strId, 'contains');
    }

    // ob_refcnt [8B] - overhead
    const refcntId = idGen.nodeId('rc');
    this.createAndAddNode(refcntId, 'py-refcount', 'ob_refcnt', {
      caption: 'ob_refcnt (8B)',
      sizeBytes: 8,
      isOverhead: true,
      refCount: 1,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), strId, refcntId, 'has-refcount');

    // ob_type [8B] - overhead, points to PyUnicode_Type
    const typeId = idGen.nodeId('tp');
    this.createAndAddNode(typeId, 'py-type-ptr', 'ob_type', {
      caption: 'ob_type → str (8B)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), strId, typeId, 'has-type-ptr');
    const strTypeId = this.typeObjectIds['str'];
    if (strTypeId) {
      this.createAndAddRelationship(idGen.edgeId(), typeId, strTypeId, 'points-to');
    }

    // length [8B] - string length - OVERHEAD (metadata, not the data itself)
    const lengthId = idGen.nodeId('len');
    this.createAndAddNode(lengthId, 'py-unicode-length', 'length', {
      caption: `length = ${len} (8B, metadata overhead)`,
      sizeBytes: 8,
      isOverhead: true,  // Metadata about data, not the data itself
      value: String(len),
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), strId, lengthId, 'struct-field');

    // hash [8B] - cached hash (-1 if not computed) - OVERHEAD (caching optimization)
    const hashId = idGen.nodeId('hash');
    this.createAndAddNode(hashId, 'py-unicode-hash', 'hash', {
      caption: 'hash (8B, caching overhead)',
      sizeBytes: 8,
      isOverhead: true,  // Caching optimization, not part of JSON data
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), strId, hashId, 'struct-field');

    // state [4B] - bitfield flags - OVERHEAD (type system metadata)
    const stateId = idGen.nodeId('state');
    this.createAndAddNode(stateId, 'py-unicode-state', 'state', {
      caption: 'state flags (4B, type overhead)',
      sizeBytes: 4,
      isOverhead: true,  // Type system flags beyond JSON needs
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), strId, stateId, 'struct-field');

    // wstr [8B] - deprecated legacy pointer (NULL for compact ASCII)
    const wstrId = idGen.nodeId('wstr');
    this.createAndAddNode(wstrId, 'py-unicode-wstr', 'wstr', {
      caption: 'wstr = NULL (8B, deprecated)',
      sizeBytes: 8,
      isOverhead: true,  // Deprecated field, pure overhead
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), strId, wstrId, 'struct-field');

    // data [len+1] - inline character data + null terminator
    const dataId = idGen.nodeId('data');
    const displayValue = len <= 20 ? `"${value}"` : `"${value.slice(0, 17)}..."`;
    this.createAndAddNode(dataId, 'py-unicode-data', 'data[]', {
      caption: `data[${len + 1}] = ${displayValue}`,
      sizeBytes: len + 1,  // Include null terminator
      value,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), strId, dataId, 'struct-field');

    return { rootNodeId: strId, nodes, relationships: [] };
  }

  /**
   * Generate PyListObject with FULL field-level breakdown
   *
   * PyListObject layout (CPython 3.11):
   * - ob_refcnt [8B] - reference count (overhead)
   * - ob_type [8B] - pointer to PyList_Type (overhead)
   * - ob_size [8B] - number of items (part of PyVarObject header)
   * - ob_item [8B ptr] → heap-allocated array of PyObject* pointers
   * - allocated [8B] - capacity of the array (>= ob_size)
   *
   * The ob_item pointer points to a separate heap allocation:
   * - PyObject* slots [8B × allocated]
   *
   * Total: 40 bytes base + 8 bytes per slot in items array
   */
  private generateList(value: unknown[], path: string, parentId?: string): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const count = value.length;

    // ========== PyListObject (container) ==========
    const listId = idGen.nodeId('list');
    this.createAndAddNode(listId, 'py-list', `list[${count}]`, {
      caption: `PyListObject @ ${path}`,
      sizeBytes: 0,  // Size is in child fields
      implementation: this.implementation,
    });

    // Connect to parent
    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, listId, 'contains');
    }

    // ob_refcnt [8B] - overhead
    const refcntId = idGen.nodeId('rc');
    this.createAndAddNode(refcntId, 'py-refcount', 'ob_refcnt', {
      caption: 'ob_refcnt (8B)',
      sizeBytes: 8,
      isOverhead: true,
      refCount: 1,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), listId, refcntId, 'has-refcount');

    // ob_type [8B] - overhead, points to PyList_Type
    const typeId = idGen.nodeId('tp');
    this.createAndAddNode(typeId, 'py-type-ptr', 'ob_type', {
      caption: 'ob_type → list (8B)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), listId, typeId, 'has-type-ptr');
    const listTypeId = this.typeObjectIds['list'];
    if (listTypeId) {
      this.createAndAddRelationship(idGen.edgeId(), typeId, listTypeId, 'points-to');
    }

    // ob_size [8B] - number of items - OVERHEAD (metadata)
    const obSizeId = idGen.nodeId('ob-size');
    this.createAndAddNode(obSizeId, 'py-list-ob-size', 'ob_size', {
      caption: `ob_size = ${count} (8B, metadata overhead)`,
      sizeBytes: 8,
      isOverhead: true,  // Metadata about data, not the data itself
      value: String(count),
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), listId, obSizeId, 'struct-field');

    // allocated [8B] - capacity - OVERHEAD (over-allocation metadata)
    const allocatedId = idGen.nodeId('alloc');
    this.createAndAddNode(allocatedId, 'py-list-allocated', 'allocated', {
      caption: `allocated = ${count} (8B, capacity overhead)`,
      sizeBytes: 8,
      isOverhead: true,  // Over-allocation tracking, not data
      value: String(count),
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), listId, allocatedId, 'struct-field');

    // ob_item [8B ptr] → items array - OVERHEAD (pointer to heap)
    const obItemPtrId = idGen.nodeId('ob-item-ptr');
    this.createAndAddNode(obItemPtrId, 'py-list-ob-item-ptr', 'ob_item', {
      caption: 'ob_item ptr (8B, pointer overhead)',
      sizeBytes: 8,
      isOverhead: true,  // Pointer is representation overhead
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), listId, obItemPtrId, 'struct-field');

    // Items array (heap-allocated PyObject* array)
    if (count > 0) {
      const itemsArrayId = idGen.nodeId('items-arr');
      this.createAndAddNode(itemsArrayId, 'py-list-items-array', 'items[]', {
        caption: `PyObject*[${count}] (${count * 8}B heap)`,
        sizeBytes: 0,  // Size is in individual slots
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), obItemPtrId, itemsArrayId, 'points-to');

      // Generate each item slot and child value
      value.forEach((item, index) => {
        const childPath = `${path}[${index}]`;

        // Individual slot [8B] - pointer to child object - OVERHEAD
        const slotId = idGen.nodeId('slot');
        this.createAndAddNode(slotId, 'py-list-item-slot', `[${index}]`, {
          caption: `slot[${index}] ptr (8B, pointer overhead)`,
          sizeBytes: 8,
          isOverhead: true,  // Pointer is representation overhead
          implementation: this.implementation,
        });
        this.createAndAddRelationship(idGen.edgeId(), itemsArrayId, slotId, 'array-element');

        // Generate child value and connect slot to it
        const result = this.generateValue(item, childPath);
        this.createAndAddRelationship(idGen.edgeId(), slotId, result.rootNodeId, 'points-to');
        nodes.push(...result.nodes);
      });
    }

    return { rootNodeId: listId, nodes, relationships: [] };
  }

  /**
   * Generate PyDictObject with FULL field-level breakdown
   *
   * PyDictObject layout (CPython 3.11):
   * - ob_refcnt [8B] - reference count (overhead)
   * - ob_type [8B] - pointer to PyDict_Type (overhead)
   * - ma_used [8B] - number of items
   * - ma_version_tag [8B] - version for dict views
   * - ma_keys [8B ptr] → PyDictKeysObject
   * - ma_values [8B ptr] - NULL for combined dict
   *
   * PyDictKeysObject layout:
   * - dk_refcnt [8B] - reference count (shared keys)
   * - dk_log2_size [1B] + dk_log2_index_bytes [1B] + dk_kind [1B]
   * - dk_version [4B]
   * - dk_usable [8B]
   * - dk_nentries [8B]
   * - dk_indices[] - hash table
   * - dk_entries[] - array of (me_hash, me_key, me_value)
   */
  private generateDict(
    value: Record<string, unknown>,
    path: string,
    parentId?: string
  ): GenerationResult {
    const idGen = this.context.idGenerator;
    const nodes: NvlNode[] = [];
    const keys = Object.keys(value);
    const numEntries = keys.length;

    // ========== PyDictObject (container) ==========
    const dictId = idGen.nodeId('dict');
    this.createAndAddNode(dictId, 'py-dict', `dict{${numEntries}}`, {
      caption: `PyDictObject @ ${path}`,
      sizeBytes: 0,  // Size is in child fields
      implementation: this.implementation,
    });

    // Connect to parent
    if (parentId) {
      this.createAndAddRelationship(idGen.edgeId(), parentId, dictId, 'contains');
    }

    // ob_refcnt [8B] - overhead
    const refcntId = idGen.nodeId('rc');
    this.createAndAddNode(refcntId, 'py-refcount', 'ob_refcnt', {
      caption: 'ob_refcnt (8B)',
      sizeBytes: 8,
      isOverhead: true,
      refCount: 1,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), dictId, refcntId, 'has-refcount');

    // ob_type [8B] - overhead, points to PyDict_Type
    const typeId = idGen.nodeId('tp');
    this.createAndAddNode(typeId, 'py-type-ptr', 'ob_type', {
      caption: 'ob_type → dict (8B)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), dictId, typeId, 'has-type-ptr');
    const dictTypeId = this.typeObjectIds['dict'];
    if (dictTypeId) {
      this.createAndAddRelationship(idGen.edgeId(), typeId, dictTypeId, 'points-to');
    }

    // ma_used [8B] - number of items - OVERHEAD (metadata)
    const maUsedId = idGen.nodeId('ma-used');
    this.createAndAddNode(maUsedId, 'py-dict-ma-used', 'ma_used', {
      caption: `ma_used = ${numEntries} (8B, metadata overhead)`,
      sizeBytes: 8,
      isOverhead: true,  // Metadata about data, not the data itself
      value: String(numEntries),
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), dictId, maUsedId, 'struct-field');

    // ma_version_tag [8B] - version for iteration - OVERHEAD (invalidation tracking)
    const maVersionId = idGen.nodeId('ma-ver');
    this.createAndAddNode(maVersionId, 'py-dict-ma-version', 'ma_version_tag', {
      caption: 'ma_version_tag (8B, iteration overhead)',
      sizeBytes: 8,
      isOverhead: true,  // Iteration invalidation tracking, not JSON data
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), dictId, maVersionId, 'struct-field');

    // ma_values [8B] - NULL for combined dict - OVERHEAD (pointer)
    const maValuesId = idGen.nodeId('ma-vals');
    this.createAndAddNode(maValuesId, 'py-dict-ma-values-ptr', 'ma_values', {
      caption: 'ma_values = NULL (8B, pointer overhead)',
      sizeBytes: 8,
      isOverhead: true,  // Pointer is representation overhead
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), dictId, maValuesId, 'struct-field');

    // ========== PyDictKeysObject ==========
    // ma_keys [8B] - pointer to keys object - OVERHEAD (pointer)
    const maKeysId = idGen.nodeId('ma-keys');
    this.createAndAddNode(maKeysId, 'py-dict-ma-keys-ptr', 'ma_keys', {
      caption: 'ma_keys ptr (8B, pointer overhead)',
      sizeBytes: 8,
      isOverhead: true,  // Pointer is representation overhead
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), dictId, maKeysId, 'struct-field');

    // PyDictKeysObject structure
    const keysObjId = idGen.nodeId('keys-obj');
    this.createAndAddNode(keysObjId, 'py-dict-keys', 'PyDictKeysObject', {
      caption: 'PyDictKeysObject',
      sizeBytes: 0,  // Size in child fields
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), maKeysId, keysObjId, 'points-to');

    // dk_refcnt [8B] - shared keys refcount (overhead)
    const dkRefcntId = idGen.nodeId('dk-rc');
    this.createAndAddNode(dkRefcntId, 'py-dict-keys-refcnt', 'dk_refcnt', {
      caption: 'dk_refcnt (8B)',
      sizeBytes: 8,
      isOverhead: true,
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), keysObjId, dkRefcntId, 'struct-field');

    // dk_log2_size + dk_log2_index_bytes + dk_kind [3B] - OVERHEAD (hash table metadata)
    const dkLog2Id = idGen.nodeId('dk-log2');
    this.createAndAddNode(dkLog2Id, 'py-dict-keys-log2', 'dk_log2_*', {
      caption: 'dk_log2_size + index_bytes + kind (3B, hashtable overhead)',
      sizeBytes: 3,
      isOverhead: true,  // Hash table metadata, not data
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), keysObjId, dkLog2Id, 'struct-field');

    // dk_version [4B] - OVERHEAD (iteration invalidation)
    const dkVersionId = idGen.nodeId('dk-ver');
    this.createAndAddNode(dkVersionId, 'py-dict-keys-version', 'dk_version', {
      caption: 'dk_version (4B, iteration overhead)',
      sizeBytes: 4,
      isOverhead: true,  // Iteration invalidation tracking, not JSON data
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), keysObjId, dkVersionId, 'struct-field');

    // dk_usable [8B] - OVERHEAD (capacity metadata)
    const dkUsableId = idGen.nodeId('dk-usable');
    this.createAndAddNode(dkUsableId, 'py-dict-keys-usable', 'dk_usable', {
      caption: 'dk_usable (8B, capacity overhead)',
      sizeBytes: 8,
      isOverhead: true,  // Capacity tracking, not data
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), keysObjId, dkUsableId, 'struct-field');

    // dk_nentries [8B] - OVERHEAD (metadata)
    const dkNentriesId = idGen.nodeId('dk-nent');
    this.createAndAddNode(dkNentriesId, 'py-dict-keys-nentries', 'dk_nentries', {
      caption: `dk_nentries = ${numEntries} (8B, metadata overhead)`,
      sizeBytes: 8,
      isOverhead: true,  // Metadata about data, not the data itself
      value: String(numEntries),
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), keysObjId, dkNentriesId, 'struct-field');

    // dk_indices[] - hash table (minimum 8 slots × 1 byte) - OVERHEAD (hash table)
    const minHashSize = 8;  // CPython minimum
    const dkIndicesId = idGen.nodeId('dk-idx');
    this.createAndAddNode(dkIndicesId, 'py-dict-keys-indices', 'dk_indices[]', {
      caption: `dk_indices[${minHashSize}] (${minHashSize}B, hashtable overhead)`,
      sizeBytes: minHashSize,
      isOverhead: true,  // Hash table indices, not data
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), keysObjId, dkIndicesId, 'struct-field');

    // dk_entries[] container
    const dkEntriesId = idGen.nodeId('dk-ent');
    this.createAndAddNode(dkEntriesId, 'py-dict-keys-entries', 'dk_entries[]', {
      caption: `dk_entries[${numEntries}]`,
      sizeBytes: 0,  // Size in children
      implementation: this.implementation,
    });
    this.createAndAddRelationship(idGen.edgeId(), keysObjId, dkEntriesId, 'struct-field');

    // ========== Generate each entry ==========
    let entryIndex = 0;
    for (const key of keys) {
      const childPath = `${path}["${key}"]`;

      // Entry container
      const entryId = idGen.nodeId(`entry-${entryIndex}`);
      this.createAndAddNode(entryId, 'py-dict-entry', `entry[${entryIndex}]`, {
        caption: `PyDictKeyEntry[${entryIndex}]`,
        sizeBytes: 0,  // Size in children
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), dkEntriesId, entryId, 'contains');

      // me_hash [8B] - cached hash - OVERHEAD (caching)
      const hashId = idGen.nodeId(`hash-${entryIndex}`);
      this.createAndAddNode(hashId, 'py-dict-entry-hash', 'me_hash', {
        caption: 'me_hash (8B, caching overhead)',
        sizeBytes: 8,
        isOverhead: true,  // Cached hash, not data
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), entryId, hashId, 'struct-field');

      // me_key [8B ptr] - pointer to key object (overhead)
      const keyPtrId = idGen.nodeId(`keyptr-${entryIndex}`);
      this.createAndAddNode(keyPtrId, 'py-dict-entry-key-ptr', 'me_key', {
        caption: 'me_key ptr (8B)',
        sizeBytes: 8,
        isOverhead: true,  // Pointer overhead
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), entryId, keyPtrId, 'struct-field');

      // me_value [8B ptr] - pointer to value object (overhead)
      const valPtrId = idGen.nodeId(`valptr-${entryIndex}`);
      this.createAndAddNode(valPtrId, 'py-dict-entry-value-ptr', 'me_value', {
        caption: 'me_value ptr (8B)',
        sizeBytes: 8,
        isOverhead: true,  // Pointer overhead
        implementation: this.implementation,
      });
      this.createAndAddRelationship(idGen.edgeId(), entryId, valPtrId, 'struct-field');

      // Generate KEY (PyUnicodeObject)
      // Note: duplicate tracking is handled inside generateString
      const keyResult = this.generateString(key, `${path}.key("${key}")`);
      // Link me_key pointer to the key object
      this.createAndAddRelationship(idGen.edgeId(), keyPtrId, keyResult.rootNodeId, 'points-to');

      // Generate VALUE
      const valueResult = this.generateValue(value[key], childPath);
      // Link me_value pointer to the value object
      this.createAndAddRelationship(idGen.edgeId(), valPtrId, valueResult.rootNodeId, 'points-to');

      entryIndex++;
    }

    return { rootNodeId: dictId, nodes, relationships: [] };
  }
}
