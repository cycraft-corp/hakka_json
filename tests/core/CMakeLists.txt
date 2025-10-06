# Core tests (explicit source list)
set(TEST_SOURCES
    hakka_json_array.cpp
    hakka_json_bool.cpp
    hakka_json_float.cpp
    hakka_json_int.cpp
    hakka_json_invalid.cpp
    hakka_json_null.cpp
    hakka_json_object.cpp
    hakka_json_string.cpp
)

foreach(source ${TEST_SOURCES})
    get_filename_component(name ${source} NAME_WE)
    add_executable(t_${name} ${source})

    target_link_libraries(t_${name}
        PRIVATE
            hakka_json::core
            hakka_json::compiler_options
            GTest::gtest_main
            GTest::gmock_main
    )

    # Link coverage options if enabled
    if(HAKKA_JSON_ENABLE_COVERAGE)
        target_link_libraries(t_${name} PRIVATE hakka_json::coverage_options)
    endif()

    # Windows: Copy ICU DLLs to test executable directory
    if(WIN32)
        add_custom_command(TARGET t_${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${ICU_INSTALL_DIR}/bin"
                $<TARGET_FILE_DIR:t_${name}>
            COMMAND ${CMAKE_COMMAND} -E echo "Copied all ICU DLLs to test directory"
        )
        # Delay test discovery until runtime to ensure DLLs are available
        gtest_discover_tests(t_${name}
            DISCOVERY_MODE PRE_TEST
            WORKING_DIRECTORY $<TARGET_FILE_DIR:t_${name}>
        )
    else()
        gtest_discover_tests(t_${name})
    endif()
endforeach()
