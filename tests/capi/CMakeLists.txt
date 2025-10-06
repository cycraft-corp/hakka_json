# C API tests (explicit source list)
set(TEST_SOURCES
    hakka_json_array.cpp
    hakka_json_complex.cpp
    hakka_json_object.cpp
    hakka_json_primitive.cpp
)

foreach(source ${TEST_SOURCES})
    get_filename_component(name ${source} NAME_WE)
    add_executable(capi_${name} ${source})

    target_link_libraries(capi_${name}
        PRIVATE
            hakka_json::capi
            hakka_json::compiler_options
            GTest::gtest_main
            GTest::gmock_main
    )

    # Link coverage options if enabled
    if(HAKKA_JSON_ENABLE_COVERAGE)
        target_link_libraries(capi_${name} PRIVATE hakka_json::coverage_options)
    endif()

    target_include_directories(capi_${name}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include/
            ${CMAKE_SOURCE_DIR}/capi
    )

    # Windows: Copy ICU DLLs and hakka_json.dll to test executable directory
    if(WIN32)
        add_custom_command(TARGET capi_${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${ICU_INSTALL_DIR}/bin"
                $<TARGET_FILE_DIR:capi_${name}>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:hakka_json>
                $<TARGET_FILE_DIR:capi_${name}>
            COMMAND ${CMAKE_COMMAND} -E echo "Copied all ICU DLLs and hakka_json.dll to test directory"
        )
        # Delay test discovery until runtime to ensure DLLs are available
        gtest_discover_tests(capi_${name}
            DISCOVERY_MODE PRE_TEST
            WORKING_DIRECTORY $<TARGET_FILE_DIR:capi_${name}>
        )
    else()
        gtest_discover_tests(capi_${name})
    endif()
endforeach()
