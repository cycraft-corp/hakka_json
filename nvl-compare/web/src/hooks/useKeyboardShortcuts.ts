import { useEffect, useCallback } from 'react';
import type { ImplementationId } from '../types/graph';

export interface KeyboardShortcutsConfig {
  /** Callback when implementation should change */
  onImplementationChange?: (id: ImplementationId, side: 'left' | 'right') => void;
  /** Callback to toggle fullscreen */
  onToggleFullscreen?: () => void;
  /** Callback to toggle help overlay */
  onToggleHelp?: () => void;
  /** Callback to reset zoom on both panels */
  onResetZoom?: () => void;
  /** Callback to toggle legend visibility */
  onToggleLegend?: () => void;
  /** Whether shortcuts are enabled */
  enabled?: boolean;
}

/** Implementation key mappings */
const IMPL_KEYS: Record<string, ImplementationId> = {
  '1': 'hakka_json',
  '2': 'serde_json',
  '3': 'cpython_json',
  '4': 'go_json',
  '5': 'jansson',
};

/** All keyboard shortcuts */
export const SHORTCUTS = {
  // Implementation selection
  '1': 'Select HakkaJson (left panel)',
  '2': 'Select serde_json (left panel)',
  '3': 'Select CPython json (left panel)',
  '4': 'Select Go encoding/json (left panel)',
  '5': 'Select Jansson (left panel)',
  'Shift+1': 'Select HakkaJson (right panel)',
  'Shift+2': 'Select serde_json (right panel)',
  'Shift+3': 'Select CPython json (right panel)',
  'Shift+4': 'Select Go encoding/json (right panel)',
  'Shift+5': 'Select Jansson (right panel)',

  // View controls
  'f': 'Toggle fullscreen mode',
  'l': 'Toggle legend visibility',
  'r': 'Reset zoom (both panels)',
  '?': 'Show/hide keyboard shortcuts',
  'Escape': 'Exit fullscreen / Close help',
} as const;

/**
 * Hook for keyboard navigation in comparison view
 *
 * @example
 * ```tsx
 * useKeyboardShortcuts({
 *   onImplementationChange: (id, side) => {
 *     if (side === 'left') setLeftImpl(id);
 *     else setRightImpl(id);
 *   },
 *   onToggleFullscreen: () => setFullscreen(!fullscreen),
 * });
 * ```
 */
export function useKeyboardShortcuts({
  onImplementationChange,
  onToggleFullscreen,
  onToggleHelp,
  onResetZoom,
  onToggleLegend,
  enabled = true,
}: KeyboardShortcutsConfig): { shortcuts: typeof SHORTCUTS } {
  const handleKeyDown = useCallback(
    (event: KeyboardEvent) => {
      // Skip if disabled or in input field
      if (!enabled) return;

      const target = event.target as HTMLElement;
      if (
        target.tagName === 'INPUT' ||
        target.tagName === 'TEXTAREA' ||
        target.tagName === 'SELECT' ||
        target.isContentEditable
      ) {
        return;
      }

      const key = event.key;
      const shift = event.shiftKey;

      // Implementation selection (1-5)
      if (IMPL_KEYS[key] && onImplementationChange) {
        event.preventDefault();
        const side = shift ? 'right' : 'left';
        onImplementationChange(IMPL_KEYS[key], side);
        return;
      }

      // View controls
      switch (key.toLowerCase()) {
        case 'f':
          event.preventDefault();
          onToggleFullscreen?.();
          break;

        case 'l':
          event.preventDefault();
          onToggleLegend?.();
          break;

        case 'r':
          event.preventDefault();
          onResetZoom?.();
          break;

        case '?':
          event.preventDefault();
          onToggleHelp?.();
          break;

        case 'escape':
          // Let fullscreen/help handlers decide what to close
          onToggleHelp?.(); // Could close help if open
          break;
      }
    },
    [enabled, onImplementationChange, onToggleFullscreen, onToggleHelp, onResetZoom, onToggleLegend]
  );

  useEffect(() => {
    if (!enabled) return;

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [enabled, handleKeyDown]);

  return { shortcuts: SHORTCUTS };
}

export default useKeyboardShortcuts;
