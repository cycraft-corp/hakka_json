import { useState, useEffect, useCallback } from 'react';
import type { GraphData, ImplementationId } from '../types/graph';

interface UseGraphDataOptions {
  /** Enable caching of loaded data */
  cache?: boolean;
}

interface UseGraphDataReturn {
  data: GraphData | null;
  loading: boolean;
  error: Error | null;
  refetch: () => void;
}

// Simple in-memory cache
const dataCache = new Map<ImplementationId, GraphData>();

/**
 * Hook to load graph data for a specific implementation
 *
 * @param implementation - The implementation ID to load
 * @param options - Hook options
 * @returns Graph data state and controls
 *
 * @example
 * ```tsx
 * const { data, loading, error } = useGraphData('hakka_json');
 * if (loading) return <Spinner />;
 * if (error) return <Error message={error.message} />;
 * return <GraphViewer data={data} />;
 * ```
 */
export function useGraphData(
  implementation: ImplementationId,
  options: UseGraphDataOptions = {}
): UseGraphDataReturn {
  const { cache = true } = options;

  const [data, setData] = useState<GraphData | null>(() => {
    // Check cache on initial render
    if (cache && dataCache.has(implementation)) {
      return dataCache.get(implementation) ?? null;
    }
    return null;
  });
  const [loading, setLoading] = useState(!data);
  const [error, setError] = useState<Error | null>(null);

  const fetchData = useCallback(async () => {
    // Check cache first
    if (cache && dataCache.has(implementation)) {
      setData(dataCache.get(implementation) ?? null);
      setLoading(false);
      return;
    }

    setLoading(true);
    setError(null);

    try {
      const response = await fetch(`/data/${implementation}.json`);

      if (!response.ok) {
        throw new Error(
          `Failed to load ${implementation}: ${response.status} ${response.statusText}`
        );
      }

      const graphData: GraphData = await response.json();

      // Validate basic structure
      if (!graphData.nodes || !graphData.relationships) {
        throw new Error(`Invalid graph data structure for ${implementation}`);
      }

      // Cache the result
      if (cache) {
        dataCache.set(implementation, graphData);
      }

      setData(graphData);
    } catch (err) {
      setError(err instanceof Error ? err : new Error(String(err)));
      setData(null);
    } finally {
      setLoading(false);
    }
  }, [implementation, cache]);

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  const refetch = useCallback(() => {
    // Clear cache for this implementation before refetching
    dataCache.delete(implementation);
    fetchData();
  }, [implementation, fetchData]);

  return { data, loading, error, refetch };
}

/**
 * Preload graph data for multiple implementations
 * Useful for warming the cache on app startup
 */
export function preloadGraphData(implementations: ImplementationId[]): void {
  implementations.forEach(async (impl) => {
    if (dataCache.has(impl)) return;

    try {
      const response = await fetch(`/data/${impl}.json`);
      if (response.ok) {
        const data = await response.json();
        dataCache.set(impl, data);
      }
    } catch {
      // Silently fail preload - actual load will handle errors
    }
  });
}

/**
 * Clear all cached graph data
 */
export function clearGraphDataCache(): void {
  dataCache.clear();
}

export default useGraphData;
