# C API tests (explicit source list)
set(TEST_SOURCES
    hakka_json_array.cpp
    hakka_json_complex.cpp
    hakka_json_object.cpp
    hakka_json_primitive.cpp
)

# Windows: Create a single target to copy ICU DLLs and hakka_json.dll (avoids parallel build race condition)
if(WIN32)
    # Use a dummy file as marker to ensure copy happens once per config
    set(CAPI_DLLS_MARKER "${CMAKE_CURRENT_BINARY_DIR}/capi_dlls_copied_$<CONFIG>.stamp")
    add_custom_command(
        OUTPUT ${CAPI_DLLS_MARKER}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ICU_INSTALL_DIR}/bin"
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:hakka_json>
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
        COMMAND ${CMAKE_COMMAND} -E touch ${CAPI_DLLS_MARKER}
        COMMENT "Copying ICU DLLs and hakka_json.dll to capi tests directory"
        DEPENDS hakka_json
        VERBATIM
    )
    add_custom_target(copy_dlls_capi_tests DEPENDS ${CAPI_DLLS_MARKER})

    # Copy ASAN runtime DLL if enabled
    if(HAKKA_JSON_ENABLE_SANITIZER_ADDRESS AND ASAN_RUNTIME_DLL)
        add_custom_command(TARGET copy_dlls_capi_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ASAN_RUNTIME_DLL}"
                "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
            COMMENT "Copying ASAN runtime DLL to capi tests directory"
            VERBATIM
        )
    endif()
endif()

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

    # Windows: Depend on DLL copy target
    if(WIN32)
        add_dependencies(capi_${name} copy_dlls_capi_tests)
        # Delay test discovery until runtime to ensure DLLs are available
        gtest_discover_tests(capi_${name}
            DISCOVERY_MODE PRE_TEST
            WORKING_DIRECTORY $<TARGET_FILE_DIR:capi_${name}>
        )
    else()
        gtest_discover_tests(capi_${name})
    endif()
endforeach()
