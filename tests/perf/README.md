# Handle System Performance Tests

This directory contains performance benchmarks for the handle system architecture documented in `docs/content/Architecture/Handle.md`.

## Overview

The performance tests measure the actual CPU cycle costs of handle operations compared to direct pointer access, validating the performance characteristics documented in the architecture.

## Benchmark Categories

1. **Baseline**: Direct pointer dereference (~1 cycle expected)
2. **Handle Access**: `get_type()` and `get_view()` operations
3. **Reference Counting**: `retain()` and `release()` overhead
4. **Handle Creation**: Overhead for different JSON types

## Building

```bash
cd build
cmake ..
make handle_perf_benchmark
```

## Running

### Basic Benchmark (Timing Only)

```bash
# From build directory
./tests/perf/handle_perf_benchmark

# Or using CMake target
cmake --build . --target run_perf_benchmark
```

This will:
- Run the benchmark with 10M iterations per test
- Generate `handle_performance_report.txt` with timing results
- Display results on console

### Comprehensive Analysis (with perf)

**Requirements:**
- Linux with `perf` tool installed:
  ```bash
  sudo apt install linux-tools-common linux-tools-generic
  ```
- Proper permissions (optional, for detailed counters):
  ```bash
  sudo sysctl -w kernel.perf_event_paranoid=-1
  ```

**Run:**

```bash
# Using CMake target (recommended)
cmake --build . --target run_perf_stat

# Or manually
cd tests/perf
./run_perf_analysis.sh ../../build/tests/perf/handle_perf_benchmark
```

This will:
- Run basic timing benchmark
- Collect hardware performance counters (CPU cycles, cache misses, IPC)
- Generate comprehensive report with analysis and recommendations
- Output: `handle_performance_report_perf.txt`

## Expected Results

Based on the architecture documentation (Handle.md, lines 327-349):

### handle.get_view() Call Chain

| Operation | Expected Cycles |
|-----------|----------------|
| `get_type()` → `get_manager().type(data)` | ~10-15 |
| `get_manager()` (redundant call) | ~5-10 |
| `manager.get_view(data)` | ~20-50 |
| `std::get<Type*>()` extraction | ~2-5 |
| **Total** | **~37-80** |

### Performance Trade-offs

- **Memory Savings**: 50% (32-bit handles vs 64-bit pointers)
- **CPU Overhead**: ~40-80x slower than direct pointer access
- **Use Case**: Worth it for memory-constrained scenarios with large datasets

## Output Files

1. **handle_performance_report.txt**: Basic timing measurements
   - Nanoseconds and estimated CPU cycles per operation
   - Comparison with documented expectations
   - Analysis of whether results are within expected range

2. **handle_performance_report_perf.txt**: Comprehensive analysis (Linux only)
   - All timing measurements
   - Hardware performance counters
   - Cache miss rates and IPC analysis
   - Detailed recommendations

## Interpreting Results

### Good Performance Indicators

- IPC (Instructions Per Cycle): 0.5 - 4.0
- Cache Miss Rate: < 5%
- Handle access overhead: 37-80 cycles (within expected range)

### Performance Issues

If actual overhead significantly exceeds expected:
- Check for system load (other processes)
- Verify CPU frequency scaling is disabled
- Consider CPU cache effects
- Check for mutex contention

## Continuous Integration

Add to CI pipeline:

```yaml
- name: Run Performance Tests
  run: |
    cd build
    cmake --build . --target run_perf_benchmark
    # Check for performance regressions
    grep "Reality Check" tests/perf/handle_performance_report.txt
```

## Architecture Reference

For detailed information about the handle system design and expected performance characteristics, see:

- **Architecture Docs**: `docs/content/Architecture/Handle.md`
- **Key Section**: Lines 327-349 (Actual Handle Access Cost Analysis)

## Troubleshooting

### perf: Permission Denied

```bash
# Temporary fix (until reboot)
sudo sysctl -w kernel.perf_event_paranoid=-1

# Permanent fix
echo "kernel.perf_event_paranoid = -1" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

### perf: Not Found

```bash
sudo apt update
sudo apt install linux-tools-common linux-tools-generic linux-tools-$(uname -r)
```

### Benchmark Shows Unusually Good Performance

Possible causes:
- Compiler optimizations too aggressive (check `-O2` flag)
- CPU caching effects
- Small data set fits in L1 cache
- Branch prediction working exceptionally well

### Benchmark Shows Unusually Poor Performance

Possible causes:
- System under load (other processes)
- CPU frequency scaling enabled
- Thermal throttling
- Cache contention

## Future Enhancements

Potential additions to the benchmark suite:

1. **Multi-threaded Benchmarks**: Measure contention overhead
2. **Large Dataset Tests**: Test with millions of handles
3. **Memory Pressure Tests**: Measure performance under low memory
4. **Comparison with Direct Pointers**: Side-by-side implementation
5. **Regression Detection**: Automated performance regression tests

## License

Same as hakka_json project.
