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

# Windows: Create a single target to copy ICU DLLs (avoids parallel build race condition)
if(WIN32)
    add_custom_target(copy_icu_dlls_core_tests
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ICU_INSTALL_DIR}/bin"
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
        COMMENT "Copying ICU DLLs to core tests directory"
        VERBATIM
    )
endif()

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

    # Windows: Depend on ICU DLL copy target
    if(WIN32)
        add_dependencies(t_${name} copy_icu_dlls_core_tests)
        # Delay test discovery until runtime to ensure DLLs are available
        gtest_discover_tests(t_${name}
            DISCOVERY_MODE PRE_TEST
            WORKING_DIRECTORY $<TARGET_FILE_DIR:t_${name}>
        )
    else()
        gtest_discover_tests(t_${name})
    endif()
endforeach()
