import { useState, useCallback } from 'react';
import { ComparisonView, KeyboardHelp } from './components';
import { useKeyboardShortcuts, useFullscreen } from './hooks';
import type { ImplementationId, NvlNode as GraphNode } from './types/graph';

function App() {
  // State for implementations
  const [leftImpl, setLeftImpl] = useState<ImplementationId>('hakka_json');
  const [rightImpl, setRightImpl] = useState<ImplementationId>('serde_json');

  // UI state
  const [showLegend, setShowLegend] = useState(true);
  const [showHelp, setShowHelp] = useState(false);

  // Fullscreen hook
  const { isFullscreen, toggleFullscreen } = useFullscreen({
    onChange: (fs) => console.log('Fullscreen:', fs),
  });

  // Handle implementation change from keyboard
  const handleImplementationChange = useCallback(
    (id: ImplementationId, side: 'left' | 'right') => {
      if (side === 'left') {
        // Don't allow same selection
        if (id !== rightImpl) setLeftImpl(id);
      } else {
        if (id !== leftImpl) setRightImpl(id);
      }
    },
    [leftImpl, rightImpl]
  );

  // Keyboard shortcuts
  useKeyboardShortcuts({
    onImplementationChange: handleImplementationChange,
    onToggleFullscreen: toggleFullscreen,
    onToggleHelp: () => setShowHelp((prev) => !prev),
    onToggleLegend: () => setShowLegend((prev) => !prev),
    onResetZoom: () => {
      // Dispatch custom event for GraphViewer to handle
      window.dispatchEvent(new CustomEvent('resetZoom'));
    },
  });

  // Node click handler
  const handleNodeClick = useCallback((node: GraphNode, side: 'left' | 'right') => {
    console.log(`Clicked "${node.label ?? node.id}" on ${side} panel`, node.properties);
  }, []);

  return (
    <div className={isFullscreen ? 'fullscreen-mode' : ''}>
      <ComparisonView
        initialLeft={leftImpl}
        initialRight={rightImpl}
        graphHeight={isFullscreen ? 'calc(100vh - 200px)' : 500}
        showLegend={showLegend}
        onNodeClick={handleNodeClick}
      />

      {/* Keyboard help overlay */}
      <KeyboardHelp visible={showHelp} onClose={() => setShowHelp(false)} />

      {/* Keyboard hint (bottom right) */}
      <div className="keyboard-hint hide-mobile">
        Press <kbd>?</kbd> for shortcuts
      </div>
    </div>
  );
}

export default App;
