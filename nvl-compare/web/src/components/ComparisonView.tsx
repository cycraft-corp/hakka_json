import { useState, useCallback, useMemo, useRef, useEffect } from 'react';
import { GraphViewer } from './GraphViewer';
import { ImplementationSelector } from './ImplementationSelector';
import { StatsPanel } from './StatsPanel';
import { Legend } from './Legend';
import { JsonInput } from './JsonInput';
import { useGraphData } from '../hooks/useGraphData';
import { useGraphGenerator, useTheme } from '../hooks';
import type { ImplementationId, NvlNode as GraphNode, GraphData } from '../types/graph';

export type DataMode = 'static' | 'dynamic';

export interface ComparisonViewProps {
  /** Initial left panel implementation */
  initialLeft?: ImplementationId;
  /** Initial right panel implementation */
  initialRight?: ImplementationId;
  /** Graph viewer height */
  graphHeight?: number | string;
  /** Show legend at bottom */
  showLegend?: boolean;
  /** Callback when node is clicked in either panel */
  onNodeClick?: (node: GraphNode, side: 'left' | 'right') => void;
  /** Initial data mode */
  initialMode?: DataMode;
  /** Additional CSS classes */
  className?: string;
}

/**
 * Side-by-side comparison view for JSON library memory graphs
 * Supports both static (pre-computed) and dynamic (user input) modes
 *
 * @example
 * ```tsx
 * <ComparisonView
 *   initialLeft="hakka_json"
 *   initialRight="serde_json"
 *   graphHeight={500}
 *   showLegend
 *   initialMode="dynamic"
 * />
 * ```
 */
export function ComparisonView({
  initialLeft = 'hakka_json',
  initialRight = 'serde_json',
  graphHeight = 500,
  showLegend = true,
  onNodeClick,
  initialMode = 'dynamic',
  className = '',
}: ComparisonViewProps): JSX.Element {
  // Data mode toggle
  const [mode, setMode] = useState<DataMode>(initialMode);

  // Selected implementations
  const [leftImpl, setLeftImpl] = useState<ImplementationId>(initialLeft);
  const [rightImpl, setRightImpl] = useState<ImplementationId>(initialRight);

  // Highlighted nodes (sync between panels when same value found)
  const [highlightedValue, setHighlightedValue] = useState<string | null>(null);

  // Theme toggle
  const { toggleTheme, isDark } = useTheme();

  // Resizable input panel
  const [inputHeight, setInputHeight] = useState(180);
  const resizeRef = useRef<HTMLDivElement>(null);
  const isResizing = useRef(false);
  const startY = useRef(0);
  const startHeight = useRef(0);

  // Handle resize drag
  useEffect(() => {
    const handleMouseMove = (e: MouseEvent) => {
      if (!isResizing.current) return;
      const delta = e.clientY - startY.current;
      const newHeight = Math.max(100, Math.min(500, startHeight.current + delta));
      setInputHeight(newHeight);
    };

    const handleMouseUp = () => {
      isResizing.current = false;
      document.body.style.cursor = '';
      document.body.style.userSelect = '';
    };

    document.addEventListener('mousemove', handleMouseMove);
    document.addEventListener('mouseup', handleMouseUp);

    return () => {
      document.removeEventListener('mousemove', handleMouseMove);
      document.removeEventListener('mouseup', handleMouseUp);
    };
  }, []);

  const handleResizeStart = useCallback((e: React.MouseEvent) => {
    isResizing.current = true;
    startY.current = e.clientY;
    startHeight.current = inputHeight;
    document.body.style.cursor = 'ns-resize';
    document.body.style.userSelect = 'none';
  }, [inputHeight]);

  // Static data (from files)
  const leftStaticData = useGraphData(leftImpl);
  const rightStaticData = useGraphData(rightImpl);

  // Dynamic data (from user input)
  const {
    jsonInput,
    setJsonInput,
    parseError,
    isValid,
    generateFor,
    setSample,
  } = useGraphGenerator();

  // Generate dynamic graphs when JSON changes
  const leftDynamicData = useMemo<GraphData | null>(() => {
    if (mode !== 'dynamic' || !isValid) return null;
    return generateFor(leftImpl);
  }, [mode, isValid, generateFor, leftImpl]);

  const rightDynamicData = useMemo<GraphData | null>(() => {
    if (mode !== 'dynamic' || !isValid) return null;
    return generateFor(rightImpl);
  }, [mode, isValid, generateFor, rightImpl]);

  // Select active data based on mode
  const leftData = mode === 'static' ? leftStaticData.data : leftDynamicData;
  const rightData = mode === 'static' ? rightStaticData.data : rightDynamicData;
  const leftLoading = mode === 'static' && leftStaticData.loading;
  const rightLoading = mode === 'static' && rightStaticData.loading;
  const leftError = mode === 'static' ? leftStaticData.error : null;
  const rightError = mode === 'static' ? rightStaticData.error : null;

  // Handle node hover - highlight matching values across panels
  const handleNodeHover = useCallback((node: GraphNode | null) => {
    if (node?.properties?.value) {
      setHighlightedValue(node.properties.value as string);
    } else {
      setHighlightedValue(null);
    }
  }, []);

  // Get highlighted node IDs based on matching value
  const getHighlightedNodes = useCallback(
    (data: GraphData | null) => {
      if (!highlightedValue || !data) return [];
      return data.nodes
        .filter((n) => n.properties?.value === highlightedValue)
        .map((n) => n.id);
    },
    [highlightedValue]
  );

  // Handle left panel node click
  const handleLeftNodeClick = useCallback(
    (node: GraphNode) => {
      onNodeClick?.(node, 'left');
    },
    [onNodeClick]
  );

  // Handle right panel node click
  const handleRightNodeClick = useCallback(
    (node: GraphNode) => {
      onNodeClick?.(node, 'right');
    },
    [onNodeClick]
  );

  return (
    <div className={`flex flex-col h-screen transition-colors duration-200 ${isDark ? 'bg-slate-950 text-slate-100' : 'bg-slate-50 text-slate-900'} ${className}`}>
      {/* Header with mode toggle */}
      <header className={`px-8 py-4 border-b ${isDark ? 'border-slate-800' : 'border-slate-200'}`}>
        <div className="flex items-center justify-between">
          <div>
            <h1 className="text-xl font-semibold">JSON Memory Graph Comparison</h1>
            <p className={`text-sm mt-1 ${isDark ? 'text-slate-400' : 'text-slate-500'}`}>HakkaJson vs Others</p>
          </div>

          {/* Controls */}
          <div className="flex items-center gap-4">
            {/* Mode Toggle */}
            <div className="flex items-center gap-2">
              <span className={`text-sm ${isDark ? 'text-slate-400' : 'text-slate-500'}`}>Data Mode:</span>
              <div className={`flex rounded-lg overflow-hidden border ${isDark ? 'border-slate-700' : 'border-slate-300'}`}>
                <button
                  onClick={() => setMode('static')}
                  className={`px-3 py-1.5 text-sm font-medium transition-colors ${
                    mode === 'static'
                      ? 'bg-emerald-600 text-white'
                      : isDark
                      ? 'bg-slate-800 text-slate-400 hover:text-slate-200'
                      : 'bg-slate-100 text-slate-600 hover:text-slate-900'
                  }`}
                >
                  Static Files
                </button>
                <button
                  onClick={() => setMode('dynamic')}
                  className={`px-3 py-1.5 text-sm font-medium transition-colors ${
                    mode === 'dynamic'
                      ? 'bg-emerald-600 text-white'
                      : isDark
                      ? 'bg-slate-800 text-slate-400 hover:text-slate-200'
                      : 'bg-slate-100 text-slate-600 hover:text-slate-900'
                  }`}
                >
                  User Input
                </button>
              </div>
            </div>

            {/* Theme Toggle */}
            <button
              onClick={toggleTheme}
              className={`p-2 rounded-lg transition-colors ${isDark ? 'bg-slate-800 hover:bg-slate-700' : 'bg-slate-200 hover:bg-slate-300'}`}
              title={`Switch to ${isDark ? 'light' : 'dark'} mode`}
            >
              {isDark ? (
                <SunIcon className="w-5 h-5 text-amber-400" />
              ) : (
                <MoonIcon className="w-5 h-5 text-slate-600" />
              )}
            </button>
          </div>
        </div>
      </header>

      {/* JSON Input (only in dynamic mode) - resizable */}
      {mode === 'dynamic' && (
        <div className={`relative border-b ${isDark ? 'border-slate-800 bg-slate-900/50' : 'border-slate-200 bg-slate-100/50'}`}>
          <div
            className="px-8 py-4 overflow-auto"
            style={{ height: inputHeight }}
          >
            <JsonInput
              value={jsonInput}
              onChange={setJsonInput}
              error={parseError}
              isValid={isValid}
              onSelectSample={setSample}
              isDark={isDark}
            />
          </div>
          {/* Resize handle */}
          <div
            ref={resizeRef}
            onMouseDown={handleResizeStart}
            className="absolute bottom-0 left-0 right-0 h-2 cursor-ns-resize hover:bg-emerald-500/30 transition-colors group"
          >
            <div className={`absolute left-1/2 -translate-x-1/2 bottom-0.5 w-12 h-1 rounded-full transition-colors ${isDark ? 'bg-slate-600' : 'bg-slate-400'} group-hover:bg-emerald-500`} />
          </div>
        </div>
      )}

      {/* Main comparison area */}
      <div className="flex flex-1 gap-0 p-4 min-h-0">
        {/* Left Panel */}
        <div className={`flex-1 flex flex-col rounded-lg overflow-hidden ${isDark ? 'bg-slate-900' : 'bg-white shadow-md'}`}>
          <div className={`px-4 py-3 border-b ${isDark ? 'border-slate-800' : 'border-slate-200'}`}>
            <ImplementationSelector
              value={leftImpl}
              onChange={setLeftImpl}
              excludeValue={rightImpl}
              label="Left implementation"
              isDark={isDark}
            />
          </div>

          <div className="flex-1 min-h-0 relative" style={{ height: graphHeight }}>
            {leftLoading && <LoadingState />}
            {leftError && <ErrorState error={leftError} />}
            {mode === 'dynamic' && !isValid && <InvalidJsonState />}
            {leftData && (
              <GraphViewer
                data={leftData}
                onNodeClick={handleLeftNodeClick}
                onNodeHover={handleNodeHover}
                highlightedNodes={getHighlightedNodes(leftData)}
                height="100%"
                showControls
                isDark={isDark}
              />
            )}
          </div>

          <div className={`px-4 py-3 border-t ${isDark ? 'border-slate-800 bg-slate-950' : 'border-slate-200 bg-slate-50'}`}>
            {leftData && <StatsPanel data={leftData} isDark={isDark} />}
          </div>
        </div>

        {/* Divider */}
        <div className="w-4 flex-shrink-0" />

        {/* Right Panel */}
        <div className={`flex-1 flex flex-col rounded-lg overflow-hidden ${isDark ? 'bg-slate-900' : 'bg-white shadow-md'}`}>
          <div className={`px-4 py-3 border-b ${isDark ? 'border-slate-800' : 'border-slate-200'}`}>
            <ImplementationSelector
              value={rightImpl}
              onChange={setRightImpl}
              excludeValue={leftImpl}
              label="Right implementation"
              isDark={isDark}
            />
          </div>

          <div className="flex-1 min-h-0 relative" style={{ height: graphHeight }}>
            {rightLoading && <LoadingState />}
            {rightError && <ErrorState error={rightError} />}
            {mode === 'dynamic' && !isValid && <InvalidJsonState />}
            {rightData && (
              <GraphViewer
                data={rightData}
                onNodeClick={handleRightNodeClick}
                onNodeHover={handleNodeHover}
                highlightedNodes={getHighlightedNodes(rightData)}
                height="100%"
                showControls
                isDark={isDark}
              />
            )}
          </div>

          <div className={`px-4 py-3 border-t ${isDark ? 'border-slate-800 bg-slate-950' : 'border-slate-200 bg-slate-50'}`}>
            {rightData && <StatsPanel data={rightData} isDark={isDark} />}
          </div>
        </div>
      </div>

      {/* Legend */}
      {showLegend && (
        <footer className={`px-8 py-4 border-t ${isDark ? 'border-slate-800' : 'border-slate-200'}`}>
          <Legend compact isDark={isDark} />
        </footer>
      )}
    </div>
  );
}

/** Loading state component */
function LoadingState(): JSX.Element {
  return (
    <div className="flex flex-col items-center justify-center h-full gap-4 text-slate-400">
      <div className="w-8 h-8 border-3 border-slate-700 border-t-emerald-500 rounded-full animate-spin" />
      <span>Loading graph data...</span>
    </div>
  );
}

/** Error state component */
function ErrorState({ error }: { error: Error }): JSX.Element {
  return (
    <div className="flex flex-col items-center justify-center h-full gap-4 text-red-400">
      <div className="w-8 h-8 flex items-center justify-center bg-red-500 text-white rounded-full font-bold">
        !
      </div>
      <span>{error.message}</span>
    </div>
  );
}

/** Invalid JSON state component */
function InvalidJsonState(): JSX.Element {
  return (
    <div className="flex flex-col items-center justify-center h-full gap-4 text-slate-500">
      <svg className="w-12 h-12" fill="none" viewBox="0 0 24 24" stroke="currentColor">
        <path
          strokeLinecap="round"
          strokeLinejoin="round"
          strokeWidth={1.5}
          d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z"
        />
      </svg>
      <span>Enter valid JSON to generate graph</span>
    </div>
  );
}

/** Sun icon for light mode */
function SunIcon({ className = '' }: { className?: string }): JSX.Element {
  return (
    <svg className={className} fill="none" viewBox="0 0 24 24" stroke="currentColor">
      <path
        strokeLinecap="round"
        strokeLinejoin="round"
        strokeWidth={2}
        d="M12 3v1m0 16v1m9-9h-1M4 12H3m15.364 6.364l-.707-.707M6.343 6.343l-.707-.707m12.728 0l-.707.707M6.343 17.657l-.707.707M16 12a4 4 0 11-8 0 4 4 0 018 0z"
      />
    </svg>
  );
}

/** Moon icon for dark mode */
function MoonIcon({ className = '' }: { className?: string }): JSX.Element {
  return (
    <svg className={className} fill="none" viewBox="0 0 24 24" stroke="currentColor">
      <path
        strokeLinecap="round"
        strokeLinejoin="round"
        strokeWidth={2}
        d="M20.354 15.354A9 9 0 018.646 3.646 9.003 9.003 0 0012 21a9.003 9.003 0 008.354-5.646z"
      />
    </svg>
  );
}

export default ComparisonView;
