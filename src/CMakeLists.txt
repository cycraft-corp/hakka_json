# Core library sources (explicit list)
add_library(hakka_json_core STATIC
    hakka_compare.cpp
    hakka_json.cpp
    hakka_json_array.cpp
    hakka_json_float.cpp
    hakka_json_handle.cpp
    hakka_json_int.cpp
    hakka_json_object.cpp
    hakka_json_string.cpp
    handles/array_manager.cpp
    handles/object_manager.cpp
    handles/scalar_manager.cpp
    handles/string_manager.cpp
    json_deserializer.cpp
)

# Note: Visibility presets are not set on static library as they're only relevant for shared libraries.
# The C++ API from hakka_json_core is available to unit tests and other consumers.
# Symbol visibility is controlled at the shared library level (hakka_json C API).

# Include directories
target_include_directories(hakka_json_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Dependencies
target_link_libraries(hakka_json_core
    PUBLIC
        nlohmann_json::nlohmann_json
        tl::expected
    PRIVATE
        hakka_json::compiler_options
)

# Link coverage options if enabled
if(HAKKA_JSON_ENABLE_COVERAGE)
    target_link_libraries(hakka_json_core PRIVATE hakka_json::coverage_options)
endif()

# ICU dependencies
# Link ICU libraries (works for both system and ExternalProject)
target_link_libraries(hakka_json_core PRIVATE ${HAKKA_ICU_LIBRARIES})

if(HAKKA_ICU_USING_FETCH)
    # When using ExternalProject, add explicit dependencies and includes
    add_dependencies(hakka_json_core icu_project_libs)
    target_include_directories(hakka_json_core PRIVATE ${ICU_INCLUDE_DIR})
    # Unix: ICU built with --disable-renaming, so we define HAKKA_ICU_DISABLE_RENAMING
    # Windows: MSBuild ICU uses versioned symbols (icu_XX), ICU headers create namespace alias automatically
    if(NOT WIN32)
        target_compile_definitions(hakka_json_core PRIVATE HAKKA_ICU_DISABLE_RENAMING)
    endif()
elseif(HAKKA_ICU_USING_SYSTEM)
    # When using system ICU, include directories should come from ICU:: targets
    # but add explicitly to ensure they're available
    if(ICU_INCLUDE_DIRS)
        target_include_directories(hakka_json_core PRIVATE ${ICU_INCLUDE_DIRS})
    endif()
    
    # If system ICU has renaming, do NOT define U_DISABLE_RENAMING
    # If system ICU doesn't have renaming, define U_DISABLE_RENAMING
    if(NOT HAKKA_ICU_HAS_RENAMING)
        target_compile_definitions(hakka_json_core PRIVATE HAKKA_ICU_DISABLE_RENAMING)
    endif()
endif()

# GCC needs atomic
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(hakka_json_core PRIVATE atomic)
endif()

# TBB if enabled
if(HAKKA_JSON_ENABLE_TBB)
    target_link_libraries(hakka_json_core PRIVATE TBB::tbb)
    target_compile_definitions(hakka_json_core PRIVATE _ENABLE_STD_EXECUTION_POLICY)
endif()

# Namespace alias
add_library(hakka_json::core ALIAS hakka_json_core)

# Copy to release directory (keep old behavior)
add_custom_command(TARGET hakka_json_core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/target/release
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hakka_json_core> ${CMAKE_SOURCE_DIR}/target/release/
    COMMENT "Copying hakka_json_core library to target/release directory"
    VERBATIM
)
