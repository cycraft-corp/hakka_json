# ICU Dependency Resolution
# Handles ICU resolution via find_package() with fallback to ExternalProject
#
# IMPORTANT: Static linking third-party libraries is a major principle in this repository.
# ICU must be built with --disable-renaming for static linking to work correctly.
# This module detects whether system ICU has renaming enabled and adapts accordingly.
#
# Exports:
#   HAKKA_ICU_USING_SYSTEM - TRUE if using system ICU, FALSE otherwise
#   HAKKA_ICU_USING_FETCH - TRUE if using ExternalProject ICU, FALSE otherwise
#   HAKKA_ICU_LIBRARIES - Ordered list of ICU:: targets (ICU::tu ICU::i18n ICU::io ICU::uc ICU::data)
#   HAKKA_ICU_HAS_RENAMING - TRUE if ICU has symbol renaming enabled, FALSE otherwise

# Initialize variables
set(HAKKA_ICU_USING_SYSTEM FALSE CACHE INTERNAL "Using system ICU")
set(HAKKA_ICU_USING_FETCH FALSE CACHE INTERNAL "Using ExternalProject ICU")
set(HAKKA_ICU_LIBRARIES "" CACHE INTERNAL "Ordered ICU libraries")
set(HAKKA_ICU_HAS_RENAMING FALSE CACHE INTERNAL "ICU has symbol renaming enabled")

# Function to detect if ICU has symbol renaming enabled
function(_detect_icu_renaming)
    # Try to compile a small test program that checks for U_ICU_VERSION_SUFFIX
    set(TEST_SOURCE "${CMAKE_BINARY_DIR}/test_icu_renaming.cpp")
    file(WRITE "${TEST_SOURCE}" "
#include <unicode/uvernum.h>
int main() {
#ifdef U_ICU_VERSION_SUFFIX
    return 1; // Has renaming
#else
    return 0; // No renaming
#endif
}
")
    
    try_run(
        RUN_RESULT COMPILE_RESULT
        "${CMAKE_BINARY_DIR}"
        "${TEST_SOURCE}"
        CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${ICU_INCLUDE_DIRS}"
        COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT
        RUN_OUTPUT_VARIABLE RUN_OUTPUT
    )
    
    if(COMPILE_RESULT AND RUN_RESULT EQUAL 1)
        set(HAKKA_ICU_HAS_RENAMING TRUE CACHE INTERNAL "ICU has symbol renaming enabled")
        message(STATUS "ICU: Detected symbol renaming enabled")
    else()
        set(HAKKA_ICU_HAS_RENAMING FALSE CACHE INTERNAL "ICU has symbol renaming enabled")
        message(STATUS "ICU: Detected symbol renaming disabled")
    endif()
endfunction()

# Determine which resolution path to use
set(_use_find_package FALSE)
set(_use_external_project FALSE)

if(HAKKA_JSON_FORCE_FETCH_CONTENT)
    message(STATUS "ICU: FORCE_FETCH_CONTENT enabled, using ExternalProject")
    set(_use_external_project TRUE)
elseif(HAKKA_JSON_FORCE_SYSTEM_DEPS)
    message(STATUS "ICU: FORCE_SYSTEM_DEPS enabled, requiring system package")
    set(_use_find_package TRUE)
    set(_require_system TRUE)
elseif(HAKKA_JSON_USE_SYSTEM_DEPS)
    # Try system package first
    message(STATUS "ICU: Trying system package first")
    set(_use_find_package TRUE)
    set(_require_system FALSE)
else()
    message(STATUS "ICU: USE_SYSTEM_DEPS disabled, using ExternalProject")
    set(_use_external_project TRUE)
endif()

# Try find_package if requested
if(_use_find_package)
    if(_require_system)
        find_package(ICU COMPONENTS uc i18n io data REQUIRED)
    else()
        find_package(ICU COMPONENTS uc i18n io data QUIET)
    endif()
    
    if(ICU_FOUND)
        message(STATUS "ICU: Found system installation (version ${ICU_VERSION})")
        
        # Detect if ICU has symbol renaming enabled
        _detect_icu_renaming()
        
        if(HAKKA_ICU_HAS_RENAMING)
            message(STATUS "ICU: System ICU has symbol renaming enabled - will adapt code accordingly")
        else()
            message(STATUS "ICU: System ICU has symbol renaming disabled - perfect for static linking")
        endif()
        
        set(HAKKA_ICU_USING_SYSTEM TRUE CACHE INTERNAL "Using system ICU")
        
        # Create ICU:: aliases if they don't exist
        # FindICU module may create targets like ICU::uc, ICU::i18n, etc.
        # We ensure consistent naming
        if(NOT TARGET ICU::uc AND TARGET ICU::uc)
            # Already has ICU:: prefix, nothing to do
        elseif(ICU_UC_LIBRARY)
            # Create imported target if it doesn't exist
            if(NOT TARGET ICU::uc)
                add_library(ICU::uc UNKNOWN IMPORTED)
                set_target_properties(ICU::uc PROPERTIES
                    IMPORTED_LOCATION "${ICU_UC_LIBRARY}"
                    INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIRS}"
                )
            endif()
        endif()
        
        if(NOT TARGET ICU::i18n)
            if(ICU_I18N_LIBRARY)
                add_library(ICU::i18n UNKNOWN IMPORTED)
                set_target_properties(ICU::i18n PROPERTIES
                    IMPORTED_LOCATION "${ICU_I18N_LIBRARY}"
                    INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIRS}"
                )
            endif()
        endif()
        
        if(NOT TARGET ICU::io)
            if(ICU_IO_LIBRARY)
                add_library(ICU::io UNKNOWN IMPORTED)
                set_target_properties(ICU::io PROPERTIES
                    IMPORTED_LOCATION "${ICU_IO_LIBRARY}"
                    INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIRS}"
                )
            endif()
        endif()
        
        if(NOT TARGET ICU::data)
            if(ICU_DATA_LIBRARY)
                add_library(ICU::data UNKNOWN IMPORTED)
                set_target_properties(ICU::data PROPERTIES
                    IMPORTED_LOCATION "${ICU_DATA_LIBRARY}"
                    INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIRS}"
                )
            endif()
        endif()
        
        # Handle ICU::tu (tools/utilities) - may not be available in all distributions
        if(NOT TARGET ICU::tu)
            # Try to find the tu library
            find_library(ICU_TU_LIBRARY
                NAMES icutu libicutu
                HINTS ${ICU_LIBRARY_DIRS}
                PATH_SUFFIXES lib lib64
            )
            
            if(ICU_TU_LIBRARY)
                message(STATUS "ICU: Found tu component: ${ICU_TU_LIBRARY}")
                add_library(ICU::tu UNKNOWN IMPORTED)
                set_target_properties(ICU::tu PROPERTIES
                    IMPORTED_LOCATION "${ICU_TU_LIBRARY}"
                    INTERFACE_INCLUDE_DIRECTORIES "${ICU_INCLUDE_DIRS}"
                )
            else()
                message(WARNING "ICU: tu component not found in system installation. Some functionality may be limited.")
                # Create a dummy interface target to avoid link errors
                add_library(ICU::tu INTERFACE IMPORTED)
            endif()
        endif()
        
        # Set ordered library list
        set(HAKKA_ICU_LIBRARIES ICU::tu ICU::i18n ICU::io ICU::uc ICU::data CACHE INTERNAL "Ordered ICU libraries")
    else()
        # find_package failed, use ExternalProject if not required
        if(_require_system)
            message(FATAL_ERROR "ICU: System package required but not found")
        else()
            message(STATUS "ICU: System package not found, falling back to ExternalProject")
            set(_use_external_project TRUE)
        endif()
    endif()
endif()

# Use ExternalProject if requested or as fallback
if(_use_external_project)
    set(HAKKA_ICU_USING_FETCH TRUE CACHE INTERNAL "Using ExternalProject ICU")
    
    # Wrap the old ICU.cmake in a function to handle PARENT_SCOPE
    function(_setup_icu_external)
        set(ICU_VERSION "" CACHE STRING "ICU version")
        set(ICU_ROOT "" CACHE PATH "ICU installation path")
        include(${CMAKE_CURRENT_LIST_DIR}/ICU.cmake)
        
        # Export to parent (ICUDependency.cmake scope)
        set(ICU_INCLUDE_DIR ${ICU_INCLUDE_DIR} PARENT_SCOPE)
        set(ICU_LIB_DIR ${ICU_LIB_DIR} PARENT_SCOPE)
        set(ICU_LIBRARIES ${ICU_LIBRARIES} PARENT_SCOPE)
        set(ICU_SHARED_LIBRARIES ${ICU_SHARED_LIBRARIES} PARENT_SCOPE)
        set(ICU_INSTALL_DIR ${ICU_INSTALL_DIR} PARENT_SCOPE)
    endfunction()
    
    _setup_icu_external()
    
    # ICU.cmake creates targets like icutu, icui18n, icuio, icuuc, icudata (Unix)
    # or icutu, icuin, icuio, icuuc, icudt (Windows)
    # We need to create ICU:: aliases for consistency
    
    if(WIN32)
        # Windows: icuin, icuuc, icudt, icuio, icutu
        if(TARGET icutu AND NOT TARGET ICU::tu)
            add_library(ICU::tu ALIAS icutu)
        endif()
        if(TARGET icuin AND NOT TARGET ICU::i18n)
            add_library(ICU::i18n ALIAS icuin)
        endif()
        if(TARGET icuio AND NOT TARGET ICU::io)
            add_library(ICU::io ALIAS icuio)
        endif()
        if(TARGET icuuc AND NOT TARGET ICU::uc)
            add_library(ICU::uc ALIAS icuuc)
        endif()
        if(TARGET icudt AND NOT TARGET ICU::data)
            add_library(ICU::data ALIAS icudt)
        endif()
        
        # Set ordered library list for Windows
        set(HAKKA_ICU_LIBRARIES ICU::tu ICU::i18n ICU::io ICU::uc ICU::data CACHE INTERNAL "Ordered ICU libraries")
    else()
        # Unix: icutu, icui18n, icuio, icuuc, icudata
        if(TARGET icutu AND NOT TARGET ICU::tu)
            add_library(ICU::tu ALIAS icutu)
        endif()
        if(TARGET icui18n AND NOT TARGET ICU::i18n)
            add_library(ICU::i18n ALIAS icui18n)
        endif()
        if(TARGET icuio AND NOT TARGET ICU::io)
            add_library(ICU::io ALIAS icuio)
        endif()
        if(TARGET icuuc AND NOT TARGET ICU::uc)
            add_library(ICU::uc ALIAS icuuc)
        endif()
        if(TARGET icudata AND NOT TARGET ICU::data)
            add_library(ICU::data ALIAS icudata)
        endif()
        
        # Set ordered library list for Unix
        set(HAKKA_ICU_LIBRARIES ICU::tu ICU::i18n ICU::io ICU::uc ICU::data CACHE INTERNAL "Ordered ICU libraries")
    endif()
endif()

# Report status
message(STATUS "ICU resolution: SYSTEM=${HAKKA_ICU_USING_SYSTEM}, FETCH=${HAKKA_ICU_USING_FETCH}")
message(STATUS "ICU libraries: ${HAKKA_ICU_LIBRARIES}")
