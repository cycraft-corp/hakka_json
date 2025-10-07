# Performance tests for Handle System

# Performance benchmark executable
add_executable(handle_perf_benchmark
    handle_performance_benchmark.cpp
)

target_link_libraries(handle_perf_benchmark
    PRIVATE
        hakka_json::core
        hakka_json::compiler_options
)

target_include_directories(handle_perf_benchmark
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include/
        ${CMAKE_BINARY_DIR}/_deps/expected-src/include/
)

# Windows: Copy ICU DLLs to benchmark executable directory
if(WIN32)
    add_custom_command(TARGET handle_perf_benchmark POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ICU_INSTALL_DIR}/bin"
            $<TARGET_FILE_DIR:handle_perf_benchmark>
        COMMENT "Copied all ICU DLLs to benchmark directory"
        VERBATIM
    )

    # Copy ASAN runtime DLL if enabled
    if(HAKKA_JSON_ENABLE_SANITIZER_ADDRESS AND ASAN_RUNTIME_DLL)
        add_custom_command(TARGET handle_perf_benchmark POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ASAN_RUNTIME_DLL}"
                $<TARGET_FILE_DIR:handle_perf_benchmark>
            COMMENT "Copying ASAN runtime DLL to benchmark directory"
            VERBATIM
        )
    endif()
endif()

# Enable optimizations for performance testing
# Use -O2 instead of -O3 to get more realistic production-like performance
target_compile_options(handle_perf_benchmark PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-O2>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-march=native>
    $<$<CXX_COMPILER_ID:MSVC>:/O2>
)

# Install the benchmark
install(TARGETS handle_perf_benchmark
    RUNTIME DESTINATION bin/perf
)

# Add custom target to run the benchmark
add_custom_target(run_perf_benchmark
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/handle_perf_benchmark
    DEPENDS handle_perf_benchmark
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running handle performance benchmark..."
)

# Add custom target to run with perf stat (Linux only)
if(UNIX AND NOT APPLE)
    add_custom_target(run_perf_stat
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/run_perf_analysis.sh
                ${CMAKE_CURRENT_BINARY_DIR}/handle_perf_benchmark
                ${CMAKE_CURRENT_BINARY_DIR}/handle_performance_report.txt
        DEPENDS handle_perf_benchmark
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running handle performance analysis with perf..."
    )
endif()
