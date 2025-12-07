import type { GraphData } from '../types/graph';

export interface StatsPanelProps {
  /** Graph data to display stats for */
  data: GraphData;
  /** Show compact version */
  compact?: boolean;
  /** Additional CSS classes */
  className?: string;
  /** Dark mode flag */
  isDark?: boolean;
}

interface StatItem {
  label: string;
  value: number | string;
  highlight?: 'good' | 'bad' | 'neutral';
  tooltip?: string;
}

/**
 * Statistics panel showing graph metrics
 *
 * @example
 * ```tsx
 * <StatsPanel data={graphData} />
 * ```
 */
export function StatsPanel({
  data,
  compact = false,
  className = '',
  isDark = true,
}: StatsPanelProps): JSX.Element {
  const stats = computeStats(data);

  if (compact) {
    return (
      <div className={`flex items-center gap-2 text-sm ${isDark ? 'text-slate-400' : 'text-slate-500'} ${className}`}>
        <span>
          <strong className={isDark ? 'text-slate-200' : 'text-slate-700'}>{stats.nodeCount}</strong> nodes
        </span>
        <span className={isDark ? 'text-slate-600' : 'text-slate-300'}>|</span>
        <span>
          <strong className={isDark ? 'text-slate-200' : 'text-slate-700'}>{stats.edgeCount}</strong> edges
        </span>
        <span className={isDark ? 'text-slate-600' : 'text-slate-300'}>|</span>
        <span>
          <strong className={isDark ? 'text-slate-200' : 'text-slate-700'}>{formatBytes(stats.totalBytes)}</strong>
        </span>
      </div>
    );
  }

  return (
    <div className={`flex flex-col gap-3 ${className}`}>
      <div className="grid grid-cols-3 gap-3 sm:grid-cols-4 lg:grid-cols-6">
        {stats.items.map((item, index) => (
          <StatItemDisplay key={index} item={item} isDark={isDark} />
        ))}
      </div>

      {/* Overhead bar visualization */}
      {stats.overheadPercent !== undefined && (
        <div className="mt-2">
          <div className="flex justify-between text-xs mb-1">
            <span className={isDark ? 'text-slate-400' : 'text-slate-500'}>Memory Overhead</span>
            <span className={`font-semibold ${getOverheadTextClass(stats.overheadPercent, isDark)}`}>
              {stats.overheadPercent.toFixed(1)}%
            </span>
          </div>
          <div className={`h-2 rounded overflow-hidden ${isDark ? 'bg-slate-800' : 'bg-slate-200'}`}>
            <div
              className={`h-full transition-all duration-300 ${getOverheadBarClass(stats.overheadPercent)}`}
              style={{ width: `${Math.min(stats.overheadPercent, 100)}%` }}
            />
          </div>
          <p className={`mt-2 text-xs leading-relaxed ${isDark ? 'text-slate-500' : 'text-slate-400'}`}>
            <span className={`font-medium ${isDark ? 'text-slate-400' : 'text-slate-500'}`}>Memory Overhead</span> =
            Structural overhead (wrapper types, pointers, infrastructure) +
            Wasted memory (duplicate allocations of immutable values like strings, numbers, booleans, null).
          </p>
        </div>
      )}
    </div>
  );
}

/** Single stat item display */
function StatItemDisplay({ item, isDark = true }: { item: StatItem; isDark?: boolean }): JSX.Element {
  const valueClass = item.highlight === 'good'
    ? isDark ? 'text-emerald-400' : 'text-emerald-600'
    : item.highlight === 'bad'
    ? isDark ? 'text-red-400' : 'text-red-600'
    : isDark ? 'text-slate-100' : 'text-slate-800';

  return (
    <div className="flex flex-col gap-0.5" title={item.tooltip}>
      <span className={`text-xl font-semibold ${valueClass}`}>{item.value}</span>
      <span className={`text-xs uppercase tracking-wider ${isDark ? 'text-slate-500' : 'text-slate-400'}`}>{item.label}</span>
    </div>
  );
}

/** Compute statistics from graph data */
function computeStats(data: GraphData): {
  nodeCount: number;
  edgeCount: number;
  totalBytes: number;
  overheadBytes?: number;
  overheadPercent?: number;
  items: StatItem[];
} {
  const nodeCount = data.nodes.length;
  const edgeCount = data.relationships.length;

  // Use stats from data if available (computed by generators)
  const statsFromData = data.stats ?? {};

  // Calculate total bytes (use computed stats or sum from nodes)
  const totalBytes = statsFromData.totalBytes ?? data.nodes.reduce(
    (sum, node) => sum + (node.properties?.sizeBytes ?? 0),
    0
  );

  // Calculate overhead bytes: structural overhead (isOverhead) + wasted memory (isDuplicate)
  const overheadBytes = statsFromData.overheadBytes ?? data.nodes
    .filter((n) => n.properties?.isOverhead || n.properties?.isDuplicate)
    .reduce((sum, node) => sum + (node.properties?.sizeBytes ?? 0), 0);

  // Calculate overhead percentage
  const overheadPercent =
    statsFromData.overheadPercent ??
    (totalBytes > 0 ? (overheadBytes / totalBytes) * 100 : 0);

  // Count special node types
  const internedCount = data.nodes.filter(
    (n) => n.properties?.type?.includes('interned')
  ).length;

  const duplicateCount = data.nodes.filter(
    (n) => n.properties?.isDuplicate
  ).length;

  // Build stat items
  const items: StatItem[] = [
    { label: 'Nodes', value: nodeCount },
    { label: 'Edges', value: edgeCount },
    { label: 'Total Size', value: formatBytes(totalBytes) },
  ];

  // Add implementation-specific stats
  if (internedCount > 0) {
    items.push({
      label: 'Interned',
      value: internedCount,
      highlight: 'good',
      tooltip: 'Shared string references (memory efficient)',
    });
  }

  if (duplicateCount > 0) {
    items.push({
      label: 'Duplicates',
      value: duplicateCount,
      highlight: 'bad',
      tooltip: 'Duplicate string allocations (wasted memory)',
    });
  }

  if (statsFromData.refCountedObjects) {
    items.push({
      label: 'Ref-Counted',
      value: statsFromData.refCountedObjects,
      highlight: 'neutral',
    });
  }

  if (statsFromData.interfaceWrappers) {
    items.push({
      label: 'interface{}',
      value: statsFromData.interfaceWrappers,
      highlight: 'bad',
      tooltip: 'Go interface wrappers (overhead)',
    });
  }

  if (statsFromData.nanBoxedValues) {
    items.push({
      label: 'NaN-boxed',
      value: statsFromData.nanBoxedValues,
      highlight: 'good',
      tooltip: 'Efficient NaN-boxed scalar values',
    });
  }

  return {
    nodeCount,
    edgeCount,
    totalBytes,
    overheadBytes,
    overheadPercent,
    items,
  };
}

/** Format bytes to human-readable string */
function formatBytes(bytes: number): string {
  if (bytes === 0) return '0 B';
  if (bytes < 1024) return `${bytes} B`;
  if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`;
  return `${(bytes / (1024 * 1024)).toFixed(2)} MB`;
}

/** Get CSS class for overhead text based on percentage */
function getOverheadTextClass(percent: number, isDark = true): string {
  if (percent < 16) return isDark ? 'text-emerald-400' : 'text-emerald-600';
  if (percent < 30) return isDark ? 'text-amber-400' : 'text-amber-600';
  return isDark ? 'text-red-400' : 'text-red-600';
}

/** Get CSS class for overhead bar based on percentage */
function getOverheadBarClass(percent: number): string {
  if (percent < 16) return 'bg-emerald-500';
  if (percent < 30) return 'bg-amber-500';
  return 'bg-red-500';
}

export default StatsPanel;
