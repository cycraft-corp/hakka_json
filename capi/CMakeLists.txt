# C API library sources (explicit list)
add_library(hakka_json SHARED
    hakka_json_array.cpp
    hakka_json_object.cpp
    hakka_json_primitive.cpp
)

# Windows: Export all symbols to generate import library
if(WIN32)
    set_target_properties(hakka_json PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
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

# ICU dependencies (explicit, same as core)
add_dependencies(hakka_json icu_project_libs)
target_include_directories(hakka_json PRIVATE ${ICU_INCLUDE_DIR})

if(WIN32)
    # Link ICU import libraries directly by full path to ensure they're found
    # Debug builds have 'd' suffix
    target_link_libraries(hakka_json PRIVATE
        "${ICU_LIB_DIR}/icuin$<$<CONFIG:Debug>:d>.lib"
        "${ICU_LIB_DIR}/icuuc$<$<CONFIG:Debug>:d>.lib"
        "${ICU_LIB_DIR}/icudt.lib"
        "${ICU_LIB_DIR}/icuio$<$<CONFIG:Debug>:d>.lib"
        "${ICU_LIB_DIR}/icutu$<$<CONFIG:Debug>:d>.lib"
    )
    # Copy ICU DLLs to output directory - separate commands for each file
    add_custom_command(TARGET hakka_json POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ICU_INSTALL_DIR}/bin/icuin$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:hakka_json>/icuin$<$<CONFIG:Debug>:d>.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ICU_INSTALL_DIR}/bin/icuuc$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:hakka_json>/icuuc$<$<CONFIG:Debug>:d>.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ICU_INSTALL_DIR}/bin/icudt.dll"
            "$<TARGET_FILE_DIR:hakka_json>/icudt.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ICU_INSTALL_DIR}/bin/icuio$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:hakka_json>/icuio$<$<CONFIG:Debug>:d>.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ICU_INSTALL_DIR}/bin/icutu$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:hakka_json>/icutu$<$<CONFIG:Debug>:d>.dll"
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
# Use copy instead of copy_if_different to avoid potential issues in CI
add_custom_command(TARGET hakka_json POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/target/release
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:hakka_json> ${CMAKE_SOURCE_DIR}/target/release/
    COMMENT "Copying hakka_json DLL to target/release directory"
    VERBATIM
)
