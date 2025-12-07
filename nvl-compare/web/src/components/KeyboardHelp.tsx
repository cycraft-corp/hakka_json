import { SHORTCUTS } from '../hooks/useKeyboardShortcuts';

export interface KeyboardHelpProps {
  /** Whether the help overlay is visible */
  visible: boolean;
  /** Callback to close the overlay */
  onClose: () => void;
}

/**
 * Keyboard shortcuts help overlay
 *
 * @example
 * ```tsx
 * <KeyboardHelp visible={showHelp} onClose={() => setShowHelp(false)} />
 * ```
 */
export function KeyboardHelp({ visible, onClose }: KeyboardHelpProps): JSX.Element | null {
  if (!visible) return null;

  // Group shortcuts by category
  const categories = {
    'Left Panel': Object.entries(SHORTCUTS).filter(([key]) => /^[1-5]$/.test(key)),
    'Right Panel': Object.entries(SHORTCUTS).filter(([key]) => key.startsWith('Shift+')),
    'View Controls': Object.entries(SHORTCUTS).filter(
      ([key]) => !key.startsWith('Shift+') && !/^[1-5]$/.test(key)
    ),
  };

  return (
    <div
      className="fixed inset-0 z-50 flex items-center justify-center bg-black/70 backdrop-blur-sm overlay-animate"
      onClick={onClose}
    >
      <div
        className="w-11/12 max-w-lg max-h-[80vh] bg-slate-800 border border-slate-700 rounded-xl shadow-xl overflow-hidden flex flex-col animate-scale-in"
        onClick={(e) => e.stopPropagation()}
      >
        {/* Header */}
        <header className="flex justify-between items-center px-6 py-4 border-b border-slate-700">
          <h2 className="text-xl font-semibold text-slate-100">Keyboard Shortcuts</h2>
          <button
            className="p-2 text-slate-400 hover:text-slate-100 hover:bg-slate-700 rounded-lg transition-colors"
            onClick={onClose}
            aria-label="Close"
          >
            <CloseIcon />
          </button>
        </header>

        {/* Content */}
        <div className="flex-1 overflow-y-auto p-6">
          {Object.entries(categories).map(([category, shortcuts]) => (
            <section key={category} className="mb-6 last:mb-0">
              <h3 className="text-sm font-semibold text-slate-500 uppercase tracking-wider mb-3">
                {category}
              </h3>
              <ul className="space-y-2">
                {shortcuts.map(([key, description]) => (
                  <li key={key} className="flex items-center gap-4">
                    <kbd className="min-w-16 px-2 py-1 font-mono text-sm bg-slate-700 border border-slate-600 rounded text-center text-slate-200">
                      {formatKey(key)}
                    </kbd>
                    <span className="text-sm text-slate-400">{description}</span>
                  </li>
                ))}
              </ul>
            </section>
          ))}
        </div>

        {/* Footer */}
        <footer className="px-6 py-3 border-t border-slate-700 text-center text-xs text-slate-500">
          Press <kbd className="px-1.5 py-0.5 bg-slate-700 border border-slate-600 rounded font-mono">?</kbd> to toggle this help
        </footer>
      </div>
    </div>
  );
}

/** Format key for display */
function formatKey(key: string): string {
  return key
    .replace('Shift+', '\u21e7 ') // Shift symbol
    .replace('Escape', 'Esc');
}

/** Close icon SVG */
function CloseIcon(): JSX.Element {
  return (
    <svg
      width="20"
      height="20"
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth="2"
      strokeLinecap="round"
      strokeLinejoin="round"
    >
      <line x1="18" y1="6" x2="6" y2="18" />
      <line x1="6" y1="6" x2="18" y2="18" />
    </svg>
  );
}

export default KeyboardHelp;
