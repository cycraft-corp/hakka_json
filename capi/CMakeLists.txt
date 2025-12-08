# C API library sources (explicit list)
add_library(hakka_json SHARED
    hakka_json_array.cpp
    hakka_json_object.cpp
    hakka_json_primitive.cpp
)

# Note: We do NOT set CXX_VISIBILITY_PRESET here because it would hide all symbols by default,
# including the C API functions. Instead, we rely on version scripts (Linux/macOS) and
# selective export definitions (Windows) to control which symbols are exported.

# Symbol visibility settings for shared library
if(WIN32)
    # Windows: Use export header for selective symbol export
    target_compile_definitions(hakka_json PRIVATE HAKKA_JSON_BUILDING_DLL)
    # Note: WINDOWS_EXPORT_ALL_SYMBOLS is intentionally NOT used for controlled exports
else()
    # Linux: Use version script to export only Hakka* symbols (C API)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set_target_properties(hakka_json PROPERTIES
            LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/cmake/exports_linux.map"
        )
    endif()
    
    # macOS: Use exported symbols list
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set_target_properties(hakka_json PROPERTIES
            LINK_FLAGS "-Wl,-exported_symbols_list,${CMAKE_SOURCE_DIR}/cmake/exports_macos.exp"
        )
    endif()
endif()

# Include directories
target_include_directories(hakka_json
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link to core library (brings all transitive dependencies)
target_link_libraries(hakka_json
    PRIVATE
        hakka_json::core
        hakka_json::compiler_options
)

# Link coverage options if enabled
if(HAKKA_JSON_ENABLE_COVERAGE)
    target_link_libraries(hakka_json PRIVATE hakka_json::coverage_options)
endif()

# ICU dependencies
# Link ICU libraries (works for both system and ExternalProject)
target_link_libraries(hakka_json PRIVATE ${HAKKA_ICU_LIBRARIES})

if(HAKKA_ICU_USING_FETCH)
    # When using ExternalProject, add explicit dependencies and includes
    add_dependencies(hakka_json icu_project_libs)
    target_include_directories(hakka_json PRIVATE ${ICU_INCLUDE_DIR})
    # Unix: ICU built with --disable-renaming, so we define HAKKA_ICU_DISABLE_RENAMING
    # Windows: MSBuild ICU uses versioned symbols (icu_XX), ICU headers create namespace alias automatically
    if(NOT WIN32)
        target_compile_definitions(hakka_json PRIVATE HAKKA_ICU_DISABLE_RENAMING)
    endif()
    
    # Copy ICU DLLs to output directory on Windows
    if(WIN32)
        add_custom_command(TARGET hakka_json POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${ICU_INSTALL_DIR}/bin"
                $<TARGET_FILE_DIR:hakka_json>
            COMMENT "Copied all ICU DLLs to capi output directory"
            VERBATIM
        )
    endif()
elseif(HAKKA_ICU_USING_SYSTEM)
    # When using system ICU, include directories should come from ICU:: targets
    if(ICU_INCLUDE_DIRS)
        target_include_directories(hakka_json PRIVATE ${ICU_INCLUDE_DIRS})
    endif()
    
    # If system ICU has renaming, do NOT define U_DISABLE_RENAMING
    # If system ICU doesn't have renaming, define U_DISABLE_RENAMING
    if(NOT HAKKA_ICU_HAS_RENAMING)
        target_compile_definitions(hakka_json PRIVATE HAKKA_ICU_DISABLE_RENAMING)
    endif()
endif()

# GCC needs atomic
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(hakka_json PRIVATE atomic)
endif()

# Namespace alias
add_library(hakka_json::capi ALIAS hakka_json)

# Copy to release directory (keep old behavior)
# Use copy instead of copy_if_different to avoid potential issues in CI
add_custom_command(TARGET hakka_json POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/target/release
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:hakka_json> ${CMAKE_SOURCE_DIR}/target/release/
    COMMENT "Copying hakka_json DLL to target/release directory"
    VERBATIM
)
