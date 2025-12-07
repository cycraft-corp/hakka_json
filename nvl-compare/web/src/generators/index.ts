import type { ImplementationId } from '../types/graph';
import type { GraphGenerator } from './types';
import { HakkaJsonGenerator } from './hakka';
import { SerdeJsonGenerator } from './serde';
import { CPythonJsonGenerator } from './cpython';
import { GoJsonGenerator } from './go';
import { JanssonGenerator } from './jansson';

// Export types
export type { GraphGenerator, GeneratorContext, GenerationResult, StringTracker, IdGenerator } from './types';
export { estimateSize, createNode, createRelationship } from './types';

// Export generators
export { HakkaJsonGenerator } from './hakka';
export { SerdeJsonGenerator } from './serde';
export { CPythonJsonGenerator } from './cpython';
export { GoJsonGenerator } from './go';
export { JanssonGenerator } from './jansson';

// Export utilities
export * from './utils';

/**
 * Map of implementation ID to generator instance
 */
const generators: Record<ImplementationId, GraphGenerator> = {
  hakka_json: new HakkaJsonGenerator(),
  serde_json: new SerdeJsonGenerator(),
  cpython_json: new CPythonJsonGenerator(),
  go_json: new GoJsonGenerator(),
  jansson: new JanssonGenerator(),
};

/**
 * Get generator for a specific implementation
 */
export function getGenerator(id: ImplementationId): GraphGenerator {
  return generators[id];
}

/**
 * Get all available generators
 */
export function getAllGenerators(): GraphGenerator[] {
  return Object.values(generators);
}

/**
 * Generate graph data for a JSON value using a specific implementation
 */
export function generateGraph(json: unknown, implementation: ImplementationId) {
  const generator = getGenerator(implementation);
  return generator.generate(json);
}
