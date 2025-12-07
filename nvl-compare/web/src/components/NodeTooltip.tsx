import { useMemo } from 'react';
import type { NvlNode } from '../types/graph';

// ============================================================
// Types
// ============================================================

interface NodeTooltipProps {
  /** The node to display info for */
  node: NvlNode | null;
  /** X position (client coordinates) */
  x: number;
  /** Y position (client coordinates) */
  y: number;
  /** Whether tooltip is visible */
  visible: boolean;
  /** Offset from cursor */
  offset?: { x: number; y: number };
  /** Dark mode flag */
  isDark?: boolean;
}

interface PropertyDisplayProps {
  label: string;
  value: string | number | boolean | undefined;
  highlight?: boolean;
  isDark?: boolean;
}

// ============================================================
// Sub-components
// ============================================================

function PropertyDisplay({ label, value, highlight = false, isDark = true }: PropertyDisplayProps): JSX.Element | null {
  if (value === undefined || value === null) return null;

  const displayValue = typeof value === 'boolean'
    ? (value ? 'Yes' : 'No')
    : String(value);

  return (
    <div className="flex justify-between gap-4 text-xs">
      <span className={isDark ? 'text-gray-400' : 'text-gray-500'}>{label}:</span>
      <span className={highlight ? 'text-amber-500 font-medium' : isDark ? 'text-gray-200' : 'text-gray-700'}>
        {displayValue}
      </span>
    </div>
  );
}

// ============================================================
// Helper Functions
// ============================================================

/**
 * Format bytes to human readable string
 */
function formatBytes(bytes: number): string {
  if (bytes === 0) return '0 B';
  if (bytes < 1024) return `${bytes} B`;
  if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`;
  return `${(bytes / (1024 * 1024)).toFixed(2)} MB`;
}

/**
 * Get implementation name from type string
 */
function getImplementationName(type: string | undefined): string {
  if (!type) return 'Unknown';

  const prefixMap: Record<string, string> = {
    'hakka-': 'HakkaJson',
    'py-': 'CPython',
    'serde-': 'serde_json',
    'go-': 'Go',
    'jansson-': 'Jansson',
    'pod-': 'POD',
    'rust-': 'Rust',
  };

  for (const [prefix, name] of Object.entries(prefixMap)) {
    if (type.startsWith(prefix)) return name;
  }

  return 'Unknown';
}

/**
 * Calculate tooltip position with boundary detection
 */
function calculatePosition(
  x: number,
  y: number,
  offset: { x: number; y: number }
): { left: string; top: string; transform: string } {
  const tooltipWidth = 280;
  const tooltipHeight = 200;
  const margin = 10;

  const viewportWidth = typeof window !== 'undefined' ? window.innerWidth : 1920;
  const viewportHeight = typeof window !== 'undefined' ? window.innerHeight : 1080;

  // Determine horizontal position
  const wouldOverflowRight = x + offset.x + tooltipWidth + margin > viewportWidth;
  const left = wouldOverflowRight ? `${x - offset.x - tooltipWidth}px` : `${x + offset.x}px`;

  // Determine vertical position
  const wouldOverflowBottom = y + offset.y + tooltipHeight + margin > viewportHeight;
  const top = wouldOverflowBottom ? `${y - offset.y - tooltipHeight}px` : `${y + offset.y}px`;

  return { left, top, transform: 'none' };
}

// ============================================================
// Component
// ============================================================

/**
 * Tooltip component displaying detailed node information
 *
 * @example
 * ```tsx
 * <NodeTooltip
 *   node={hoveredNode}
 *   x={mouseX}
 *   y={mouseY}
 *   visible={isHovering}
 * />
 * ```
 */
export function NodeTooltip({
  node,
  x,
  y,
  visible,
  offset = { x: 15, y: 15 },
  isDark = true,
}: NodeTooltipProps): JSX.Element | null {
  const position = useMemo(
    () => calculatePosition(x, y, offset),
    [x, y, offset]
  );

  if (!visible || !node) {
    return null;
  }

  const props = node.properties ?? {} as Record<string, unknown>;
  const type = props.type as string | undefined;
  const implementation = getImplementationName(type);
  const sizeBytes = props.sizeBytes as number | undefined;
  const isOverhead = props.isOverhead as boolean | undefined;
  const isDuplicateNode = props.isDuplicate as boolean | undefined;
  const refCount = props.refCount as number | undefined;
  const value = props.value as string | undefined;

  return (
    <div
      className="fixed z-50 pointer-events-none"
      style={{
        left: position.left,
        top: position.top,
        transform: position.transform,
      }}
    >
      <div className={`rounded-lg shadow-xl p-3 min-w-[200px] max-w-[280px] ${isDark ? 'bg-gray-900 border border-gray-700' : 'bg-white border border-gray-200'}`}>
        {/* Header */}
        <div className={`pb-2 mb-2 ${isDark ? 'border-b border-gray-700' : 'border-b border-gray-200'}`}>
          <div className={`font-semibold text-sm ${isDark ? 'text-white' : 'text-gray-900'}`}>
            {node.label}
          </div>
          {node.caption && node.caption !== node.label && (
            <div className={`text-xs mt-0.5 ${isDark ? 'text-gray-400' : 'text-gray-500'}`}>
              {node.caption}
            </div>
          )}
        </div>

        {/* Properties */}
        <div className="space-y-1">
          <PropertyDisplay
            label="Type"
            value={type}
            isDark={isDark}
          />
          <PropertyDisplay
            label="Implementation"
            value={implementation}
            isDark={isDark}
          />
          {sizeBytes !== undefined && (
            <PropertyDisplay
              label="Size"
              value={formatBytes(sizeBytes)}
              highlight={sizeBytes > 100}
              isDark={isDark}
            />
          )}
          {value !== undefined && (
            <PropertyDisplay
              label="Value"
              value={`"${value}"`}
              isDark={isDark}
            />
          )}
          {refCount !== undefined && (
            <PropertyDisplay
              label="Ref Count"
              value={refCount}
              isDark={isDark}
            />
          )}
        </div>

        {/* Flags */}
        {(isOverhead || isDuplicateNode) && (
          <div className={`flex gap-2 mt-2 pt-2 ${isDark ? 'border-t border-gray-700' : 'border-t border-gray-200'}`}>
            {isOverhead && (
              <span className={`px-1.5 py-0.5 text-xs rounded ${isDark ? 'bg-red-900/50 text-red-300' : 'bg-red-100 text-red-700'}`}>
                Overhead
              </span>
            )}
            {isDuplicateNode && (
              <span className={`px-1.5 py-0.5 text-xs rounded ${isDark ? 'bg-amber-900/50 text-amber-300' : 'bg-amber-100 text-amber-700'}`}>
                Duplicate
              </span>
            )}
          </div>
        )}

        {/* Node ID (for debugging) */}
        <div className={`mt-2 pt-2 ${isDark ? 'border-t border-gray-700' : 'border-t border-gray-200'}`}>
          <div className={`text-[10px] font-mono ${isDark ? 'text-gray-500' : 'text-gray-400'}`}>
            ID: {node.id}
          </div>
        </div>
      </div>
    </div>
  );
}

export default NodeTooltip;
