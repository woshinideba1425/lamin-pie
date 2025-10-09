# basic CMakeLists.txt - LaminPie Component for Generic Platform

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# LAMINPIE - Generic Platform Configuration
set(LAMINPIE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
file(GLOB_RECURSE SOURCES ${LAMINPIE_ROOT_DIR}/src/*.c)
set(SRCS_C "")
set(SRCS_CPP "")
set(LAMINPIE_COMPILE_OPTIONS "")
set(LAMINPIE_INC_DIRS ${LAMINPIE_ROOT_DIR})
set(LAMINPIE_SRC_DIRS ${LAMINPIE_ROOT_DIR}/src)
set(CORE_SRC_DIR ${LAMINPIE_ROOT_DIR}/src/core)

# Find LVGL library using enhanced detection module
include(${CMAKE_CURRENT_LIST_DIR}/FindLVGL.cmake)

# Platform-specific definitions for generic platform
add_definitions(-DPLATFORM_GENERIC)
add_definitions(-DLAMINPIE_CONF_SKIP)

# Enable required LaminPie modules for Linux platform
add_definitions(-DLAMINPIE_ENABLE_SERVICES=1)
add_definitions(-DLAMINPIE_ENABLE_SYSTEMS=1)
add_definitions(-DLAMINPIE_ENABLE_GUI=1)
add_definitions(-DLAMINPIE_ENABLE_AI_FRAMEWORK=1)

# Kconfig simulation for Linux platform (only if not using Kconfig)
if(NOT TARGET laminpie_config)
    add_definitions(-DCONFIG_LAMINPIE_ENABLE_SERVICES=1)
    add_definitions(-DCONFIG_LAMINPIE_ENABLE_SYSTEMS=1)
    add_definitions(-DCONFIG_LAMINPIE_ENABLE_GUI=1)
    add_definitions(-DCONFIG_LAMINPIE_ENABLE_AI_FRAMEWORK=1)
    add_definitions(-DCONFIG_LAMINPIE_USE_OS_VALUE=0)
    add_definitions(-DCONFIG_LAMINPIE_THREAD_STACK_SIZE_DEFAULT=4096)
    add_definitions(-DCONFIG_LAMINPIE_THREAD_PRIORITY_DEFAULT=5)
else()
    # When using Kconfig, only define if not already defined
    if(NOT DEFINED CONFIG_LAMINPIE_USE_OS_VALUE)
        add_definitions(-DCONFIG_LAMINPIE_USE_OS_VALUE=0)
    endif()
endif()

#
# COMMON
#
set(COMMON_SRC_DIR ${LAMINPIE_SRC_DIRS}/common)
file(GLOB_RECURSE COMMON_SRCS_C ${COMMON_SRC_DIR}/*.c)
file(GLOB_RECURSE COMMON_SRCS_CPP ${COMMON_SRC_DIR}/*.cpp)
list(APPEND SRCS_C ${COMMON_SRCS_C})
list(APPEND SRCS_CPP ${COMMON_SRCS_CPP})
list(APPEND LAMINPIE_INC_DIRS ${COMMON_SRC_DIR})

#
# DEVICE - Include headers only for Linux platform
#
set(DEVICE_SRC_DIR ${LAMINPIE_SRC_DIRS}/device)
set(BUS_SRC_DIR ${DEVICE_SRC_DIR}/buses)
set(DRIVER_SRC_DIR ${DEVICE_SRC_DIR}/drivers)
set(I2C_SRC_DIR ${DRIVER_SRC_DIR}/i2c)
set(INTERFACE_SRC_DIR ${DEVICE_SRC_DIR}/interface)
# Only include headers, don't compile device source files for Linux
list(APPEND LAMINPIE_INC_DIRS ${DEVICE_SRC_DIR} ${BUS_SRC_DIR} ${DRIVER_SRC_DIR} ${I2C_SRC_DIR} ${INTERFACE_SRC_DIR})    

#
# SYSTEMS
#
set(SYSTEMS_SRC_DIR ${CORE_SRC_DIR}/systems)
set(APP_SRC_DIR ${SYSTEMS_SRC_DIR}/app)
set(FRAMEWORK_SRC_DIR ${SYSTEMS_SRC_DIR}/framework)
file(GLOB_RECURSE SYSTEMS_SRCS_C ${SYSTEMS_SRC_DIR}/*.c)
file(GLOB_RECURSE SYSTEMS_SRCS_CPP ${SYSTEMS_SRC_DIR}/*.cpp)
file(GLOB_RECURSE SYSTEMS_SRC_C ${FRAMEWORK_SRC_DIR}/*.c)
file(GLOB_RECURSE SYSTEMS_SRC_CPP ${FRAMEWORK_SRC_DIR}/*.cpp)
list(APPEND SRCS_C ${SYSTEMS_SRCS_C})
list(APPEND SRCS_CPP ${SYSTEMS_SRCS_CPP})
list(APPEND LAMINPIE_INC_DIRS ${SYSTEMS_SRC_DIR} ${APP_SRC_DIR} ${FRAMEWORK_SRC_DIR})

#
# GUI
#
set(GUI_SRC_DIR ${CORE_SRC_DIR}/gui)
set(GUI_LVGL_SRC_DIR ${GUI_SRC_DIR}/lvgl)
file(GLOB_RECURSE GUI_SRCS_C ${GUI_SRC_DIR}/*.c)
file(GLOB_RECURSE GUI_SRCS_CPP ${GUI_SRC_DIR}/*.cpp)
list(APPEND SRCS_C ${GUI_SRCS_C})
list(APPEND SRCS_CPP ${GUI_SRCS_CPP})
list(APPEND LAMINPIE_INC_DIRS ${GUI_SRC_DIR} ${GUI_LVGL_SRC_DIR})
# Add LVGL include directory if found
if(LVGL_FOUND)
    list(APPEND LAMINPIE_INC_DIRS ${LVGL_INC_DIR} ${LVGL_SRC_DIR})
    message(STATUS "Added LVGL include directories to LaminPie")
else()
    message(WARNING "LVGL not found - GUI functionality may be limited")
endif()

#
# THREAD PORTING - Use std::thread for generic platform
#
set(PORTING_SRC_DIR ${LAMINPIE_SRC_DIRS}/porting)
set(PORTING_SRCS_C "")
set(PORTING_SRCS_CPP ${PORTING_SRC_DIR}/laminpie_std_thread.cpp)

# Add selected source files
list(APPEND SRCS_C ${PORTING_SRCS_C})
list(APPEND SRCS_CPP ${PORTING_SRCS_CPP})
list(APPEND LAMINPIE_INC_DIRS ${PORTING_SRC_DIR})

# Build LaminPie library (only if not already defined)
if(NOT TARGET laminpie)
    add_library(laminpie STATIC
        ${SRCS_C}
        ${SRCS_CPP}
    )
else()
    message(STATUS "laminpie target already exists, skipping creation")
endif()

# Add Kconfig dependency if Kconfig target exists
if(TARGET laminpie_config)
    add_dependencies(laminpie laminpie_config)
    get_target_property(OUTPUT_FILE laminpie_config KCONFIG_OUTPUT_FILE)
    if(OUTPUT_FILE)
        get_filename_component(OUTPUT_DIR ${OUTPUT_FILE} DIRECTORY)
        target_include_directories(laminpie PUBLIC ${OUTPUT_DIR})
        message(STATUS "Added Kconfig include directory: ${OUTPUT_DIR}")
    else()
        # Fallback: try to find sdkconfig.h in common locations
        set(POSSIBLE_CONFIG_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}/config
            ${CMAKE_CURRENT_BINARY_DIR}
            ${LAMINPIE_ROOT_DIR}/build/config
        )
        foreach(CONFIG_DIR ${POSSIBLE_CONFIG_DIRS})
            if(EXISTS ${CONFIG_DIR}/sdkconfig.h)
                target_include_directories(laminpie PUBLIC ${CONFIG_DIR})
                message(STATUS "Found sdkconfig.h in: ${CONFIG_DIR}")
                break()
            endif()
        endforeach()
    endif()
endif()

# Include directories
target_include_directories(laminpie PUBLIC
    ${LAMINPIE_INC_DIRS}
)

# Compile options
if(LAMINPIE_COMPILE_OPTIONS)
    target_compile_options(laminpie PRIVATE ${LAMINPIE_COMPILE_OPTIONS})
    message(STATUS "LaminPie compile options: ${LAMINPIE_COMPILE_OPTIONS}")
endif()

# Add C++20 concepts support
target_compile_options(laminpie PRIVATE -fconcepts)

# Link libraries
target_link_libraries(laminpie PUBLIC pthread)

# Export variables for use by other CMakeLists.txt files
set(LAMINPIE_LIBRARIES laminpie)
set(LAMINPIE_INCLUDE_DIRS ${LAMINPIE_INC_DIRS})