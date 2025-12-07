import type { GraphData, NvlNode, NvlRelationship, ImplementationId } from '../types/graph';
import type { GraphGenerator, GeneratorContext, GenerationResult, StringTracker, IdGenerator } from './types';
import { createNode, createRelationship } from './types';
import { ImmutableValueTracker } from './utils/string-tracker';

/**
 * Abstract base class for graph generators
 * Provides common functionality for traversing JSON and building graphs
 */
export abstract class BaseGenerator implements GraphGenerator {
  abstract readonly id: ImplementationId;
  abstract readonly name: string;
  protected abstract readonly implementation: 'hakka' | 'cpython' | 'serde' | 'go' | 'jansson';

  /** Get the language for this implementation */
  protected abstract getLanguage(): string;
  /** Get a short description for this implementation */
  protected abstract getDescription(): string;

  protected nodes: NvlNode[] = [];
  protected relationships: NvlRelationship[] = [];
  protected context!: GeneratorContext;

  /**
   * Create the string tracker for this implementation
   * Override in subclasses for different interning behavior
   */
  protected abstract createStringTracker(idGen: () => string): StringTracker;

  /**
   * Create the ID generator for this implementation
   */
  protected abstract createIdGenerator(): IdGenerator;

  /**
   * Generate implementation-specific root/infrastructure nodes
   * (e.g., Registry for HakkaJson, nothing for serde)
   */
  protected abstract generateInfrastructure(): void;

  /**
   * Generate nodes for a JSON value
   * Must be implemented by each generator
   */
  protected abstract generateValue(
    value: unknown,
    path: string,
    parentId?: string
  ): GenerationResult;

  /**
   * Main entry point - generates complete graph from JSON
   */
  generate(json: unknown): GraphData {
    // Reset state
    this.nodes = [];
    this.relationships = [];

    // Create context
    const idGen = this.createIdGenerator();
    this.context = {
      idGenerator: idGen,
      stringTracker: this.createStringTracker(() => idGen.nodeId('str')),
      valueTracker: new ImmutableValueTracker(),
      path: '$',
    };

    // Generate infrastructure nodes (if any)
    this.generateInfrastructure();

    // Generate the JSON graph
    this.generateValue(json, '$');

    // Calculate stats
    const stringStats = this.context.stringTracker.getStats();
    const totalBytes = this.nodes.reduce(
      (sum, n) => sum + (n.properties?.sizeBytes ?? 0),
      0
    );
    // Overhead = structural overhead (isOverhead) + wasted memory from duplicates (isDuplicate)
    const overheadBytes = this.nodes
      .filter((n) => n.properties?.isOverhead || n.properties?.isDuplicate)
      .reduce((sum, n) => sum + (n.properties?.sizeBytes ?? 0), 0);

    return {
      id: this.id,
      name: this.name,
      language: this.getLanguage(),
      description: this.getDescription(),
      nodes: this.nodes,
      relationships: this.relationships,
      stats: {
        nodeCount: this.nodes.length,
        edgeCount: this.relationships.length,
        totalBytes,
        overheadBytes,
        overheadPercent: totalBytes > 0 ? (overheadBytes / totalBytes) * 100 : 0,
        uniqueStrings: stringStats.uniqueStrings,
        duplicateStrings: stringStats.duplicateStrings,
        internedStrings: stringStats.uniqueStrings,
        bytesSaved: stringStats.bytesSaved,
      },
    };
  }

  /**
   * Helper to add a node
   */
  protected addNode(node: NvlNode): void {
    this.nodes.push(node);
  }

  /**
   * Helper to add a relationship
   */
  protected addRelationship(rel: NvlRelationship): void {
    this.relationships.push(rel);
  }

  /**
   * Helper to create and add a node in one call
   */
  protected createAndAddNode(
    ...args: Parameters<typeof createNode>
  ): NvlNode {
    const node = createNode(...args);
    this.addNode(node);
    return node;
  }

  /**
   * Helper to create and add a relationship in one call
   */
  protected createAndAddRelationship(
    ...args: Parameters<typeof createRelationship>
  ): NvlRelationship {
    const rel = createRelationship(...args);
    this.addRelationship(rel);
    return rel;
  }

  /**
   * Determine the JSON type of a value
   */
  protected getJsonType(value: unknown): 'null' | 'boolean' | 'number' | 'string' | 'array' | 'object' {
    if (value === null) return 'null';
    if (typeof value === 'boolean') return 'boolean';
    if (typeof value === 'number') return 'number';
    if (typeof value === 'string') return 'string';
    if (Array.isArray(value)) return 'array';
    return 'object';
  }

  /**
   * Generate a short label for a value
   */
  protected getValueLabel(value: unknown, maxLength = 20): string {
    if (value === null) return 'null';
    if (typeof value === 'boolean') return String(value);
    if (typeof value === 'number') return String(value);
    if (typeof value === 'string') {
      if (value.length <= maxLength) return `"${value}"`;
      return `"${value.slice(0, maxLength - 3)}..."`;
    }
    if (Array.isArray(value)) return `Array[${value.length}]`;
    if (typeof value === 'object') return `Object{${Object.keys(value).length}}`;
    return String(value);
  }
}
