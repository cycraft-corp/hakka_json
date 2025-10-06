# How to Build

## Installation via Conan (Recommended)

The easiest way to use Hakka JSON in your project is via Conan package manager:

### Prerequisites

- Conan 2.0 or later
- C++23 compliant compiler (GCC ≥11, Clang ≥15, MSVC ≥193, AppleClang ≥14)
- CMake 3.20 or later

### Using in Your Project

Add Hakka JSON to your `conanfile.txt`:

```ini
[requires]
hakka_json/1.0.0

[generators]
CMakeDeps
CMakeToolchain
```

Or in `conanfile.py`:

```python
def requirements(self):
    self.requires("hakka_json/1.0.0")
```

Then in your `CMakeLists.txt`:

```cmake
find_package(HakkaJson REQUIRED)
target_link_libraries(your_target PRIVATE HakkaJson::core)
```

Install dependencies and build:

```bash
conan install . --build=missing -s compiler.cppstd=23
cmake --preset conan-default
cmake --build --preset conan-default
```

## Building from Source

### Dependencies

- CMake 3.20 or later is required (a network connection is needed to download dependencies).
    - Automatically downloaded dependencies:
        - Google Test
        - nlohmann/json
        - TartanLlama/expected
        - ICU (International Components for Unicode)
- A C++23 compliant compiler is required.
- [Ninja](https://ninja-build.org/) build system (optional, but recommended).
- [mold](https://github.com/rui314/mold) (optional, but recommended).

## Building the C++ Library

Hakka JSON builds two libraries by default:

### Linux build

- `libhakka_json_core.a` (static library)
- `libhakka_json.so` (shared library)

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_LINKER=$(which mold) -Wno-dev
ninja
```

### Windows build

- `hakka_json_core.lib` (static library)
- `hakka_json.dll` (shared library)
- ICU libraries (`.dll` and `.lib`)

```powershell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 19 2022" -A x64 -Wno-dev
cmake --build . --config Release --parallel
```

### macOS build

- `libhakka_json_core.a` (static library)
- `libhakka_json.dylib` (shared library)

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_LINKER=$(which mold) -Wno-dev
ninja
```

## Testing the C++ Library
It's similar to building the library, but with the `ENABLE_TESTS` flag and `debug` build type.

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -G Ninja -DCMAKE_LINKER=$(which mold) -DENABLE_TESTS=true -Wno-dev
ninja
ctest
```

Address sanitizer are default enabled for debug build type, except the `AppleClang` compiler.
It might have some compatibility issues with `AppleClang` compiler. If you encounter any issues, please disable the address sanitizer from the CMakelists.txt file.

Windows users can also enable the address sanitizer by using the `Debug` build type with `clang-cl`.


## Experimental: Building with Profile-Guided Optimization (PGO)
Profile-Guided Optimization (PGO) can significantly enhance the performance of your application by optimizing the build based on runtime profiling data. Below are the steps to build your project with PGO using Clang on Linux and macOS.

### 1. Instrument Build

First, compile the code with instrumentation to collect profiling data.

**Linux and macOS:**

```bash
mkdir build_pgo
cd build_pgo
cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DENABLE_TESTS=true -DCMAKE_CXX_FLAGS="-fprofile-instr-generate" -DCMAKE_LINKER=$(which mold) -Wno-dev
ninja
```

This configuration compiles the project with instrumentation enabled, preparing it to generate profiling data during execution.

### 2. Execute the Instrumented Binary

Run the instrumented application to generate profiling data. Ensure that the application is exercised with typical workloads to collect representative data.

```bash
ctest
```

This execution will produce profiling data files (e.g., `./tests/capi/default.profraw`, `./tests/core/default.profraw`) in the current directory.

### 3. Merge Profiling Data

After collecting the raw profiling data, merge it into a single profile data file using `llvm-profdata`.

```bash
llvm-profdata merge -output=merged.profdata *.profraw
```

This command consolidates the profiling information into `merged.profdata`, which will be used in the optimization phase.

### 4. Optimized Build Using Profile Data

Rebuild the project using the collected profile data to apply the optimizations.

**Linux and macOS:**

```bash
mkdir build_pgo_optimized
cd build_pgo_optimized
cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DLLVM_PROFDATA_FILE=../build_pgo/merged.profdata -DCMAKE_LINKER=$(which mold) -Wno-dev
ninja
```

This step compiles the project with the profiling data, enabling the compiler to optimize the code based on the collected execution patterns.

### 5. Run Optimized Binary
Now the project is built with Profile-Guided Optimization. Execute the optimized binary to observe the performance improvements.

