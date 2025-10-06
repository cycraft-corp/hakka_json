cmake_minimum_required(VERSION 3.20)
project(hakka_json
    VERSION 1.0.0
    LANGUAGES CXX C
    DESCRIPTION "Blazingly small, memory-efficient JSON library for C++"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(CompilerOptions)
include(Dependencies)

add_subdirectory(src)
add_subdirectory(capi)

# Install license files
install(FILES LICENSE LICENSE-BOOST LICENSE-BSD
        DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/hakka_json)

# Install headers
install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

# Install libraries (compiler_options is INTERFACE and not exported)
install(TARGETS hakka_json_core hakka_json
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Tests
option(HAKKA_JSON_BUILD_TESTS "Build tests" OFF)
if(HAKKA_JSON_BUILD_TESTS)
    enable_testing()
    include(CTest)
    add_subdirectory(tests)
endif()
