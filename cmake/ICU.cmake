cmake_minimum_required(VERSION 3.12)

# User-configurable variables
set(ICU_VERSION "" CACHE STRING "Specify the version of ICU to use (e.g., 76.1). Leave empty to use the latest release.")
set(ICU_ROOT "" CACHE PATH "Specify the installation path of ICU, if already installed")
set(ICU_MSBUILD_PATH "" CACHE PATH "Path to msbuild.exe for building on Windows")
set(ICU_ARCH "x64" CACHE STRING "Target architecture: x86 or x64")

# Internal variables
set(ICU_SRC_DIR "${CMAKE_BINARY_DIR}/icu-src")
set(ICU_BUILD_DIR "${CMAKE_BINARY_DIR}/icu-build")
set(ICU_INSTALL_DIR "${CMAKE_BINARY_DIR}/icu-install")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CONFIGURE_ICU_PLATFORM "Linux")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CONFIGURE_ICU_PLATFORM "macOS")
endif()

include(ProcessorCount)
ProcessorCount(N)

# Function to determine ICU version and URL
function(determine_icu_version_and_url)
    if(ICU_VERSION)
        string(REPLACE "." "-" ICU_VERSION_DASH "${ICU_VERSION}")
        string(REPLACE "." "_" ICU_VERSION_UNDERSCORE "${ICU_VERSION}")
        set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-${ICU_VERSION_DASH}/icu4c-${ICU_VERSION_UNDERSCORE}-src.tgz" PARENT_SCOPE)
    else()
        # Fetch the latest release version
        file(DOWNLOAD "https://api.github.com/repos/unicode-org/icu/releases/latest" "${CMAKE_BINARY_DIR}/icu_latest_release.json" STATUS status)
        if(NOT status EQUAL 0)
            message(FATAL_ERROR "Failed to fetch the latest ICU release information.")
        endif()
        file(READ "${CMAKE_BINARY_DIR}/icu_latest_release.json" latest_release_content)
        string(REGEX MATCH "\"tag_name\": \"([^\"]+)\"" _ ${latest_release_content})
        set(ICU_VERSION "${CMAKE_MATCH_1}")
        string(REPLACE "release-" "" ICU_VERSION "${ICU_VERSION}")
        string(REPLACE "-" "." ICU_VERSION "${ICU_VERSION}")
        string(REPLACE "." "-" ICU_VERSION_DASH "${ICU_VERSION}")
        string(REPLACE "." "_" ICU_VERSION_UNDERSCORE "${ICU_VERSION}")
        set(ICU_URL "https://github.com/unicode-org/icu/releases/download/release-${ICU_VERSION_DASH}/icu4c-${ICU_VERSION_UNDERSCORE}-src.tgz" PARENT_SCOPE)
    endif()
endfunction()

# Function to handle ICU build
function(build_icu_unix)
    ExternalProject_Add(
        icu_project
        URL ${ICU_URL}
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SOURCE_DIR ${ICU_SRC_DIR}
        BINARY_DIR ${ICU_BUILD_DIR}
        INSTALL_DIR ${ICU_INSTALL_DIR}
        CONFIGURE_COMMAND 
            ${ICU_SRC_DIR}/source/runConfigureICU ${CONFIGURE_ICU_PLATFORM} --prefix=${ICU_INSTALL_DIR} --enable-static --disable-renaming --with-data-packaging=static CXXFLAGS=-fPIC LDFLAGS=-Wl,-rpath,${ICU_INSTALL_DIR}/lib
        BUILD_COMMAND
            make -j ${N}
        INSTALL_COMMAND
            make install
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
        BUILD_BYPRODUCTS
            ${ICU_INSTALL_DIR}/lib/libicui18n.a
            ${ICU_INSTALL_DIR}/lib/libicuuc.a
            ${ICU_INSTALL_DIR}/lib/libicudata.a
            ${ICU_INSTALL_DIR}/lib/libicuio.a
            ${ICU_INSTALL_DIR}/lib/libicutu.a
    )
endfunction()

function(build_icu_windows)
    if(NOT ICU_MSBUILD_PATH)
        set(ICU_MSBUILD_PATH "MSBuild.exe")
    endif()

    # Map ICU_ARCH to MSBuild platform
    if(ICU_ARCH STREQUAL "x86")
        set(MSBUILD_PLATFORM "Win32")
        set(WINDOWS_SRC_BIN_DIR "${ICU_SRC_DIR}/bin")
        set(WINDOWS_SRC_LIB_DIR "${ICU_SRC_DIR}/lib")
    elseif(ICU_ARCH STREQUAL "x64")
        set(MSBUILD_PLATFORM "x64")
        set(WINDOWS_SRC_BIN_DIR "${ICU_SRC_DIR}/bin64")
        set(WINDOWS_SRC_LIB_DIR "${ICU_SRC_DIR}/lib64")
    else()
        message(FATAL_ERROR "Unsupported architecture: ${ICU_ARCH}. Please specify 'x86' or 'x64'.")
    endif()

    ExternalProject_Add(
        icu_project
        URL ${ICU_URL}
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SOURCE_DIR ${ICU_SRC_DIR}
        BINARY_DIR ${ICU_BUILD_DIR}
        INSTALL_DIR ${ICU_INSTALL_DIR}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND 
            ${ICU_MSBUILD_PATH} ${ICU_SRC_DIR}/source/allinone/allinone.sln 
            /p:Configuration=Release /p:Platform=${MSBUILD_PLATFORM} /p:SkipUWP=true
        INSTALL_COMMAND 
            ${CMAKE_COMMAND} -E copy_directory 
            ${WINDOWS_SRC_BIN_DIR} ${ICU_INSTALL_DIR}/bin
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${WINDOWS_SRC_LIB_DIR} ${ICU_INSTALL_DIR}/lib
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${ICU_SRC_DIR}/include ${ICU_INSTALL_DIR}/include
        LOG_DOWNLOAD ON
        LOG_BUILD ON
        LOG_INSTALL ON
        WORKING_DIRECTORY ${ICU_SRC_DIR}
        BUILD_BYPRODUCTS
            ${ICU_INSTALL_DIR}/lib/icuin.lib
            ${ICU_INSTALL_DIR}/lib/icuuc.lib
            ${ICU_INSTALL_DIR}/lib/icudt.lib
            ${ICU_INSTALL_DIR}/lib/icuio.lib
            ${ICU_INSTALL_DIR}/lib/icutu.lib
    )
endfunction()

function(build_icu)
    if(UNIX)
        build_icu_unix()
    elseif(WIN32)
        build_icu_windows()
    else()
        message(FATAL_ERROR "Unsupported platform")
    endif()
endfunction()

# Main logic
if(ICU_ROOT AND EXISTS "${ICU_ROOT}")
    message(STATUS "Using ICU installation from ${ICU_ROOT}")
    set(ICU_INCLUDE_DIR "${ICU_ROOT}/include" CACHE INTERNAL "ICU include directory")
    set(ICU_LIB_DIR "${ICU_ROOT}/lib" CACHE INTERNAL "ICU library directory")
else()
    include(ExternalProject)
    determine_icu_version_and_url()
    build_icu()
    set(ICU_INCLUDE_DIR "${ICU_INSTALL_DIR}/include" CACHE INTERNAL "ICU include directory")
    set(ICU_LIB_DIR "${ICU_INSTALL_DIR}/lib" CACHE INTERNAL "ICU library directory")
endif()

# Export variables for dependent projects
set(ICU_INCLUDE_DIR ${ICU_INCLUDE_DIR} PARENT_SCOPE)
set(ICU_LIB_DIR ${ICU_LIB_DIR} PARENT_SCOPE)
set(ICU_WINDOWS_LIBRARY_NAMES
    icutu
    icuin
    icuio
    icuuc
    icudt
)
set(ICU_UNIX_LIBRARY_NAMES
    icutu
    icui18n
    icuio
    icuuc
    icudata
)
if (WIN32)
    set(ICU_LIBRARY_NAMES ${ICU_WINDOWS_LIBRARY_NAMES})
else()
    set(ICU_LIBRARY_NAMES ${ICU_UNIX_LIBRARY_NAMES})
endif()

add_custom_target(icu_project_libs)
if (WIN32)
    # On Windows, libraries have .lib extension
    foreach(lib ${ICU_WINDOWS_LIBRARY_NAMES})
        set(LIB_STATIC_PATH "${ICU_LIB_DIR}/${lib}.lib")
        list(APPEND ICU_LIBRARIES "${LIB_STATIC_PATH}")
        list(APPEND ICU_SHARED_LIBRARIES "${ICU_LIB_DIR}/${lib}.dll")
        add_library(${lib} STATIC IMPORTED GLOBAL)
        set_target_properties(${lib} PROPERTIES 
            IMPORTED_LOCATION "${LIB_STATIC_PATH}"
        )
        add_dependencies(${lib} icu_project)
        add_dependencies(icu_project_libs ${lib})
    endforeach(lib ${ICU_WINDOWS_LIBRARY_NAMES})
else()
    # On Unix-like systems, libraries have .a extension
    foreach(lib ${ICU_UNIX_LIBRARY_NAMES})
        set(LIB_STATIC_PATH "${ICU_LIB_DIR}/lib${lib}.a")
        if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            set(LIB_SHARED_PATH "${ICU_LIB_DIR}/lib${lib}.so")
        elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            set(LIB_SHARED_PATH "${ICU_LIB_DIR}/lib${lib}.dylib")
        endif()

        list(APPEND ICU_LIBRARIES "${LIB_STATIC_PATH}")
        list(APPEND ICU_SHARED_LIBRARIES "${LIB_SHARED_PATH}")

        add_library(${lib} STATIC IMPORTED GLOBAL)
        add_library(${lib}_shared SHARED IMPORTED GLOBAL)

        set_target_properties(${lib} PROPERTIES 
            IMPORTED_LOCATION "${LIB_STATIC_PATH}"
        )
        set_target_properties(${lib}_shared PROPERTIES 
            IMPORTED_LOCATION "${LIB_SHARED_PATH}"
        )

        add_dependencies(${lib} icu_project)
        add_dependencies(${lib}_shared icu_project)
        add_dependencies(icu_project_libs ${lib} ${lib}_shared)
    endforeach()
endif()
set(ICU_LIBRARIES "${ICU_LIBRARIES}" PARENT_SCOPE)
set(ICU_SHARED_LIBRARIES "${ICU_SHARED_LIBRARIES}" PARENT_SCOPE)
