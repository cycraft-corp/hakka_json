# C API library sources (explicit list)
add_library(hakka_json SHARED
    hakka_json_array.cpp
    hakka_json_object.cpp
    hakka_json_primitive.cpp
)

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

# ICU dependencies (explicit, same as core)
add_dependencies(hakka_json icu_project_libs)

if(WIN32)
    target_link_libraries(hakka_json PRIVATE
        icutu icuin icuio icuuc icudt
    )
else()
    target_link_libraries(hakka_json PRIVATE
        icutu icui18n icuio icuuc icudata
    )
endif()

# GCC needs atomic
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(hakka_json PRIVATE atomic)
endif()

# Namespace alias
add_library(hakka_json::capi ALIAS hakka_json)

# Copy to release directory (keep old behavior)
add_custom_command(TARGET hakka_json POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/target/release
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hakka_json> ${CMAKE_SOURCE_DIR}/target/release/
)
