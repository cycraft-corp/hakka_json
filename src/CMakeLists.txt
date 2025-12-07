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

# Set visibility presets for static library (symbols hidden by default)
set_target_properties(hakka_json_core PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

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
if(HAKKA_ICU_USING_FETCH)
    # When using ExternalProject, add explicit dependencies and includes
    add_dependencies(hakka_json_core icu_project_libs)
    target_include_directories(hakka_json_core PRIVATE ${ICU_INCLUDE_DIR})
endif()

# Link ICU libraries (works for both system and ExternalProject)
target_link_libraries(hakka_json_core PRIVATE ${HAKKA_ICU_LIBRARIES})

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
