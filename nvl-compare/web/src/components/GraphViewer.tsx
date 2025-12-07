import { useRef, useEffect, useState, useCallback, useMemo } from 'react';
import { InteractiveNvlWrapper } from '@neo4j-nvl/react';
import type { NVL, Node as NvlNodeBase, Relationship as NvlRelBase, NvlOptions } from '@neo4j-nvl/base';
import type { GraphData, NvlNode as GraphNode } from '../types/graph';
import { NODE_COLORS, NODE_SIZES, RELATIONSHIP_COLORS } from '../data/schema';
import NodeTooltip from './NodeTooltip';

// ============================================================
// Types
// ============================================================

interface GraphViewerProps {
  /** Graph data to visualize */
  data: GraphData;
  /** Container width (default: 100%) */
  width?: number | string;
  /** Container height (default: 600px) */
  height?: number | string;
  /** Callback when a node is clicked */
  onNodeClick?: (node: GraphNode) => void;
  /** Callback when a node is hovered */
  onNodeHover?: (node: GraphNode | null) => void;
  /** IDs of nodes to highlight */
  highlightedNodes?: string[];
  /** Layout algorithm */
  layout?: 'force' | 'hierarchical';
  /** Show zoom controls */
  showControls?: boolean;
  /** Initial zoom level (0.1 - 2.0) */
  initialZoom?: number;
  /** Class name for container */
  className?: string;
  /** Dark mode flag */
  isDark?: boolean;
}

interface TooltipState {
  node: GraphNode | null;
  x: number;
  y: number;
  visible: boolean;
}

// ============================================================
// Styling Helpers
// ============================================================

/**
 * Get node color from schema, with fallback
 */
function getNodeColor(nodeType: string): string {
  return NODE_COLORS[nodeType as keyof typeof NODE_COLORS] ?? '#6B7280';
}

/**
 * Get node size from schema, with fallback
 */
function getNodeSize(nodeType: string): number {
  return NODE_SIZES[nodeType as keyof typeof NODE_SIZES] ?? 20;
}

/**
 * Get relationship color from schema, with fallback
 */
function getRelationshipColor(relType: string): string {
  return RELATIONSHIP_COLORS[relType as keyof typeof RELATIONSHIP_COLORS] ?? '#9CA3AF';
}

/**
 * Transform graph data to NVL format with styling
 */
function transformToNvlFormat(
  data: GraphData,
  highlightedNodes?: string[]
): { nodes: NvlNodeBase[]; relationships: NvlRelBase[] } {
  const highlightSet = new Set(highlightedNodes ?? []);

  const nodes: NvlNodeBase[] = data.nodes.map((node) => ({
    id: node.id,
    size: getNodeSize(node.properties?.type ?? ''),
    color: getNodeColor(node.properties?.type ?? ''),
    caption: node.caption ?? node.label,
    // Highlight effect
    ...(highlightSet.has(node.id) && {
      color: '#FBBF24', // Amber highlight
      size: getNodeSize(node.properties?.type ?? '') * 1.3,
    }),
    // Store original data for tooltips
    properties: {
      ...node.properties,
      label: node.label,
      caption: node.caption,
    },
  }));

  const relationships: NvlRelBase[] = data.relationships.map((rel) => ({
    id: rel.id,
    from: rel.from,
    to: rel.to,
    caption: rel.type.replace(/-/g, ' '),
    color: getRelationshipColor(rel.type),
    width: 1.5,
    properties: rel.properties,
  }));

  return { nodes, relationships };
}

// ============================================================
// Component
// ============================================================

/**
 * Interactive graph viewer component using Neo4j NVL
 *
 * @example
 * ```tsx
 * <GraphViewer
 *   data={graphData}
 *   height={500}
 *   onNodeClick={(node) => console.log('Clicked:', node)}
 *   highlightedNodes={['node-1', 'node-2']}
 * />
 * ```
 */
export function GraphViewer({
  data,
  width = '100%',
  height = 600,
  onNodeClick,
  onNodeHover,
  highlightedNodes,
  layout = 'force',
  showControls = true,
  initialZoom = 1.0,
  className = '',
  isDark = true,
}: GraphViewerProps): JSX.Element {
  const nvlRef = useRef<NVL | null>(null);
  const containerRef = useRef<HTMLDivElement>(null);

  const [tooltip, setTooltip] = useState<TooltipState>({
    node: null,
    x: 0,
    y: 0,
    visible: false,
  });

  // Transform data with memoization
  const { nodes, relationships } = useMemo(
    () => transformToNvlFormat(data, highlightedNodes),
    [data, highlightedNodes]
  );

  // Node click handler
  const handleNodeClick = useCallback(
    (clickedNode: NvlNodeBase) => {
      const originalNode = data.nodes.find((n) => n.id === clickedNode.id);
      if (originalNode && onNodeClick) {
        onNodeClick(originalNode);
      }
    },
    [data.nodes, onNodeClick]
  );

  // Hover handler - use the combined onHover callback
  const handleHover = useCallback(
    (element: NvlNodeBase | NvlRelBase, _hitElements: unknown, event: MouseEvent) => {
      // Check if it's a node (has 'id' but no 'from'/'to' - relationships have those)
      const isNode = element && !('from' in element);
      if (isNode) {
        const hoveredNode = element as NvlNodeBase;
        const originalNode = data.nodes.find((n) => n.id === hoveredNode.id);
        if (originalNode) {
          setTooltip({
            node: originalNode,
            x: event.clientX,
            y: event.clientY,
            visible: true,
          });
          onNodeHover?.(originalNode);
        }
      } else if (!element) {
        // No element hovered - clear tooltip
        setTooltip((prev) => ({ ...prev, visible: false }));
        onNodeHover?.(null);
      }
    },
    [data.nodes, onNodeHover]
  );

  // NVL configuration
  const nvlOptions: NvlOptions = useMemo(
    () => ({
      layout: layout === 'force' ? 'd3Force' : 'hierarchical',
      initialZoom,
      relationshipThreshold: 0.5,
      renderer: 'webgl',
    }),
    [layout, initialZoom]
  );

  // Interaction options - enable panning even when dragging near nodes
  const interactionOptions = useMemo(
    () => ({
      // Pan: allow panning even when near nodes (don't exclude node margin)
      excludeNodeMargin: true,
      // Click: don't auto-select on click (we handle selection manually)
      selectOnClick: false,
    }),
    []
  );

  // Zoom controls
  const handleZoomIn = useCallback(() => {
    const nvl = nvlRef.current;
    if (nvl) {
      const currentZoom = nvl.getScale?.() ?? 1;
      nvl.setZoom(currentZoom * 1.2);
    }
  }, []);

  const handleZoomOut = useCallback(() => {
    const nvl = nvlRef.current;
    if (nvl) {
      const currentZoom = nvl.getScale?.() ?? 1;
      nvl.setZoom(currentZoom * 0.8);
    }
  }, []);

  const handleFitToScreen = useCallback(() => {
    const nvl = nvlRef.current;
    if (nvl) {
      // Get all node IDs for fit
      const nodeIds = nodes.map((n) => n.id);
      nvl.fit(nodeIds);
    }
  }, [nodes]);

  // Update highlight effect when highlightedNodes changes
  useEffect(() => {
    if (nvlRef.current && highlightedNodes) {
      nvlRef.current.updateElementsInGraph(nodes, []);
    }
  }, [highlightedNodes, nodes]);

  return (
    <div
      ref={containerRef}
      className={`relative rounded-lg overflow-hidden transition-colors ${isDark ? 'bg-gray-900' : 'bg-slate-100'} ${className}`}
      style={{ width, height }}
    >
      {/* NVL Graph - supports mouse drag to pan, scroll to zoom, drag nodes */}
      <InteractiveNvlWrapper
        ref={nvlRef}
        nodes={nodes}
        rels={relationships}
        nvlOptions={nvlOptions}
        interactionOptions={interactionOptions}
        mouseEventCallbacks={{
          // Node interactions
          onNodeClick: handleNodeClick,
          onHover: handleHover,
          // Enable drag node - set to true for default behavior
          onDrag: true,
          onDragStart: true,
          onDragEnd: true,
          // Enable pan - set to true for default behavior
          onPan: true,
          // Enable zoom - set to true for default behavior
          onZoom: true,
        }}
      />

      {/* Zoom Controls */}
      {showControls && (
        <div className="absolute bottom-4 right-4 flex flex-col gap-2">
          <button
            onClick={handleZoomIn}
            className={`p-2 rounded transition-colors ${isDark ? 'bg-gray-800 hover:bg-gray-700 text-white' : 'bg-white hover:bg-gray-100 text-gray-700 shadow'}`}
            title="Zoom In"
            aria-label="Zoom In"
          >
            <svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 4v16m8-8H4" />
            </svg>
          </button>
          <button
            onClick={handleZoomOut}
            className={`p-2 rounded transition-colors ${isDark ? 'bg-gray-800 hover:bg-gray-700 text-white' : 'bg-white hover:bg-gray-100 text-gray-700 shadow'}`}
            title="Zoom Out"
            aria-label="Zoom Out"
          >
            <svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M20 12H4" />
            </svg>
          </button>
          <button
            onClick={handleFitToScreen}
            className={`p-2 rounded transition-colors ${isDark ? 'bg-gray-800 hover:bg-gray-700 text-white' : 'bg-white hover:bg-gray-100 text-gray-700 shadow'}`}
            title="Fit to Screen"
            aria-label="Fit to Screen"
          >
            <svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M4 8V4m0 0h4M4 4l5 5m11-1V4m0 0h-4m4 0l-5 5M4 16v4m0 0h4m-4 0l5-5m11 5l-5-5m5 5v-4m0 4h-4" />
            </svg>
          </button>
        </div>
      )}

      {/* Node Tooltip */}
      <NodeTooltip
        node={tooltip.node}
        x={tooltip.x}
        y={tooltip.y}
        visible={tooltip.visible}
        isDark={isDark}
      />

      {/* Stats Overlay */}
      <div className={`absolute top-2 left-2 px-2 py-1 rounded text-xs ${isDark ? 'bg-gray-800/80 text-gray-300' : 'bg-white/90 text-gray-600 shadow'}`}>
        {data.stats?.nodeCount ?? data.nodes.length} nodes · {data.stats?.edgeCount ?? data.relationships.length} edges
      </div>

      {/* Interaction Hint */}
      <div className={`absolute top-2 right-2 px-2 py-1 rounded text-xs ${isDark ? 'bg-gray-800/60 text-gray-400' : 'bg-white/80 text-gray-500 shadow'}`}>
        Drag canvas to pan · Drag node to move · Scroll to zoom
      </div>
    </div>
  );
}

export default GraphViewer;
