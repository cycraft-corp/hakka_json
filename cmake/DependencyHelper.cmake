# Helper macro to standardize dependency resolution logic
# Usage: resolve_dependency(<name> <find_package_args> <fetchcontent_args>)
#
# Arguments:
#   NAME - Package name
#   FIND_PACKAGE_ARGS - Arguments to pass to find_package() (version, components, etc.)
#   FETCHCONTENT_DECLARE_ARGS - Arguments to pass to FetchContent_Declare()
#
# Logic:
#   - If HAKKA_JSON_FORCE_FETCH_CONTENT=ON -> Always use FetchContent
#   - If HAKKA_JSON_FORCE_SYSTEM_DEPS=ON -> Use find_package(REQUIRED)
#   - If HAKKA_JSON_USE_SYSTEM_DEPS=ON -> Try find_package(QUIET), fallback to FetchContent
#   - Otherwise -> Use FetchContent directly

macro(resolve_dependency)
    set(options "")
    set(oneValueArgs NAME)
    set(multiValueArgs FIND_PACKAGE_ARGS FETCHCONTENT_DECLARE_ARGS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_NAME)
        message(FATAL_ERROR "resolve_dependency: NAME is required")
    endif()

    if(HAKKA_JSON_FORCE_FETCH_CONTENT)
        # Force FetchContent
        message(STATUS "Dependency ${ARG_NAME}: FORCE_FETCH_CONTENT enabled, using FetchContent")
        FetchContent_Declare(${ARG_NAME} ${ARG_FETCHCONTENT_DECLARE_ARGS})
        FetchContent_MakeAvailable(${ARG_NAME})
    elseif(HAKKA_JSON_FORCE_SYSTEM_DEPS)
        # Force system package (REQUIRED)
        message(STATUS "Dependency ${ARG_NAME}: FORCE_SYSTEM_DEPS enabled, requiring system package")
        find_package(${ARG_NAME} ${ARG_FIND_PACKAGE_ARGS} REQUIRED)
    elseif(HAKKA_JSON_USE_SYSTEM_DEPS)
        # Try system package first (QUIET), fallback to FetchContent
        find_package(${ARG_NAME} ${ARG_FIND_PACKAGE_ARGS} QUIET)
        if(NOT ${ARG_NAME}_FOUND)
            message(STATUS "Dependency ${ARG_NAME}: system package not found, using FetchContent")
            FetchContent_Declare(${ARG_NAME} ${ARG_FETCHCONTENT_DECLARE_ARGS})
            FetchContent_MakeAvailable(${ARG_NAME})
        else()
            message(STATUS "Dependency ${ARG_NAME}: using system package")
        endif()
    else()
        # Default to FetchContent when USE_SYSTEM_DEPS=OFF
        message(STATUS "Dependency ${ARG_NAME}: USE_SYSTEM_DEPS disabled, using FetchContent")
        FetchContent_Declare(${ARG_NAME} ${ARG_FETCHCONTENT_DECLARE_ARGS})
        FetchContent_MakeAvailable(${ARG_NAME})
    endif()
endmacro()
