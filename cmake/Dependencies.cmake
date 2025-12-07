include(FetchContent)

# CMake Options for dependency resolution
option(HAKKA_JSON_USE_SYSTEM_DEPS "Try system packages first (default path)" ON)
option(HAKKA_JSON_FORCE_FETCH_CONTENT "Always use FetchContent/ExternalProject (never system)" OFF)
option(HAKKA_JSON_FORCE_SYSTEM_DEPS "Always require system packages (never fetch)" OFF)

# Validation: prevent both FORCE options from being ON simultaneously
if(HAKKA_JSON_FORCE_FETCH_CONTENT AND HAKKA_JSON_FORCE_SYSTEM_DEPS)
    message(FATAL_ERROR "HAKKA_JSON_FORCE_FETCH_CONTENT and HAKKA_JSON_FORCE_SYSTEM_DEPS cannot both be ON")
endif()

# Include helper macro for dependency resolution
include(${CMAKE_CURRENT_LIST_DIR}/DependencyHelper.cmake)

# nlohmann_json
resolve_dependency(
    NAME nlohmann_json
    FIND_PACKAGE_ARGS 3.12
    FETCHCONTENT_DECLARE_ARGS
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)

# tl::expected
set(EXPECTED_BUILD_TESTS OFF)
resolve_dependency(
    NAME tl-expected
    FIND_PACKAGE_ARGS 1.1
    FETCHCONTENT_DECLARE_ARGS
        GIT_REPOSITORY https://github.com/TartanLlama/expected.git
        GIT_TAG v1.1.0
)
# Note: FetchContent names the target "expected" not "tl-expected"
# Create alias if using FetchContent and alias doesn't exist
if(NOT tl-expected_FOUND AND TARGET expected AND NOT TARGET tl::expected)
    add_library(tl::expected ALIAS expected)
endif()

# ICU - use new ICUDependency.cmake
include(${CMAKE_CURRENT_LIST_DIR}/ICUDependency.cmake)

# Sanitizer options (must be declared before GoogleTest to ensure consistent compilation)
option(HAKKA_JSON_ENABLE_SANITIZER_ADDRESS "Enable Address Sanitizer" OFF)
option(HAKKA_JSON_ENABLE_SANITIZER_UNDEFINED "Enable Undefined Behavior Sanitizer" OFF)
option(HAKKA_JSON_ENABLE_SANITIZER_THREAD "Enable Thread Sanitizer" OFF)
option(HAKKA_JSON_ENABLE_SANITIZER_MEMORY "Enable Memory Sanitizer (Clang only)" OFF)

# GoogleTest (tests only)
if(HAKKA_JSON_BUILD_TESTS)
    if(HAKKA_JSON_USE_SYSTEM_DEPS)
        find_package(GTest 1.15 QUIET)
    endif()
    if(NOT GTest_FOUND)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.15.2
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        include(GoogleTest)

        # Apply ASAN flags to GoogleTest targets if enabled
        if(HAKKA_JSON_ENABLE_SANITIZER_ADDRESS AND MSVC)
            foreach(target gtest gtest_main gmock gmock_main)
                if(TARGET ${target})
                    target_compile_options(${target} PRIVATE /fsanitize=address)
                endif()
            endforeach()
        endif()
    endif()

    # Coverage tools (gcov + lcov for coverage reports)
    option(HAKKA_JSON_ENABLE_COVERAGE "Enable code coverage reporting" OFF)
    if(HAKKA_JSON_ENABLE_COVERAGE)
        find_program(LCOV_EXECUTABLE lcov)
        find_program(GENHTML_EXECUTABLE genhtml)

        if(NOT LCOV_EXECUTABLE)
            message(WARNING "lcov not found. Coverage report generation will not be available.")
        endif()
        if(NOT GENHTML_EXECUTABLE)
            message(WARNING "genhtml not found. HTML coverage report generation will not be available.")
        endif()

        if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
            message(STATUS "Coverage tools found: lcov=${LCOV_EXECUTABLE}, genhtml=${GENHTML_EXECUTABLE}")
        endif()
    endif()
endif()

# TBB (optional)
option(HAKKA_JSON_ENABLE_TBB "Enable TBB support" OFF)
if(HAKKA_JSON_ENABLE_TBB)
    find_package(TBB REQUIRED)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|i[3-6]86")
        add_compile_definitions(_ENABLE_STD_EXECUTION_POLICY)
    endif()
endif()
