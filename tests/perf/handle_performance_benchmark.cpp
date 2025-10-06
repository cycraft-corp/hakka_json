/**
 * Performance benchmark for Handle System Architecture
 *
 * Measures actual CPU cycle costs of handle operations as documented in
 * docs/content/Architecture/Handle.md
 *
 * Test Categories:
 * 1. Direct pointer dereference (baseline: ~1 cycle)
 * 2. Handle get_type() overhead (~10-15 cycles expected)
 * 3. Handle get_view() overhead (~20-50 cycles expected)
 * 4. Reference counting (retain/release) overhead
 * 5. Handle creation overhead
 */

#include <hakka_json_int.hpp>
#include <hakka_json_float.hpp>
#include <hakka_json_string.hpp>
#include <hakka_json_array.hpp>
#include <hakka_json_object.hpp>
#include <hakka_json_handle.hpp>

#include <chrono>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>

using namespace hakka;

// Benchmark configuration
constexpr size_t ITERATIONS = 10000000;  // 10M iterations for stable measurements
constexpr size_t WARMUP_ITERATIONS = 100000;  // Warmup to stabilize CPU

// Helper to prevent compiler optimization
#if defined(_MSC_VER)
template<typename T>
__declspec(noinline)
void DoNotOptimize(const T& value) {
    _ReadWriteBarrier();
    const volatile void* p = &value;
    (void)p;
}
#else
template<typename T>
__attribute__((noinline))
void DoNotOptimize(const T& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}
#endif

// Timing helper
class Timer {
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    TimePoint start_time;

public:
    void start() {
        start_time = Clock::now();
    }

    double elapsed_ns() const {
        auto end = Clock::now();
        return std::chrono::duration<double, std::nano>(end - start_time).count();
    }
};

// Baseline: Direct pointer dereference
double benchmark_direct_pointer_access() {
    // Create a simple struct to dereference
    struct SimpleData {
        int value;
        double data;
    };

    SimpleData obj{42, 3.14};
    SimpleData* ptr = &obj;

    // Warmup
    volatile int sink = 0;
    for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
        sink = ptr->value;
        DoNotOptimize(sink);
    }

    // Actual measurement
    Timer timer;
    timer.start();

    for (size_t i = 0; i < ITERATIONS; ++i) {
        sink = ptr->value;
        DoNotOptimize(sink);
    }

    return timer.elapsed_ns() / ITERATIONS;
}

// Test 1: Handle get_type() overhead
double benchmark_handle_get_type() {
    auto handle = JsonIntCompact::create(42);

    // Warmup
    volatile HakkaJsonType type;
    for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
        type = handle.get_type();
        DoNotOptimize(type);
    }

    // Actual measurement
    Timer timer;
    timer.start();

    for (size_t i = 0; i < ITERATIONS; ++i) {
        type = handle.get_type();
        DoNotOptimize(type);
    }

    return timer.elapsed_ns() / ITERATIONS;
}

// Test 2: Handle get_view() overhead
double benchmark_handle_get_view() {
    auto handle = JsonIntCompact::create(42);

    // Warmup
    for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
        auto view = handle.get_view();
        DoNotOptimize(view);
    }

    // Actual measurement
    Timer timer;
    timer.start();

    for (size_t i = 0; i < ITERATIONS; ++i) {
        auto view = handle.get_view();
        DoNotOptimize(view);
    }

    return timer.elapsed_ns() / ITERATIONS;
}

// Test 3: Full handle access chain (get_type + get_view)
double benchmark_handle_full_chain() {
    auto handle = JsonIntCompact::create(42);

    // Warmup
    for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
        auto type = handle.get_type();
        auto view = handle.get_view();
        DoNotOptimize(type);
        DoNotOptimize(view);
    }

    // Actual measurement
    Timer timer;
    timer.start();

    for (size_t i = 0; i < ITERATIONS; ++i) {
        auto type = handle.get_type();
        auto view = handle.get_view();
        DoNotOptimize(type);
        DoNotOptimize(view);
    }

    return timer.elapsed_ns() / ITERATIONS;
}

// Test 4: Reference counting overhead (copy constructor)
double benchmark_handle_retain() {
    auto handle = JsonIntCompact::create(42);

    // Warmup
    for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
        JsonHandleCompact copy = handle;
        DoNotOptimize(copy);
    }

    // Actual measurement
    Timer timer;
    timer.start();

    for (size_t i = 0; i < ITERATIONS; ++i) {
        JsonHandleCompact copy = handle;
        DoNotOptimize(copy);
    }

    return timer.elapsed_ns() / ITERATIONS;
}

// Test 5: Handle creation overhead (different types)
double benchmark_handle_creation_int() {
    // Warmup
    for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
        auto handle = JsonIntCompact::create(static_cast<int64_t>(i));
        DoNotOptimize(handle);
    }

    // Actual measurement
    Timer timer;
    timer.start();

    for (size_t i = 0; i < ITERATIONS; ++i) {
        auto handle = JsonIntCompact::create(static_cast<int64_t>(i % 1000));
        DoNotOptimize(handle);
    }

    return timer.elapsed_ns() / ITERATIONS;
}

double benchmark_handle_creation_float() {
    // Warmup
    for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
        auto handle = JsonFloatCompact::create(static_cast<double>(i) * 3.14);
        DoNotOptimize(handle);
    }

    // Actual measurement
    Timer timer;
    timer.start();

    for (size_t i = 0; i < ITERATIONS; ++i) {
        auto handle = JsonFloatCompact::create(static_cast<double>(i % 1000) * 3.14);
        DoNotOptimize(handle);
    }

    return timer.elapsed_ns() / ITERATIONS;
}

double benchmark_handle_creation_string() {
    // Warmup
    for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
        auto handle = JsonStringCompact::create("test_string");
        DoNotOptimize(handle);
    }

    // Actual measurement
    Timer timer;
    timer.start();

    for (size_t i = 0; i < ITERATIONS; ++i) {
        auto handle = JsonStringCompact::create("test_string");
        DoNotOptimize(handle);
    }

    return timer.elapsed_ns() / ITERATIONS;
}

// Estimate CPU cycles from nanoseconds (assumes 3.0 GHz CPU, will be calibrated)
double ns_to_cycles(double ns, double cpu_ghz = 3.0) {
    return ns * cpu_ghz;
}

void print_results(std::ostream& out, const std::string& test_name,
                   double time_ns, double baseline_ns = 0.0) {
    double cycles = ns_to_cycles(time_ns);

    out << std::left << std::setw(50) << test_name << ": "
        << std::right << std::setw(8) << std::fixed << std::setprecision(2)
        << time_ns << " ns  (~" << std::setw(5) << std::fixed << std::setprecision(1)
        << cycles << " cycles)";

    if (baseline_ns > 0.0) {
        double overhead = time_ns - baseline_ns;
        double overhead_cycles = ns_to_cycles(overhead);
        out << "  [overhead: +" << std::setw(5) << std::fixed << std::setprecision(1)
            << overhead_cycles << " cycles]";
    }

    out << "\n";
}

int main(int argc, char** argv) {
    std::cout << "=================================================================\n";
    std::cout << "Handle System Performance Benchmark\n";
    std::cout << "=================================================================\n";
    std::cout << "Architecture documentation: docs/content/Architecture/Handle.md\n";
    std::cout << "Iterations per test: " << ITERATIONS << "\n";
    std::cout << "=================================================================\n\n";

    // Run benchmarks
    std::cout << "Running benchmarks...\n\n";

    double baseline = benchmark_direct_pointer_access();
    double get_type_time = benchmark_handle_get_type();
    double get_view_time = benchmark_handle_get_view();
    double full_chain_time = benchmark_handle_full_chain();
    double retain_time = benchmark_handle_retain();
    double create_int_time = benchmark_handle_creation_int();
    double create_float_time = benchmark_handle_creation_float();
    double create_string_time = benchmark_handle_creation_string();

    // Print results to console
    std::cout << "BASELINE:\n";
    print_results(std::cout, "Direct pointer dereference", baseline);
    std::cout << "\n";

    std::cout << "HANDLE ACCESS OPERATIONS:\n";
    print_results(std::cout, "get_type() call", get_type_time, baseline);
    print_results(std::cout, "get_view() call", get_view_time, baseline);
    print_results(std::cout, "Full chain (get_type + get_view)", full_chain_time, baseline);
    std::cout << "\n";

    std::cout << "REFERENCE COUNTING:\n";
    print_results(std::cout, "Handle copy (retain)", retain_time, baseline);
    std::cout << "\n";

    std::cout << "HANDLE CREATION:\n";
    print_results(std::cout, "Create Int handle", create_int_time, baseline);
    print_results(std::cout, "Create Float handle", create_float_time, baseline);
    print_results(std::cout, "Create String handle", create_string_time, baseline);
    std::cout << "\n";

    // Write detailed report to file
    std::string report_filename = "handle_performance_report.txt";
    if (argc > 1) {
        report_filename = argv[1];
    }

    std::ofstream report(report_filename);
    if (!report.is_open()) {
        std::cerr << "Failed to open report file: " << report_filename << "\n";
        return 1;
    }

    report << "=================================================================\n";
    report << "Handle System Performance Benchmark Report\n";
    report << "=================================================================\n";
    report << "Generated: " << __DATE__ << " " << __TIME__ << "\n";
    report << "Architecture: docs/content/Architecture/Handle.md\n";
    report << "Iterations: " << ITERATIONS << "\n";
    report << "=================================================================\n\n";

    report << "MEASURED PERFORMANCE:\n\n";

    report << "BASELINE:\n";
    print_results(report, "Direct pointer dereference", baseline);
    report << "\n";

    report << "HANDLE ACCESS OPERATIONS:\n";
    print_results(report, "get_type() call", get_type_time, baseline);
    print_results(report, "get_view() call", get_view_time, baseline);
    print_results(report, "Full chain (get_type + get_view)", full_chain_time, baseline);
    report << "\n";

    report << "REFERENCE COUNTING:\n";
    print_results(report, "Handle copy (retain)", retain_time, baseline);
    report << "\n";

    report << "HANDLE CREATION:\n";
    print_results(report, "Create Int handle", create_int_time, baseline);
    print_results(report, "Create Float handle", create_float_time, baseline);
    print_results(report, "Create String handle", create_string_time, baseline);
    report << "\n";

    report << "=================================================================\n";
    report << "COMPARISON WITH DOCUMENTED EXPECTATIONS:\n";
    report << "=================================================================\n\n";

    report << "From Handle.md (lines 327-349):\n";
    report << "Expected handle.get_view() call chain:\n";
    report << "  1. get_type() → get_manager().type(data): ~10-15 cycles\n";
    report << "  2. get_manager() (redundant call):        ~5-10 cycles\n";
    report << "  3. manager.get_view(data):                ~20-50 cycles\n";
    report << "  4. std::get<Type*>() extraction:          ~2-5 cycles\n";
    report << "  Expected total:                           ~37-80 cycles\n\n";

    report << "Actual measurements:\n";
    report << "  get_type() measured:                      ~" << std::fixed << std::setprecision(1)
           << ns_to_cycles(get_type_time) << " cycles\n";
    report << "  get_view() measured:                      ~" << std::fixed << std::setprecision(1)
           << ns_to_cycles(get_view_time) << " cycles\n";
    report << "  Full chain measured:                      ~" << std::fixed << std::setprecision(1)
           << ns_to_cycles(full_chain_time) << " cycles\n\n";

    report << "Analysis:\n";
    double expected_min = 37.0;
    double expected_max = 80.0;
    double actual = ns_to_cycles(full_chain_time);

    if (actual >= expected_min && actual <= expected_max) {
        report << "  ✓ Measured performance is within expected range\n";
    } else if (actual < expected_min) {
        report << "  ⚠ Measured performance is BETTER than expected\n";
        report << "    Possible reasons: CPU optimization, caching, compiler optimizations\n";
    } else {
        report << "  ⚠ Measured performance is WORSE than expected\n";
        report << "    Possible reasons: CPU contention, cache misses, system load\n";
    }

    report << "\n=================================================================\n";
    report << "END OF REPORT\n";
    report << "=================================================================\n";

    report.close();

    std::cout << "Report written to: " << report_filename << "\n";
    std::cout << "\nBenchmark completed successfully.\n";

    return 0;
}
