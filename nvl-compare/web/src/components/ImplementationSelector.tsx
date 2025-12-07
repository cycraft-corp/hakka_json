import type { ImplementationId } from '../types/graph';

/** Implementation metadata */
interface Implementation {
  id: ImplementationId;
  name: string;
  language: string;
  file: string;
}

/** Available implementations */
export const IMPLEMENTATIONS: Implementation[] = [
  { id: 'hakka_json', name: 'HakkaJson', language: 'C++', file: 'hakka_json.json' },
  { id: 'serde_json', name: 'serde_json', language: 'Rust', file: 'serde_json.json' },
  { id: 'cpython_json', name: 'json', language: 'CPython', file: 'cpython_json.json' },
  { id: 'go_json', name: 'encoding/json', language: 'Go', file: 'go_json.json' },
  { id: 'jansson', name: 'Jansson', language: 'C', file: 'jansson.json' },
];

export interface ImplementationSelectorProps {
  /** Currently selected implementation */
  value: ImplementationId;
  /** Called when selection changes */
  onChange: (id: ImplementationId) => void;
  /** Implementation to exclude from options (for avoiding same selection) */
  excludeValue?: ImplementationId;
  /** Accessible label for the selector */
  label?: string;
  /** Additional CSS classes */
  className?: string;
  /** Dark mode flag */
  isDark?: boolean;
}

/** Language color mapping */
const LANGUAGE_COLORS: Record<string, string> = {
  'C++': '#10b981',    // HakkaJson green
  'Rust': '#f97316',   // serde orange
  'CPython': '#3b82f6', // Python blue
  'Go': '#06b6d4',     // Go cyan
  'C': '#8b5cf6',      // Jansson purple
};

/**
 * Dropdown selector for JSON library implementations
 *
 * @example
 * ```tsx
 * <ImplementationSelector
 *   value={selected}
 *   onChange={setSelected}
 *   excludeValue={otherPanelValue}
 * />
 * ```
 */
export function ImplementationSelector({
  value,
  onChange,
  excludeValue,
  label = 'Select implementation',
  className = '',
  isDark = true,
}: ImplementationSelectorProps): JSX.Element {
  const handleChange = (e: React.ChangeEvent<HTMLSelectElement>) => {
    onChange(e.target.value as ImplementationId);
  };

  // Get display name with language
  const getDisplayName = (impl: Implementation): string => {
    return `${impl.name} (${impl.language})`;
  };

  // Filter out excluded value
  const availableImplementations = excludeValue
    ? IMPLEMENTATIONS.filter((impl) => impl.id !== excludeValue)
    : IMPLEMENTATIONS;

  // Get language for implementation ID
  const getCurrentLanguage = (id: ImplementationId): string => {
    return IMPLEMENTATIONS.find((impl) => impl.id === id)?.language ?? 'Unknown';
  };

  const currentLanguage = getCurrentLanguage(value);

  // SVG arrow color based on theme
  const arrowColor = isDark ? '%2394a3b8' : '%236b7280';

  return (
    <div className={`flex items-center gap-3 ${className}`}>
      <label className="sr-only" htmlFor={`impl-select-${label.replace(/\s/g, '-')}`}>
        {label}
      </label>
      <select
        id={`impl-select-${label.replace(/\s/g, '-')}`}
        value={value}
        onChange={handleChange}
        className={`flex-1 px-3 py-2 text-sm font-medium border rounded-md cursor-pointer appearance-none focus:outline-none focus:border-emerald-500 focus:ring-2 focus:ring-emerald-500/20 transition-colors ${
          isDark
            ? 'bg-slate-900 text-slate-100 border-slate-700 hover:border-emerald-500'
            : 'bg-white text-slate-900 border-slate-300 hover:border-emerald-500'
        }`}
        style={{
          backgroundImage: `url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 20 20' fill='${arrowColor}'%3E%3Cpath fill-rule='evenodd' d='M5.293 7.293a1 1 0 011.414 0L10 10.586l3.293-3.293a1 1 0 111.414 1.414l-4 4a1 1 0 01-1.414 0l-4-4a1 1 0 010-1.414z'/%3E%3C/svg%3E")`,
          backgroundRepeat: 'no-repeat',
          backgroundPosition: 'right 0.5rem center',
          backgroundSize: '1.25rem',
          paddingRight: '2rem',
        }}
      >
        {availableImplementations.map((impl) => (
          <option key={impl.id} value={impl.id}>
            {getDisplayName(impl)}
          </option>
        ))}
      </select>

      {/* Language badge */}
      <span
        className="px-2 py-1 text-xs font-semibold text-white rounded uppercase tracking-wider"
        style={{ backgroundColor: LANGUAGE_COLORS[currentLanguage] ?? '#6b7280' }}
      >
        {currentLanguage}
      </span>
    </div>
  );
}

export default ImplementationSelector;
