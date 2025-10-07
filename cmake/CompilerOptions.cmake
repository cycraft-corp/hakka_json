# Compiler options interface target
add_library(hakka_json_compiler_options INTERFACE)
add_library(hakka_json::compiler_options ALIAS hakka_json_compiler_options)

# Colored diagnostics
target_compile_options(hakka_json_compiler_options INTERFACE
    $<$<CXX_COMPILER_ID:GNU>:-fdiagnostics-color=always>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fcolor-diagnostics>
)

# Warning flags
target_compile_options(hakka_json_compiler_options INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
        -Wall -Wextra -Wpedantic -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /wd4702>
)

# MSVC specific defines
target_compile_definitions(hakka_json_compiler_options INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS _CTYPE_DISABLE_MACROS NOMINMAX UNICODE _UNICODE _C_ALWAYS_NO_IS>
)


# MSVC source file encoding
target_compile_options(hakka_json_compiler_options INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)

# Debug: sanitizers
# GCC/Clang: Always enabled in Debug mode
target_compile_options(hakka_json_compiler_options INTERFACE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-g -fsanitize=address,undefined,leak>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:Clang>>:-g -fsanitize=address,undefined,leak>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:AppleClang>>:-g>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:MSVC>>:/Zi /Od>
)

target_link_options(hakka_json_compiler_options INTERFACE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-fsanitize=address,undefined,leak>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=address,undefined,leak>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:MSVC>>:/INCREMENTAL:NO /DEBUG:FULL>
)

# MSVC: Address Sanitizer (opt-in via HAKKA_JSON_ENABLE_SANITIZER_ADDRESS)
if(HAKKA_JSON_ENABLE_SANITIZER_ADDRESS AND MSVC)
    target_compile_options(hakka_json_compiler_options INTERFACE /fsanitize=address)
    target_link_options(hakka_json_compiler_options INTERFACE /INCREMENTAL:NO)

    # Locate ASAN runtime DLL for Windows testing
    get_filename_component(COMPILER_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    find_file(ASAN_RUNTIME_DLL
        NAMES clang_rt.asan_dynamic-x86_64.dll
        PATHS "${COMPILER_DIR}"
        NO_DEFAULT_PATH
    )
    if(ASAN_RUNTIME_DLL)
        set(ASAN_RUNTIME_DLL "${ASAN_RUNTIME_DLL}" CACHE FILEPATH "ASAN runtime DLL path" FORCE)
        message(STATUS "Found ASAN runtime: ${ASAN_RUNTIME_DLL}")
    else()
        message(WARNING "ASAN runtime DLL not found. Tests may fail to run.")
    endif()
endif()

# Release: optimization
target_compile_options(hakka_json_compiler_options INTERFACE
    $<$<AND:$<CONFIG:Release>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>>:-O3>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/O2>
)

target_compile_definitions(hakka_json_compiler_options INTERFACE
    $<$<CONFIG:Release>:NDEBUG>
)

# Optional benchmarks
option(HAKKA_JSON_ENABLE_BENCHMARKS "Enable memory benchmarks for various JSON libraries" OFF)

# Optional profiling
option(HAKKA_JSON_ENABLE_PROFILING "Enable profiling" OFF)
if(HAKKA_JSON_ENABLE_PROFILING)
    target_compile_options(hakka_json_compiler_options INTERFACE
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-pg>
    )
    target_link_options(hakka_json_compiler_options INTERFACE
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-pg>
    )
endif()

# Coverage options (gcov instrumentation)
if(HAKKA_JSON_ENABLE_COVERAGE)
    add_library(hakka_json_coverage_options INTERFACE)
    add_library(hakka_json::coverage_options ALIAS hakka_json_coverage_options)

    target_compile_options(hakka_json_coverage_options INTERFACE
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:--coverage>
    )

    target_link_options(hakka_json_coverage_options INTERFACE
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:--coverage>
    )
endif()
