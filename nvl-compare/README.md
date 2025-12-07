# NVL-Compare: JSON Memory Graph Visualizer

Interactive visualization tool comparing how different JSON libraries represent data in memory, using Neo4j NVL (Network Visualization Library).

## Live Demo

**[Launch Memory Visualizer](https://cycraft.github.io/hakka_json/nvl-compare/)**

## Purpose

See how HakkaJson's memory-efficient architecture compares to other JSON implementations:

| Implementation | Language | Key Characteristics |
|----------------|----------|---------------------|
| **HakkaJson** | C++ | Handles, arenas, string interning, NaN-boxing, PicoString |
| **serde_json** | Rust | Enum-based values, ownership semantics |
| **json** | CPython | PyObject hierarchy, reference counting |
| **encoding/json** | Go | Interface-based with reflection |
| **Jansson** | C | Union-based with reference counting |

## Features

### Dynamic JSON Input
- Enter any JSON and see real-time memory graph generation
- Pre-built sample JSON templates for common patterns
- Syntax validation with error highlighting

### Side-by-Side Comparison
- Compare two implementations simultaneously
- Synchronized hover highlighting for matching values
- Interactive node drag, zoom, and pan

### Memory Analysis
- **Total Size**: Estimated memory footprint in bytes
- **Memory Overhead**: Structural overhead + duplicate allocations
- **Interned Strings**: Shared string references (HakkaJson)
- **Duplicates**: Wasted memory from repeated allocations
- **NaN-boxed Values**: Efficient scalar storage (HakkaJson)

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `1-5` | Select implementation (left panel) |
| `Shift+1-5` | Select implementation (right panel) |
| `f` | Toggle fullscreen |
| `l` | Toggle legend |
| `r` | Reset zoom |
| `?` | Show keyboard help |

### Theme Support
- Light and dark mode toggle
- Resizable JSON input panel

## Architecture

```
nvl-compare/
└── web/                       # React + Vite application
    └── src/
        ├── components/        # UI components
        │   ├── ComparisonView.tsx   # Main comparison layout
        │   ├── GraphViewer.tsx      # NVL graph renderer
        │   ├── JsonInput.tsx        # JSON editor with samples
        │   ├── StatsPanel.tsx       # Memory statistics
        │   ├── Legend.tsx           # Node type legend
        │   └── NodeTooltip.tsx      # Hover details
        ├── generators/        # Memory graph generators
        │   ├── hakka.ts       # HakkaJson simulation
        │   ├── serde.ts       # serde_json simulation
        │   ├── cpython.ts     # CPython json simulation
        │   ├── go.ts          # Go encoding/json simulation
        │   └── jansson.ts     # Jansson simulation
        ├── hooks/             # React hooks
        │   ├── useGraphGenerator.ts  # JSON → Graph conversion
        │   ├── useTheme.ts           # Light/dark mode
        │   └── useKeyboardShortcuts.ts
        └── types/             # TypeScript definitions
```

## Development

```bash
cd web
npm install
npm run dev
# Open http://localhost:5173
```

### Build

```bash
npm run build     # Production build
npm run preview   # Preview production build
npm run lint      # ESLint check
npm run typecheck # TypeScript check
```

## Deployment

Deployed automatically to GitHub Pages via GitHub Actions when changes are pushed to `main`.

The app is served at `/nvl-compare/` under the main HakkaJson documentation site.

See `.github/workflows/mkdocs.yml` for the combined docs + app build workflow.

## How It Works

Each generator simulates how a JSON library would allocate memory:

1. **Parse JSON** → Abstract representation
2. **Generate Nodes** → Memory allocations (objects, arrays, strings, scalars)
3. **Generate Edges** → Relationships (contains, references, manages)
4. **Calculate Stats** → Total size, overhead, duplicates

### HakkaJson Specifics

- **Handles**: 4-byte tokens instead of 8-byte pointers
- **String Interning**: Identical strings share memory
- **PicoString**: Tiered fixed-size storage (1-64 bytes) for short strings
- **NaN-boxing**: Scalars encoded in 8 bytes without wrapper objects
- **Arena Allocation**: Registry-managed memory pools

### Memory Overhead Calculation

```
Overhead = Structural Overhead + Wasted Memory
         = (wrappers + pointers + infrastructure)
         + (duplicate allocations of immutable values)
```

## Tech Stack

- **React 18** + TypeScript
- **Vite 6** (build tool)
- **Neo4j NVL** (graph visualization)
- **Tailwind CSS** (styling)

## License

BSD
