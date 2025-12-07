import { useState, useCallback, useMemo } from 'react';
import type { GraphData, ImplementationId } from '../types/graph';
import { generateGraph } from '../generators';

export interface UseGraphGeneratorOptions {
  /** Initial JSON string */
  initialJson?: string;
  /** Debounce delay in ms (default: 300) */
  debounceMs?: number;
}

export interface UseGraphGeneratorReturn {
  /** Current JSON input string */
  jsonInput: string;
  /** Set JSON input string */
  setJsonInput: (json: string) => void;
  /** Parsed JSON (null if invalid) */
  parsedJson: unknown | null;
  /** JSON parse error (null if valid) */
  parseError: string | null;
  /** Whether JSON is currently valid */
  isValid: boolean;
  /** Generate graph for an implementation */
  generateFor: (implementation: ImplementationId) => GraphData | null;
  /** Clear the input */
  clear: () => void;
  /** Set to a sample JSON */
  setSample: (name: SampleName) => void;
}

/** Available sample JSON presets */
export type SampleName = 'api_response' | 'duplicates' | 'nested' | 'simple' | 'mixed';

/** Sample JSON data - ordered to highlight HakkaJson's advantages */
export const SAMPLE_JSON: Record<SampleName, { name: string; json: string }> = {
  // Default: API Response with many repeated string VALUES - shows string interning benefit
  // Keys like "id", "name", "role", "status", "department" repeat across all user objects
  // Values like "active", "engineering", "user", "admin" repeat multiple times
  api_response: {
    name: 'API Response',
    json: JSON.stringify(
      {
        status: 'success',
        data: {
          users: [
            { id: 1, name: 'Alice', role: 'admin', status: 'active', department: 'engineering' },
            { id: 2, name: 'Bob', role: 'user', status: 'active', department: 'engineering' },
            { id: 3, name: 'Charlie', role: 'user', status: 'inactive', department: 'sales' },
            { id: 4, name: 'Diana', role: 'admin', status: 'active', department: 'sales' },
            { id: 5, name: 'Eve', role: 'user', status: 'active', department: 'engineering' },
          ],
          metadata: {
            total: 5,
            page: 1,
            requestStatus: 'success',
            department: 'all',
          },
        },
        message: 'Users retrieved successfully',
      },
      null,
      2
    ),
  },
  // Many duplicate strings - dramatic interning savings
  duplicates: {
    name: 'Heavy Duplicates',
    json: JSON.stringify(
      {
        events: [
          { type: 'click', target: 'button', action: 'submit', status: 'completed' },
          { type: 'click', target: 'button', action: 'cancel', status: 'completed' },
          { type: 'hover', target: 'button', action: 'highlight', status: 'completed' },
          { type: 'click', target: 'link', action: 'navigate', status: 'pending' },
          { type: 'click', target: 'button', action: 'submit', status: 'completed' },
          { type: 'scroll', target: 'page', action: 'load', status: 'completed' },
        ],
        summary: {
          type: 'analytics',
          status: 'completed',
          target: 'dashboard',
        },
      },
      null,
      2
    ),
  },
  // Deep nesting - shows structure handling
  nested: {
    name: 'Deep Nested',
    json: JSON.stringify(
      {
        company: {
          name: 'TechCorp',
          departments: {
            engineering: {
              head: { name: 'Alice', title: 'VP Engineering' },
              teams: {
                frontend: { lead: 'Bob', members: 5 },
                backend: { lead: 'Charlie', members: 8 },
              },
            },
            sales: {
              head: { name: 'Diana', title: 'VP Sales' },
              regions: ['North', 'South', 'East', 'West'],
            },
          },
        },
      },
      null,
      2
    ),
  },
  // Simple for basic testing
  simple: {
    name: 'Simple',
    json: JSON.stringify(
      {
        name: 'test',
        value: 42,
        active: true,
        tags: ['a', 'b', 'c'],
      },
      null,
      2
    ),
  },
  // All JSON types
  mixed: {
    name: 'All Types',
    json: JSON.stringify(
      {
        string: 'hello world',
        integer: 42,
        float: 3.14159,
        boolean_true: true,
        boolean_false: false,
        null_value: null,
        array: [1, 'two', true, null, { nested: 'object' }],
        object: { key1: 'value1', key2: 'value2' },
      },
      null,
      2
    ),
  },
};

/**
 * Hook for generating graphs from user JSON input
 *
 * @example
 * ```tsx
 * const { jsonInput, setJsonInput, isValid, generateFor } = useGraphGenerator();
 *
 * // Generate graph for HakkaJson
 * const graphData = generateFor('hakka_json');
 * ```
 */
export function useGraphGenerator(
  options: UseGraphGeneratorOptions = {}
): UseGraphGeneratorReturn {
  // Default to API Response - highlights string interning benefits
  const { initialJson = SAMPLE_JSON.api_response.json } = options;

  const [jsonInput, setJsonInputState] = useState(initialJson);
  const [parseError, setParseError] = useState<string | null>(null);

  // Parse JSON and validate
  const parsedJson = useMemo(() => {
    if (!jsonInput.trim()) {
      setParseError(null);
      return null;
    }

    try {
      const parsed = JSON.parse(jsonInput);
      setParseError(null);
      return parsed;
    } catch (e) {
      const error = e instanceof Error ? e.message : 'Invalid JSON';
      setParseError(error);
      return null;
    }
  }, [jsonInput]);

  const isValid = parsedJson !== null && parseError === null;

  // Set JSON input with validation
  const setJsonInput = useCallback((json: string) => {
    setJsonInputState(json);
  }, []);

  // Generate graph for specific implementation
  const generateFor = useCallback(
    (implementation: ImplementationId): GraphData | null => {
      if (!isValid || parsedJson === null) {
        return null;
      }

      try {
        return generateGraph(parsedJson, implementation);
      } catch (e) {
        console.error(`Generation error for ${implementation}:`, e);
        return null;
      }
    },
    [isValid, parsedJson]
  );

  // Clear input
  const clear = useCallback(() => {
    setJsonInputState('');
    setParseError(null);
  }, []);

  // Set to a sample
  const setSample = useCallback((name: SampleName) => {
    const sample = SAMPLE_JSON[name];
    if (sample) {
      setJsonInputState(sample.json);
    }
  }, []);

  return {
    jsonInput,
    setJsonInput,
    parsedJson,
    parseError,
    isValid,
    generateFor,
    clear,
    setSample,
  };
}

export default useGraphGenerator;
