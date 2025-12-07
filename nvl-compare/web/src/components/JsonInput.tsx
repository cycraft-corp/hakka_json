import { useCallback, useRef, useEffect } from 'react';
import type { SampleName } from '../hooks/useGraphGenerator';
import { SAMPLE_JSON } from '../hooks/useGraphGenerator';

export interface JsonInputProps {
  /** Current JSON value */
  value: string;
  /** Called when JSON changes */
  onChange: (json: string) => void;
  /** Parse error message (if any) */
  error: string | null;
  /** Whether the JSON is valid */
  isValid: boolean;
  /** Called when a sample is selected */
  onSelectSample: (name: SampleName) => void;
  /** Placeholder text */
  placeholder?: string;
  /** Additional CSS classes */
  className?: string;
  /** Dark mode flag */
  isDark?: boolean;
}

/**
 * JSON input component with validation and sample selection
 *
 * @example
 * ```tsx
 * <JsonInput
 *   value={jsonInput}
 *   onChange={setJsonInput}
 *   error={parseError}
 *   isValid={isValid}
 *   onSelectSample={setSample}
 * />
 * ```
 */
export function JsonInput({
  value,
  onChange,
  error,
  isValid,
  onSelectSample,
  placeholder = 'Enter JSON here...',
  className = '',
  isDark = true,
}: JsonInputProps): JSX.Element {
  const textareaRef = useRef<HTMLTextAreaElement>(null);

  // Auto-resize textarea
  useEffect(() => {
    const textarea = textareaRef.current;
    if (textarea) {
      textarea.style.height = 'auto';
      textarea.style.height = `${Math.min(textarea.scrollHeight, 300)}px`;
    }
  }, [value]);

  const handleChange = useCallback(
    (e: React.ChangeEvent<HTMLTextAreaElement>) => {
      onChange(e.target.value);
    },
    [onChange]
  );

  const handleFormat = useCallback(() => {
    try {
      const parsed = JSON.parse(value);
      onChange(JSON.stringify(parsed, null, 2));
    } catch {
      // Can't format invalid JSON
    }
  }, [value, onChange]);

  const handleMinify = useCallback(() => {
    try {
      const parsed = JSON.parse(value);
      onChange(JSON.stringify(parsed));
    } catch {
      // Can't minify invalid JSON
    }
  }, [value, onChange]);

  const handleClear = useCallback(() => {
    onChange('');
  }, [onChange]);

  return (
    <div className={`flex flex-col gap-3 ${className}`}>
      {/* Header with samples */}
      <div className="flex flex-wrap items-center justify-between gap-2">
        <div className="flex items-center gap-2">
          <span className={`text-sm font-medium ${isDark ? 'text-slate-400' : 'text-slate-500'}`}>Samples:</span>
          {(Object.keys(SAMPLE_JSON) as SampleName[]).map((name) => (
            <button
              key={name}
              onClick={() => onSelectSample(name)}
              className={`px-2 py-1 text-xs font-medium rounded transition-colors ${
                isDark
                  ? 'text-slate-300 bg-slate-800 hover:bg-slate-700'
                  : 'text-slate-600 bg-slate-200 hover:bg-slate-300'
              }`}
            >
              {SAMPLE_JSON[name].name}
            </button>
          ))}
        </div>

        <div className="flex items-center gap-2">
          <button
            onClick={handleFormat}
            disabled={!isValid}
            className={`px-2 py-1 text-xs font-medium disabled:opacity-50 disabled:cursor-not-allowed rounded transition-colors ${
              isDark
                ? 'text-slate-300 bg-slate-800 hover:bg-slate-700'
                : 'text-slate-600 bg-slate-200 hover:bg-slate-300'
            }`}
            title="Format JSON"
          >
            Format
          </button>
          <button
            onClick={handleMinify}
            disabled={!isValid}
            className={`px-2 py-1 text-xs font-medium disabled:opacity-50 disabled:cursor-not-allowed rounded transition-colors ${
              isDark
                ? 'text-slate-300 bg-slate-800 hover:bg-slate-700'
                : 'text-slate-600 bg-slate-200 hover:bg-slate-300'
            }`}
            title="Minify JSON"
          >
            Minify
          </button>
          <button
            onClick={handleClear}
            className={`px-2 py-1 text-xs font-medium rounded transition-colors ${
              isDark
                ? 'text-red-400 bg-slate-800 hover:bg-red-900/30'
                : 'text-red-600 bg-slate-200 hover:bg-red-100'
            }`}
            title="Clear"
          >
            Clear
          </button>
        </div>
      </div>

      {/* Textarea */}
      <div className="relative">
        <textarea
          ref={textareaRef}
          value={value}
          onChange={handleChange}
          placeholder={placeholder}
          spellCheck={false}
          className={`
            w-full min-h-[120px] max-h-[300px] p-3
            font-mono text-sm leading-relaxed
            border rounded-lg resize-none
            focus:outline-none focus:ring-2
            transition-colors
            ${isDark ? 'bg-slate-900 text-slate-100' : 'bg-white text-slate-900'}
            ${
              error
                ? 'border-red-500/50 focus:border-red-500 focus:ring-red-500/20'
                : isValid
                ? 'border-emerald-500/30 focus:border-emerald-500 focus:ring-emerald-500/20'
                : isDark
                ? 'border-slate-700 focus:border-slate-600 focus:ring-slate-500/20'
                : 'border-slate-300 focus:border-slate-400 focus:ring-slate-400/20'
            }
          `}
        />

        {/* Validation indicator */}
        <div className="absolute top-2 right-2">
          {value.trim() && (
            <span
              className={`
                inline-flex items-center gap-1 px-2 py-0.5 text-xs font-medium rounded
                ${isValid
                  ? isDark ? 'bg-emerald-900/50 text-emerald-400' : 'bg-emerald-100 text-emerald-700'
                  : isDark ? 'bg-red-900/50 text-red-400' : 'bg-red-100 text-red-700'
                }
              `}
            >
              {isValid ? (
                <>
                  <CheckIcon /> Valid
                </>
              ) : (
                <>
                  <XIcon /> Invalid
                </>
              )}
            </span>
          )}
        </div>
      </div>

      {/* Error message */}
      {error && (
        <div className={`px-3 py-2 text-sm rounded ${
          isDark
            ? 'text-red-400 bg-red-900/20 border border-red-500/30'
            : 'text-red-700 bg-red-50 border border-red-200'
        }`}>
          <span className="font-medium">Parse Error:</span> {error}
        </div>
      )}

      {/* Stats (when valid) */}
      {isValid && value.trim() && (
        <div className={`flex items-center gap-4 text-xs ${isDark ? 'text-slate-500' : 'text-slate-400'}`}>
          <span>{value.length} characters</span>
          <span>{countKeys(value)} keys</span>
          <span>{countValues(value)} values</span>
        </div>
      )}
    </div>
  );
}

/** Count approximate number of object keys in JSON */
function countKeys(json: string): number {
  try {
    const parsed = JSON.parse(json);
    return countKeysRecursive(parsed);
  } catch {
    return 0;
  }
}

function countKeysRecursive(obj: unknown): number {
  if (obj === null || typeof obj !== 'object') return 0;
  if (Array.isArray(obj)) {
    return obj.reduce((sum, item) => sum + countKeysRecursive(item), 0);
  }
  const keys = Object.keys(obj);
  return keys.length + keys.reduce((sum, k) => sum + countKeysRecursive((obj as Record<string, unknown>)[k]), 0);
}

/** Count approximate number of values in JSON */
function countValues(json: string): number {
  try {
    const parsed = JSON.parse(json);
    return countValuesRecursive(parsed);
  } catch {
    return 0;
  }
}

function countValuesRecursive(obj: unknown): number {
  if (obj === null || typeof obj !== 'object') return 1;
  if (Array.isArray(obj)) {
    return obj.reduce((sum, item) => sum + countValuesRecursive(item), 0);
  }
  return Object.values(obj).reduce((sum, v) => sum + countValuesRecursive(v), 0);
}

/** Check icon */
function CheckIcon(): JSX.Element {
  return (
    <svg className="w-3 h-3" fill="none" viewBox="0 0 24 24" stroke="currentColor">
      <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
    </svg>
  );
}

/** X icon */
function XIcon(): JSX.Element {
  return (
    <svg className="w-3 h-3" fill="none" viewBox="0 0 24 24" stroke="currentColor">
      <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M6 18L18 6M6 6l12 12" />
    </svg>
  );
}

export default JsonInput;
