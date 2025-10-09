# FindLVGL.cmake - Enhanced LVGL library detection
# This module provides improved LVGL library detection with multiple fallback methods

cmake_minimum_required(VERSION 3.10)

# Set default values
set(LVGL_FOUND FALSE)
set(LVGL_ROOT_DIR "")
set(LVGL_INC_DIR "")
set(LVGL_SRC_DIR "")
set(LVGL_VERSION_MAJOR "")
set(LVGL_VERSION_MINOR "")
set(LVGL_VERSION_PATCH "")

# Allow user to specify LVGL_ROOT_DIR manually
if(DEFINED LVGL_ROOT_DIR AND EXISTS ${LVGL_ROOT_DIR})
    if(EXISTS ${LVGL_ROOT_DIR}/lvgl.h AND EXISTS ${LVGL_ROOT_DIR}/src)
        set(LVGL_INC_DIR ${LVGL_ROOT_DIR})
        set(LVGL_SRC_DIR ${LVGL_ROOT_DIR}/src)
        set(LVGL_FOUND TRUE)
        message(STATUS "Using manually specified LVGL at: ${LVGL_ROOT_DIR}")
    else()
        message(WARNING "Manually specified LVGL_ROOT_DIR does not contain valid LVGL installation: ${LVGL_ROOT_DIR}")
    endif()
endif()

# Method 1: Try to find LVGL using find_package (if installed system-wide)
if(NOT LVGL_FOUND)
    find_package(lvgl QUIET)
    if(lvgl_FOUND)
        set(LVGL_FOUND TRUE)
        get_target_property(LVGL_INC_DIR lvgl::lvgl INTERFACE_INCLUDE_DIRECTORIES)
        message(STATUS "Found LVGL via find_package: ${LVGL_INC_DIR}")
    endif()
endif()

# Method 2: Check for managed_components directory (ESP-IDF style)
if(NOT LVGL_FOUND)
    set(POSSIBLE_LVGL_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/lvgl
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/lvgl
    )
    
    # Check for versioned LVGL directories (e.g., lvgl-8.3.0, lvgl-9.0.0)
    file(GLOB VERSIONED_LVGL_DIRS 
        ${CMAKE_CURRENT_SOURCE_DIR}/lvgl-*
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/lvgl-*
    )
    list(APPEND POSSIBLE_LVGL_DIRS ${VERSIONED_LVGL_DIRS})
    
    foreach(LVGL_DIR ${POSSIBLE_LVGL_DIRS})
        if(EXISTS ${LVGL_DIR}/lvgl.h AND EXISTS ${LVGL_DIR}/src)
            set(LVGL_ROOT_DIR ${LVGL_DIR})
            set(LVGL_INC_DIR ${LVGL_DIR})
            set(LVGL_SRC_DIR ${LVGL_DIR}/src)
            set(LVGL_FOUND TRUE)
            message(STATUS "Found LVGL at: ${LVGL_ROOT_DIR}")
            break()
        endif()
    endforeach()
endif()

# Method 3: Check common system installation paths
if(NOT LVGL_FOUND)
    find_path(LVGL_INCLUDE_DIR
        NAMES lvgl.h
        PATHS
            /usr/include/lvgl
            /usr/local/include/lvgl
            /opt/lvgl/include
            ${CMAKE_PREFIX_PATH}/include/lvgl
        PATH_SUFFIXES lvgl
    )
    
    if(LVGL_INCLUDE_DIR)
        get_filename_component(LVGL_ROOT_DIR ${LVGL_INCLUDE_DIR} DIRECTORY)
        set(LVGL_INC_DIR ${LVGL_INCLUDE_DIR})
        set(LVGL_SRC_DIR ${LVGL_ROOT_DIR}/src)
        set(LVGL_FOUND TRUE)
        message(STATUS "Found LVGL system installation at: ${LVGL_ROOT_DIR}")
    endif()
endif()

# Extract version information if LVGL is found
if(LVGL_FOUND)
    # Check for LVGL version if possible
    if(EXISTS ${LVGL_ROOT_DIR}/lv_version.h)
        file(READ ${LVGL_ROOT_DIR}/lv_version.h LVGL_VERSION_CONTENT)
        string(REGEX MATCH "LVGL_VERSION_MAJOR[ ]+([0-9]+)" _ ${LVGL_VERSION_CONTENT})
        set(LVGL_VERSION_MAJOR ${CMAKE_MATCH_1})
        string(REGEX MATCH "LVGL_VERSION_MINOR[ ]+([0-9]+)" _ ${LVGL_VERSION_CONTENT})
        set(LVGL_VERSION_MINOR ${CMAKE_MATCH_1})
        string(REGEX MATCH "LVGL_VERSION_PATCH[ ]+([0-9]+)" _ ${LVGL_VERSION_CONTENT})
        set(LVGL_VERSION_PATCH ${CMAKE_MATCH_1})
    endif()
    
    # Report detailed information
    message(STATUS "LVGL found successfully:")
    message(STATUS "  Root directory: ${LVGL_ROOT_DIR}")
    message(STATUS "  Include directory: ${LVGL_INC_DIR}")
    message(STATUS "  Source directory: ${LVGL_SRC_DIR}")
    if(LVGL_VERSION_MAJOR)
        message(STATUS "  Version: ${LVGL_VERSION_MAJOR}.${LVGL_VERSION_MINOR}.${LVGL_VERSION_PATCH}")
    endif()
    
    # Set standard CMake variables
    set(LVGL_FOUND TRUE CACHE BOOL "LVGL library found")
    set(LVGL_ROOT_DIR ${LVGL_ROOT_DIR} CACHE PATH "LVGL root directory")
    set(LVGL_INC_DIR ${LVGL_INC_DIR} CACHE PATH "LVGL include directory")
    set(LVGL_SRC_DIR ${LVGL_SRC_DIR} CACHE PATH "LVGL source directory")
    
    # Create imported target if not already created
    if(NOT TARGET lvgl::lvgl)
        add_library(lvgl::lvgl INTERFACE IMPORTED)
        set_target_properties(lvgl::lvgl PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${LVGL_INC_DIR};${LVGL_SRC_DIR}"
        )
    endif()
    
else()
    # Provide helpful error message with suggestions
    message(FATAL_ERROR "LVGL library not found! Please ensure LVGL is installed or available in one of the following locations:
    - System installation (use find_package)
    - managed_components/lvgl__lvgl (ESP-IDF style)
    - external/lvgl, third_party/lvgl, or vendor/lvgl directories
    - Standard system paths (/usr/include/lvgl, /usr/local/include/lvgl, etc.)
    - Package managers (vcpkg, conan, etc.)
    
    You can also set LVGL_ROOT_DIR manually to specify the LVGL installation path.
    
    Example:
      cmake -DLVGL_ROOT_DIR=/path/to/lvgl ..
      or
      set(LVGL_ROOT_DIR /path/to/lvgl) in your CMakeLists.txt")
endif()

# Mark variables as advanced to hide them from GUI
mark_as_advanced(LVGL_ROOT_DIR LVGL_INC_DIR LVGL_SRC_DIR)
