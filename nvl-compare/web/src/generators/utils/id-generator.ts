import type { IdGenerator } from '../types';

/**
 * Simple sequential ID generator
 */
export class SequentialIdGenerator implements IdGenerator {
  private nodeCounter = 0;
  private edgeCounter = 0;
  private prefix: string;

  constructor(prefix = '') {
    this.prefix = prefix;
  }

  nodeId(typePrefix?: string): string {
    const id = `${this.prefix}${typePrefix ? typePrefix + '-' : 'n'}${++this.nodeCounter}`;
    return id;
  }

  edgeId(): string {
    return `${this.prefix}e${++this.edgeCounter}`;
  }

  reset(): void {
    this.nodeCounter = 0;
    this.edgeCounter = 0;
  }
}

/**
 * Create a new ID generator for a specific implementation
 */
export function createIdGenerator(implementation: string): IdGenerator {
  return new SequentialIdGenerator(`${implementation}-`);
}
