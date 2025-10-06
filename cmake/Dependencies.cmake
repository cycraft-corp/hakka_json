include(FetchContent)

option(HAKKA_JSON_USE_SYSTEM_DEPS "Try system packages first" ON)

# nlohmann_json
if(HAKKA_JSON_USE_SYSTEM_DEPS)
    find_package(nlohmann_json 3.11 QUIET)
endif()
if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# tl::expected
if(HAKKA_JSON_USE_SYSTEM_DEPS)
    find_package(tl-expected 1.1 QUIET)
endif()
if(NOT tl-expected_FOUND)
    FetchContent_Declare(
        expected
        GIT_REPOSITORY https://github.com/TartanLlama/expected.git
        GIT_TAG v1.1.0
    )
    set(EXPECTED_BUILD_TESTS OFF)
    FetchContent_MakeAvailable(expected)
endif()

# ICU - wrap old script to handle PARENT_SCOPE
function(_setup_icu)
    set(ICU_VERSION "" CACHE STRING "ICU version")
    set(ICU_ROOT "" CACHE PATH "ICU installation path")
    include(${CMAKE_SOURCE_DIR}/cmake/ICU.cmake)
    # Export to parent (Dependencies.cmake scope)
    set(ICU_INCLUDE_DIR ${ICU_INCLUDE_DIR} PARENT_SCOPE)
    set(ICU_LIB_DIR ${ICU_LIB_DIR} PARENT_SCOPE)
    set(ICU_LIBRARIES ${ICU_LIBRARIES} PARENT_SCOPE)
    set(ICU_SHARED_LIBRARIES ${ICU_SHARED_LIBRARIES} PARENT_SCOPE)
endfunction()
_setup_icu()

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
