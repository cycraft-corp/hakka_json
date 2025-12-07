import { NODE_COLORS } from '../data/schema';
import type { NodeType } from '../types/graph';

export interface LegendProps {
  /** Show only specific categories */
  categories?: LegendCategory[];
  /** Compact horizontal layout */
  compact?: boolean;
  /** Additional CSS classes */
  className?: string;
  /** Dark mode flag */
  isDark?: boolean;
}

type LegendCategory = 'hakka' | 'cpython' | 'serde' | 'go' | 'jansson' | 'common';

interface LegendItem {
  type: NodeType;
  label: string;
  color: string;
  category: LegendCategory;
}

/** Legend items grouped by category */
const LEGEND_ITEMS: LegendItem[] = [
  // HakkaJson
  { type: 'hakka-registry', label: 'Registry', color: NODE_COLORS['hakka-registry'], category: 'hakka' },
  { type: 'hakka-handle', label: 'Handle', color: NODE_COLORS['hakka-handle'], category: 'hakka' },
  { type: 'hakka-interned-string', label: 'Interned', color: NODE_COLORS['hakka-interned-string'], category: 'hakka' },

  // CPython
  { type: 'py-dict', label: 'Dict', color: NODE_COLORS['py-dict'], category: 'cpython' },
  { type: 'py-unicode', label: 'String', color: NODE_COLORS['py-unicode'], category: 'cpython' },
  { type: 'py-long', label: 'Integer', color: NODE_COLORS['py-long'], category: 'cpython' },

  // serde_json
  { type: 'serde-value-enum', label: 'Value', color: NODE_COLORS['serde-value-enum'], category: 'serde' },
  { type: 'serde-string', label: 'String', color: NODE_COLORS['serde-string'], category: 'serde' },
  { type: 'serde-indexmap', label: 'Map', color: NODE_COLORS['serde-indexmap'], category: 'serde' },

  // Go
  { type: 'go-interface', label: 'interface{}', color: NODE_COLORS['go-interface'], category: 'go' },
  { type: 'go-hmap', label: 'Map', color: NODE_COLORS['go-hmap'], category: 'go' },
  { type: 'go-string', label: 'String', color: NODE_COLORS['go-string'], category: 'go' },

  // Jansson
  { type: 'jansson-object', label: 'Object', color: NODE_COLORS['jansson-object'], category: 'jansson' },
  { type: 'jansson-hashtable', label: 'Hashtable', color: NODE_COLORS['jansson-hashtable'], category: 'jansson' },
  { type: 'jansson-string', label: 'String', color: NODE_COLORS['jansson-string'], category: 'jansson' },
];

/** Common special indicators */
const SPECIAL_INDICATORS = [
  { label: 'Overhead', color: '#f87171', description: 'Memory overhead (pointers, refcounts)' },
  { label: 'Duplicate', color: '#fca5a5', description: 'Duplicated string allocation' },
  { label: 'Shared', color: '#a78bfa', description: 'Interned/shared value' },
];

/**
 * Node color legend component
 *
 * @example
 * ```tsx
 * <Legend categories={['hakka', 'serde']} compact />
 * ```
 */
export function Legend({
  categories,
  compact = false,
  className = '',
  isDark = true,
}: LegendProps): JSX.Element {
  // Filter items by category if specified
  const filteredItems = categories
    ? LEGEND_ITEMS.filter((item) => categories.includes(item.category))
    : LEGEND_ITEMS;

  // Group by category
  const groupedItems = groupByCategory(filteredItems);

  if (compact) {
    return (
      <div className={`flex flex-wrap items-center justify-center gap-4 ${className}`}>
        {SPECIAL_INDICATORS.map((indicator) => (
          <LegendDot
            key={indicator.label}
            color={indicator.color}
            label={indicator.label}
            isDark={isDark}
          />
        ))}
      </div>
    );
  }

  return (
    <div className={`flex flex-wrap gap-6 ${className}`}>
      {/* Special indicators */}
      <div className="flex flex-col gap-2">
        <h3 className={`text-xs font-semibold uppercase tracking-wider ${isDark ? 'text-slate-500' : 'text-slate-400'}`}>
          Indicators
        </h3>
        <div className="flex flex-wrap gap-3">
          {SPECIAL_INDICATORS.map((indicator) => (
            <LegendDot
              key={indicator.label}
              color={indicator.color}
              label={indicator.label}
              tooltip={indicator.description}
              isDark={isDark}
            />
          ))}
        </div>
      </div>

      {/* Implementation-specific legends */}
      {Object.entries(groupedItems).map(([category, items]) => (
        <div key={category} className="flex flex-col gap-2">
          <h3 className={`text-xs font-semibold uppercase tracking-wider ${isDark ? 'text-slate-500' : 'text-slate-400'}`}>
            {getCategoryTitle(category)}
          </h3>
          <div className="flex flex-wrap gap-3">
            {items.map((item) => (
              <LegendDot
                key={item.type}
                color={item.color}
                label={item.label}
                isDark={isDark}
              />
            ))}
          </div>
        </div>
      ))}
    </div>
  );
}

/** Single legend dot */
function LegendDot({
  color,
  label,
  tooltip,
  isDark = true,
}: {
  color: string;
  label: string;
  tooltip?: string;
  isDark?: boolean;
}): JSX.Element {
  return (
    <div className="flex items-center gap-1.5 cursor-default" title={tooltip}>
      <span
        className="w-3 h-3 rounded-full flex-shrink-0"
        style={{ backgroundColor: color }}
      />
      <span className={`text-sm ${isDark ? 'text-slate-200' : 'text-slate-700'}`}>{label}</span>
    </div>
  );
}

/** Group items by category */
function groupByCategory(items: LegendItem[]): Record<string, LegendItem[]> {
  return items.reduce(
    (acc, item) => {
      if (!acc[item.category]) {
        acc[item.category] = [];
      }
      acc[item.category].push(item);
      return acc;
    },
    {} as Record<string, LegendItem[]>
  );
}

/** Get display title for category */
function getCategoryTitle(category: string): string {
  const titles: Record<string, string> = {
    hakka: 'HakkaJson',
    cpython: 'CPython',
    serde: 'serde_json',
    go: 'Go',
    jansson: 'Jansson',
    common: 'Common',
  };
  return titles[category] ?? category;
}

export default Legend;
