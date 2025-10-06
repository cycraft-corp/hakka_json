#!/bin/bash

###############################################################################
# Handle System Performance Analysis Script
#
# This script runs the handle performance benchmark with Linux perf to measure
# actual hardware performance counters (CPU cycles, cache misses, etc.)
#
# Usage: ./run_perf_analysis.sh [benchmark_executable] [output_report]
#
# Requirements:
#   - Linux perf tool (apt install linux-tools-common linux-tools-generic)
#   - Proper permissions (may require: sudo sysctl -w kernel.perf_event_paranoid=-1)
###############################################################################

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BENCHMARK_EXE="${1:-./handle_perf_benchmark}"
OUTPUT_REPORT="${2:-./handle_performance_report.txt}"
PERF_REPORT="${OUTPUT_REPORT%.txt}_perf.txt"

echo -e "${BLUE}==================================================================${NC}"
echo -e "${BLUE}Handle System Performance Analysis${NC}"
echo -e "${BLUE}==================================================================${NC}"
echo -e "Benchmark executable: ${BENCHMARK_EXE}"
echo -e "Output report:        ${OUTPUT_REPORT}"
echo -e "Perf report:          ${PERF_REPORT}"
echo -e "${BLUE}==================================================================${NC}"
echo ""

# Check if benchmark executable exists
if [ ! -f "${BENCHMARK_EXE}" ]; then
    echo -e "${RED}Error: Benchmark executable not found: ${BENCHMARK_EXE}${NC}"
    exit 1
fi

# Check if perf is available
if ! command -v perf &> /dev/null; then
    echo -e "${YELLOW}Warning: perf tool not found. Please install:${NC}"
    echo -e "${YELLOW}  sudo apt install linux-tools-common linux-tools-generic${NC}"
    echo -e "${YELLOW}Falling back to basic benchmark without perf...${NC}"
    echo ""
    ${BENCHMARK_EXE} "${OUTPUT_REPORT}"
    exit 0
fi

# Check perf permissions
PERF_PARANOID=$(cat /proc/sys/kernel/perf_event_paranoid 2>/dev/null || echo "unknown")
if [ "$PERF_PARANOID" != "unknown" ] && [ "$PERF_PARANOID" -gt 1 ]; then
    echo -e "${YELLOW}Warning: perf_event_paranoid = ${PERF_PARANOID}${NC}"
    echo -e "${YELLOW}For detailed performance counters, consider running:${NC}"
    echo -e "${YELLOW}  sudo sysctl -w kernel.perf_event_paranoid=-1${NC}"
    echo -e "${YELLOW}Or running this script with sudo${NC}"
    echo ""
fi

# Step 1: Run the basic benchmark to get timing measurements
echo -e "${GREEN}[1/3] Running basic performance benchmark...${NC}"
${BENCHMARK_EXE} "${OUTPUT_REPORT}"
echo -e "${GREEN}✓ Basic benchmark completed${NC}"
echo ""

# Step 2: Run with perf stat to get hardware counters
echo -e "${GREEN}[2/3] Running perf stat analysis...${NC}"
echo -e "${BLUE}Collecting hardware performance counters...${NC}"

PERF_STAT_OUTPUT=$(mktemp)

# Run perf stat with comprehensive event list
perf stat -e cycles,instructions,cache-references,cache-misses,branches,branch-misses,L1-dcache-loads,L1-dcache-load-misses,LLC-loads,LLC-load-misses \
    -o "${PERF_STAT_OUTPUT}" \
    ${BENCHMARK_EXE} /dev/null 2>&1 > /dev/null || {
    echo -e "${YELLOW}Warning: Full perf stat failed, trying simplified version...${NC}"
    # Fallback to basic counters
    perf stat -e cycles,instructions,cache-references,cache-misses \
        -o "${PERF_STAT_OUTPUT}" \
        ${BENCHMARK_EXE} /dev/null 2>&1 > /dev/null || {
        echo -e "${RED}Error: perf stat failed${NC}"
        exit 1
    }
}

echo -e "${GREEN}✓ Perf stat completed${NC}"
echo ""

# Step 3: Generate comprehensive report
echo -e "${GREEN}[3/3] Generating comprehensive performance report...${NC}"

{
    echo "=================================================================="
    echo "COMPREHENSIVE PERFORMANCE ANALYSIS REPORT"
    echo "=================================================================="
    echo "Generated: $(date)"
    echo "Hostname: $(hostname)"
    echo "Kernel: $(uname -r)"
    echo "CPU: $(lscpu | grep "Model name" | cut -d: -f2 | xargs)"
    echo "CPU MHz: $(lscpu | grep "CPU MHz" | cut -d: -f2 | xargs)"
    echo "=================================================================="
    echo ""
    echo "TIMING MEASUREMENTS:"
    echo "=================================================================="
    cat "${OUTPUT_REPORT}"
    echo ""
    echo ""
    echo "=================================================================="
    echo "HARDWARE PERFORMANCE COUNTERS (from perf stat):"
    echo "=================================================================="
    cat "${PERF_STAT_OUTPUT}"
    echo ""
    echo "=================================================================="
    echo "ANALYSIS:"
    echo "=================================================================="

    # Extract key metrics from perf output
    CYCLES=$(grep -E "^\s*[0-9,]+\s+cycles" "${PERF_STAT_OUTPUT}" | awk '{print $1}' | tr -d ',')
    INSTRUCTIONS=$(grep -E "^\s*[0-9,]+\s+instructions" "${PERF_STAT_OUTPUT}" | awk '{print $1}' | tr -d ',')
    CACHE_REFS=$(grep -E "^\s*[0-9,]+\s+cache-references" "${PERF_STAT_OUTPUT}" | awk '{print $1}' | tr -d ',')
    CACHE_MISSES=$(grep -E "^\s*[0-9,]+\s+cache-misses" "${PERF_STAT_OUTPUT}" | awk '{print $1}' | tr -d ',')

    if [ -n "$CYCLES" ] && [ -n "$INSTRUCTIONS" ] && [ "$CYCLES" -gt 0 ]; then
        IPC=$(echo "scale=3; $INSTRUCTIONS / $CYCLES" | bc)
        echo "Instructions Per Cycle (IPC): ${IPC}"
        echo "  - Higher IPC indicates better CPU utilization"
        echo "  - Typical range: 0.5 - 4.0"
        echo ""
    fi

    if [ -n "$CACHE_REFS" ] && [ -n "$CACHE_MISSES" ] && [ "$CACHE_REFS" -gt 0 ]; then
        CACHE_MISS_RATE=$(echo "scale=4; $CACHE_MISSES / $CACHE_REFS * 100" | bc)
        echo "Cache Miss Rate: ${CACHE_MISS_RATE}%"
        echo "  - Lower is better (fewer cache misses)"
        echo "  - Typical good rate: < 5%"
        echo ""
    fi

    echo "CPU Cycles: ${CYCLES:-N/A}"
    echo "Instructions: ${INSTRUCTIONS:-N/A}"
    echo "Cache References: ${CACHE_REFS:-N/A}"
    echo "Cache Misses: ${CACHE_MISSES:-N/A}"
    echo ""

    echo "=================================================================="
    echo "INTERPRETATION:"
    echo "=================================================================="
    echo ""
    echo "Handle Access Overhead Analysis:"
    echo "  The handle system trades CPU cycles for memory density."
    echo "  Expected overhead: ~37-80 cycles per handle access"
    echo "  vs. ~1 cycle for direct pointer dereference"
    echo ""
    echo "Memory vs Performance Trade-off:"
    echo "  - Memory savings: 50% (32-bit handles vs 64-bit pointers)"
    echo "  - CPU overhead: ~40-80x slower access"
    echo "  - Worth it for: Memory-constrained scenarios with large datasets"
    echo "  - Not worth it for: CPU-bound tasks with frequent access"
    echo ""
    echo "Cache Performance:"
    echo "  Handle indirection may cause additional cache misses due to:"
    echo "  - Manager lookup (registry access)"
    echo "  - Mutex acquisition (synchronization overhead)"
    echo "  - Index array access (additional memory hop)"
    echo ""
    echo "=================================================================="
    echo "RECOMMENDATIONS:"
    echo "=================================================================="
    echo ""
    echo "1. Use handles when:"
    echo "   - Processing large JSON datasets (>1GB)"
    echo "   - Memory is constrained"
    echo "   - Access frequency is moderate"
    echo ""
    echo "2. Avoid handles when:"
    echo "   - Hot path requires frequent access"
    echo "   - Memory is abundant"
    echo "   - Latency is critical"
    echo ""
    echo "3. Optimization opportunities:"
    echo "   - Cache handle views for repeated access"
    echo "   - Batch operations to amortize overhead"
    echo "   - Consider lock-free alternatives for high contention"
    echo ""
    echo "=================================================================="
    echo "END OF COMPREHENSIVE REPORT"
    echo "=================================================================="
} > "${PERF_REPORT}"

# Cleanup
rm -f "${PERF_STAT_OUTPUT}"

echo -e "${GREEN}✓ Comprehensive report generated${NC}"
echo ""
echo -e "${BLUE}==================================================================${NC}"
echo -e "${GREEN}Performance analysis completed successfully!${NC}"
echo -e "${BLUE}==================================================================${NC}"
echo -e "Reports generated:"
echo -e "  1. Basic timing:       ${OUTPUT_REPORT}"
echo -e "  2. Comprehensive:      ${PERF_REPORT}"
echo ""
echo -e "${YELLOW}View reports:${NC}"
echo -e "  cat ${OUTPUT_REPORT}"
echo -e "  cat ${PERF_REPORT}"
echo ""
