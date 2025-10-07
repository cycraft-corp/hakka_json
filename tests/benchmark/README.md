# Memory Consumption Benchmarks for JSON Parsing Libraries

This directory provides a comparative analysis of resident memory consumption across multiple JSON parsing implementations. The benchmark measures RSS (Resident Set Size) delta during JSON document loading to quantify the memory overhead of different parsing and representation strategies.

## Implementations Under Test

| Implementation | Version | Language | Representation Strategy |
|----------------|---------|----------|-------------------------|
| hakka_json | current | C++23 | Compact handle-based with string deduplication |
| nlohmann/json | 3.12+ | C++17 | DOM tree with `std::variant` |
| jansson | 2.14+ | C | Reference-counted DOM tree |
| cppstd | N/A | C++17 | Recursive descent parser with `std::variant` + `std::unordered_map` + `std::vector` |
| Python | 3.10, 3.13 | Python | Native `dict`/`list` structures |
| Go | 1.23 | Go | `map[string]interface{}` + reflection |
| Rust | 1.90 | Rust | serde_json's `Value` enum |

## Methodology

### Measurement Technique

Memory consumption is measured using the Linux `/proc/self/status` interface, specifically the VmRSS field. The measurement protocol:

1. Record RSS before JSON loading (`RSS_before`)
2. Parse input file line-by-line (newline-delimited JSON)
3. Store all parsed objects in memory
4. Record RSS after loading (`RSS_after`)
5. Report delta: `RSS_delta = RSS_after - RSS_before`

This approach isolates the memory overhead of the JSON representation from baseline process overhead.

### Compilation Settings

All compiled benchmarks use aggressive optimization:

- **C/C++ targets**: `-O3 -march=native -mtune=native -flto` (GCC/Clang) or `/O2 /GL /LTCG` (MSVC)
- **Interprocedural optimization**: Enabled where supported (CMake `INTERPROCEDURAL_OPTIMIZATION`)
- **Go**: `-ldflags "-s -w"` (strip symbols), `-gcflags "-l=4"` (aggressive inlining)
- **Rust**: `--release` profile (opt-level 3, LTO enabled by default)
- **Python**: Standard interpreter (no optimization flags)

### Test Data

**Dataset**: ClickHouse JSON dataset from [jsonbench.com](https://jsonbench.com/)
- **File**: `file_0001.json`
- **Format**: Newline-delimited JSON (NDJSON)
- **Size**: 481 MB (504,889,344 bytes)
- **Characteristics**: Real-world database export with nested objects and repeated keys

## Build Instructions

Benchmarks are disabled by default. Enable via CMake configuration:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DHAKKA_JSON_ENABLE_BENCHMARKS=ON
cmake --build build --target benchmarks
```

Individual targets: `benchmark_hakka`, `benchmark_nlohmann`, `benchmark_jansson`, `benchmark_cppstd`, `benchmark_golang`, `benchmark_rust`

## Execution

All benchmarks accept a single positional argument specifying the input file path:

```bash
cd build/tests/benchmark
./benchmark_hakka /path/to/input.json
./benchmark_jansson /path/to/input.json
./benchmark_cppstd /path/to/input.json
./benchmark_golang /path/to/input.json
./benchmark_rust /path/to/input.json
./benchmark_python310 /path/to/input.json
./benchmark_python313 /path/to/input.json
```

Output format: `<implementation> RSS: <delta_kb> KB`

## Experimental Results

### Memory Consumption (481 MB Input)

| Implementation | RSS Delta (KB) | RSS Delta (MB) | Memory Amplification Factor |
|----------------|----------------|----------------|----------------------------|
| hakka_json | 1,525,972 | 1,490 | 3.10x |
| golang | 2,311,984 | 2,258 | 4.70x |
| python3.13 | 2,376,168 | 2,321 | 4.83x |
| cppstd | 2,836,796 | 2,770 | 5.76x |
| python3.10 | 2,874,956 | 2,808 | 5.84x |
| jansson | 3,081,548 | 3,009 | 6.26x |
| rust (serde_json) | 3,405,940 | 3,326 | 6.92x |

**Memory Amplification Factor**: Ratio of RSS delta to input file size.

### Relative Performance

All values normalized to hakka_json as baseline:

| Implementation | Relative RSS | Overhead vs. Baseline |
|----------------|--------------|----------------------|
| hakka_json | 1.00x | — |
| golang | 1.52x | +52% |
| python3.13 | 1.56x | +56% |
| cppstd | 1.86x | +86% |
| python3.10 | 1.88x | +88% |
| jansson | 2.02x | +102% |
| rust (serde_json) | 2.23x | +123% |

## Analysis

### Memory Efficiency Ordering

The benchmark demonstrates substantial variation in memory overhead across implementations. hakka_json exhibits the lowest memory amplification factor (3.10x), approximately 34% lower than the second-place implementation (golang, 4.70x) and 55% lower than the highest consumer (rust/serde_json, 6.92x).

### Implementation Characteristics

**hakka_json**: Employs handle-based compact representation with string deduplication, optimized for read-heavy workloads with repeated string values.

**golang**: Standard library `encoding/json` uses `map[string]interface{}` with interface boxing overhead.

**python**: CPython dict/list objects with reference counting and pre-allocated capacity.

**cppstd**: Naive recursive descent parser using standard library containers without optimization for memory efficiency.

**jansson**: C library with reference-counted objects and hash tables.

**rust/serde_json**: Enum-based `Value` type with heap allocation for compound types.

## System Requirements

- **Platform**: Linux (x86_64 or ARM64)
- **Kernel**: `/proc` filesystem support for RSS measurement
- **Build Dependencies**:
  - C++23-capable compiler (GCC 11+, Clang 17+, MSVC 2022+)
  - CMake 3.20+
  - pkg-config (for jansson detection)
  - [uv](https://github.com/astral-sh/uv) (for Python benchmarks)
  - Go 1.23+ (optional)
  - Rust 1.90+ with Cargo (optional)
  - jansson ≥2.14 (optional, `libjansson-dev` on Debian/Ubuntu)
  - nlohmann/json ≥3.12 (optional, `libnlohmann-json3-dev` on Debian/Ubuntu)

## Implementation Details

### C++ Standard Library Benchmark

The `cppstd` benchmark implements a minimal recursive descent parser using only C++17 standard library facilities:

- JSON values: `std::variant<std::monostate, bool, double, std::string, JsonArray, JsonObject>`
- Objects: `std::unordered_map<std::string, JsonValue>`
- Arrays: `std::vector<JsonValue>`

This serves as a baseline for standard library overhead without specialized JSON optimizations.

### Python Benchmarks

Python benchmarks execute via the `uv` tool to ensure consistent runtime environments:

```bash
uv run --python 3.10 python_bench.py <input>
uv run --python 3.13 python_bench.py <input>
```

Both use the standard library `json` module without third-party dependencies.

## Limitations and Caveats

1. **Single-threaded execution**: All benchmarks execute in a single thread. Multi-threaded scenarios may exhibit different memory patterns.

2. **Read-only workload**: Benchmarks only measure parsing and storage. Modification workloads are not evaluated.

3. **Platform-specific measurements**: RSS measurement via `/proc/self/status` is Linux-specific. Results may vary on other operating systems.

4. **Input data dependency**: Memory consumption patterns depend on JSON structure. Results may differ for datasets with different key distribution, nesting depth, or value types.

5. **Allocator effects**: Memory consumption includes allocator metadata and fragmentation. Different allocators (jemalloc, tcmalloc, mimalloc) may produce different results.

6. **JIT compilation**: Python and interpreted language benchmarks may include JIT compilation overhead in RSS measurements.

7. **Garbage collection**: Languages with garbage collectors (Python, Go) may retain additional memory due to collection heuristics.

## Reproducibility

To reproduce these results:

```bash
# Download test data
wget https://jsonbench.com/datasets/clickhouse/file_0001.json -O /tmp/test.json

# Build benchmarks
cmake -B build -DCMAKE_BUILD_TYPE=Release -DHAKKA_JSON_ENABLE_BENCHMARKS=ON
cmake --build build --target benchmarks

# Execute
cd build/tests/benchmark
for bench in benchmark_*; do
    ./$bench /tmp/test.json
done
```

## References

- RSS measurement: `proc(5)` manual page
- NDJSON format: http://ndjson.org/
- Test data source: https://jsonbench.com/
