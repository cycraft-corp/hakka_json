add_subdirectory(core)
add_subdirectory(capi)
add_subdirectory(perf)

# add_executable(draft test_mem_draft.cpp)
# target_link_libraries(draft PRIVATE hakka_json_core)
# target_include_directories(draft PRIVATE
#     ${CMAKE_SOURCE_DIR}/include/
#     ${CMAKE_BINARY_DIR}/_deps/expected-src/include/
# )

# Coverage report generation (gcov + lcov)
if(HAKKA_JSON_ENABLE_COVERAGE)
    if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        # Clean coverage data
        add_custom_target(coverage_clean
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Cleaning coverage data..."
        )

        # Generate coverage data by running tests
        add_custom_target(coverage_generate
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            DEPENDS coverage_clean
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running tests to generate coverage data..."
        )

        # Generate HTML coverage report
        add_custom_target(coverage_report
            COMMAND ${LCOV_EXECUTABLE}
                --capture
                --directory ${CMAKE_BINARY_DIR}
                --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info
                --rc branch_coverage=1
                --ignore-errors inconsistent,mismatch
            COMMAND ${LCOV_EXECUTABLE}
                --remove ${CMAKE_BINARY_DIR}/coverage/coverage.info
                '/usr/*' '*/tests/*' '*/_deps/*'
                --output-file ${CMAKE_BINARY_DIR}/coverage/coverage_filtered.info
                --rc branch_coverage=1
                --ignore-errors inconsistent,mismatch,unused
            COMMAND ${GENHTML_EXECUTABLE}
                ${CMAKE_BINARY_DIR}/coverage/coverage_filtered.info
                --output-directory ${CMAKE_BINARY_DIR}/coverage/html
                --branch-coverage
                --title "Hakka JSON Coverage Report"
                --legend
                --ignore-errors inconsistent,mismatch
            DEPENDS coverage_generate
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating HTML coverage report..."
        )

        # Master coverage target
        add_custom_target(coverage
            DEPENDS coverage_report
            COMMENT "Coverage report available at: ${CMAKE_BINARY_DIR}/coverage/html/index.html"
        )
    else()
        message(WARNING "Coverage tools not found. Coverage targets will not be available.")
    endif()
endif()
