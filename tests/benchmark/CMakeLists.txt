# Benchmark suite for JSON libraries memory comparison

# Enable IPO/LTO if available
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
if(IPO_SUPPORTED)
    message(STATUS "IPO/LTO enabled for benchmarks")
else()
    message(STATUS "IPO/LTO not supported: ${IPO_ERROR}")
endif()

# Set optimization flags for benchmarks
set(BENCHMARK_COMPILE_OPTIONS
    $<$<CXX_COMPILER_ID:GNU,Clang>:-O3 -march=native -mtune=native -flto>
    $<$<CXX_COMPILER_ID:MSVC>:/O2 /GL>
)

set(BENCHMARK_LINK_OPTIONS
    $<$<CXX_COMPILER_ID:GNU,Clang>:-flto>
    $<$<CXX_COMPILER_ID:MSVC>:/LTCG>
)

# Hakka JSON benchmark
add_executable(benchmark_hakka hakka/hakka.cpp)
target_link_libraries(benchmark_hakka PRIVATE hakka_json_core)
target_include_directories(benchmark_hakka PRIVATE
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_BINARY_DIR}/_deps/expected-src/include/
)
target_compile_options(benchmark_hakka PRIVATE ${BENCHMARK_COMPILE_OPTIONS})
target_link_options(benchmark_hakka PRIVATE ${BENCHMARK_LINK_OPTIONS})
if(IPO_SUPPORTED)
    set_target_properties(benchmark_hakka PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# nlohmann/json benchmark
find_package(nlohmann_json 3.12 QUIET)

if(NOT nlohmann_json_FOUND)
    # Try FetchContent if find_package failed
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
        GIT_SHALLOW TRUE
    )

    # Check if already populated by parent project
    FetchContent_GetProperties(nlohmann_json)
    if(NOT nlohmann_json_POPULATED)
        FetchContent_MakeAvailable(nlohmann_json)
    endif()

    set(nlohmann_json_FOUND TRUE)
endif()

if(nlohmann_json_FOUND)
    add_executable(benchmark_nlohmann nlohmann/nlohmann.cpp)
    target_link_libraries(benchmark_nlohmann PRIVATE nlohmann_json::nlohmann_json)
    target_compile_features(benchmark_nlohmann PRIVATE cxx_std_17)
    target_compile_options(benchmark_nlohmann PRIVATE ${BENCHMARK_COMPILE_OPTIONS})
    target_link_options(benchmark_nlohmann PRIVATE ${BENCHMARK_LINK_OPTIONS})
    if(IPO_SUPPORTED)
        set_target_properties(benchmark_nlohmann PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    message(STATUS "nlohmann/json benchmark enabled")
else()
    message(STATUS "nlohmann/json not found, benchmark disabled")
endif()

# jansson benchmark
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(JANSSON jansson>=2.14)
    if(JANSSON_FOUND)
        add_executable(benchmark_jansson jansson/jansson.c)
        target_include_directories(benchmark_jansson PRIVATE ${JANSSON_INCLUDE_DIRS})
        target_link_libraries(benchmark_jansson PRIVATE ${JANSSON_LIBRARIES})
        target_compile_options(benchmark_jansson PRIVATE
            $<$<C_COMPILER_ID:GNU,Clang>:-O3 -march=native -mtune=native -flto>
            $<$<C_COMPILER_ID:MSVC>:/O2 /GL>
        )
        target_link_options(benchmark_jansson PRIVATE ${BENCHMARK_LINK_OPTIONS})
        if(IPO_SUPPORTED)
            set_target_properties(benchmark_jansson PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
        endif()
        message(STATUS "jansson benchmark enabled")
    else()
        message(STATUS "jansson not found, benchmark disabled")
    endif()
endif()

# C++ std benchmark
add_executable(benchmark_cppstd cppstd/cppstd.cpp)
target_compile_features(benchmark_cppstd PRIVATE cxx_std_17)
target_compile_options(benchmark_cppstd PRIVATE ${BENCHMARK_COMPILE_OPTIONS})
target_link_options(benchmark_cppstd PRIVATE ${BENCHMARK_LINK_OPTIONS})
if(IPO_SUPPORTED)
    set_target_properties(benchmark_cppstd PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Boost.JSON benchmark
find_package(Boost 1.88 QUIET COMPONENTS json CONFIG)

if(Boost_FOUND)
    add_executable(benchmark_boost boost/boost_json.cpp)
    target_link_libraries(benchmark_boost PRIVATE Boost::json)
    target_compile_features(benchmark_boost PRIVATE cxx_std_17)
    target_compile_options(benchmark_boost PRIVATE ${BENCHMARK_COMPILE_OPTIONS})
    target_link_options(benchmark_boost PRIVATE ${BENCHMARK_LINK_OPTIONS})
    if(IPO_SUPPORTED)
        set_target_properties(benchmark_boost PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    message(STATUS "Boost.JSON benchmark enabled")
else()
    message(STATUS "Boost.JSON not found, benchmark disabled")
endif()

# Python benchmark (uv-based wrappers)
find_program(UV_EXECUTABLE uv)
if(UV_EXECUTABLE)
    # Generate Python 3.10 wrapper script
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/benchmark_python310
"#!/usr/bin/env bash
exec ${UV_EXECUTABLE} run --python 3.10 ${CMAKE_CURRENT_SOURCE_DIR}/python/python_bench.py \"\$@\"
")
    file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/benchmark_python310
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

    # Generate Python 3.13 wrapper script
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/benchmark_python313
"#!/usr/bin/env bash
exec ${UV_EXECUTABLE} run --python 3.13 ${CMAKE_CURRENT_SOURCE_DIR}/python/python_bench.py \"\$@\"
")
    file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/benchmark_python313
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE)

    message(STATUS "Python benchmarks enabled (via uv)")
else()
    message(STATUS "uv not found, Python benchmarks disabled")
endif()

# Golang benchmark
find_program(GO_EXECUTABLE go)
if(GO_EXECUTABLE)
    add_custom_target(benchmark_golang
        COMMAND ${GO_EXECUTABLE} build -ldflags "-s -w" -gcflags "-l=4" -o ${CMAKE_CURRENT_BINARY_DIR}/benchmark_golang main.go
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/golang
        COMMENT "Building Golang benchmark with optimizations"
    )
    message(STATUS "Golang benchmark enabled")
else()
    message(STATUS "Go not found, benchmark disabled")
endif()

# Rust benchmark
find_program(CARGO_EXECUTABLE cargo)
if(CARGO_EXECUTABLE)
    add_custom_target(benchmark_rust
        COMMAND ${CARGO_EXECUTABLE} build --release --target-dir ${CMAKE_CURRENT_BINARY_DIR}/rust
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_CURRENT_BINARY_DIR}/rust/release/rust_json_benchmark
            ${CMAKE_CURRENT_BINARY_DIR}/benchmark_rust
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rust
        COMMENT "Building Rust benchmark"
    )
    message(STATUS "Rust benchmark enabled")
else()
    message(STATUS "Cargo not found, benchmark disabled")
endif()

# Master benchmark target that builds all available benchmarks
add_custom_target(benchmarks
    DEPENDS benchmark_hakka benchmark_cppstd
    COMMENT "Building all available benchmarks"
)

# Add optional dependencies if they exist
if(nlohmann_json_FOUND)
    add_dependencies(benchmarks benchmark_nlohmann)
endif()

if(JANSSON_FOUND)
    add_dependencies(benchmarks benchmark_jansson)
endif()

if(Boost_FOUND)
    add_dependencies(benchmarks benchmark_boost)
endif()

if(GO_EXECUTABLE)
    add_dependencies(benchmarks benchmark_golang)
endif()

if(CARGO_EXECUTABLE)
    add_dependencies(benchmarks benchmark_rust)
endif()
