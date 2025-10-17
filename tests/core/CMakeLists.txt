# Core tests (explicit source list)
set(TEST_SOURCES
    hakka_json_array.cpp
    hakka_json_bool.cpp
    hakka_json_float.cpp
    hakka_json_int.cpp
    hakka_json_invalid.cpp
    hakka_json_null.cpp
    hakka_json_object.cpp
    hakka_json_python_hashing.cpp
    hakka_json_string.cpp
)

# Windows: Create a single target to copy ICU DLLs (avoids parallel build race condition)
if(WIN32)
    # Use a dummy file as marker to ensure copy happens once per config
    set(ICU_COPY_MARKER "${CMAKE_CURRENT_BINARY_DIR}/icu_dlls_copied_$<CONFIG>.stamp")
    add_custom_command(
        OUTPUT ${ICU_COPY_MARKER}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ICU_INSTALL_DIR}/bin"
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
        COMMAND ${CMAKE_COMMAND} -E touch ${ICU_COPY_MARKER}
        COMMENT "Copying ICU DLLs to core tests directory"
        VERBATIM
    )
    add_custom_target(copy_icu_dlls_core_tests DEPENDS ${ICU_COPY_MARKER})

    # Copy ASAN runtime DLL if enabled
    if(HAKKA_JSON_ENABLE_SANITIZER_ADDRESS AND ASAN_RUNTIME_DLL)
        add_custom_command(TARGET copy_icu_dlls_core_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ASAN_RUNTIME_DLL}"
                "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
            COMMENT "Copying ASAN runtime DLL to core tests directory"
            VERBATIM
        )
    endif()
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
