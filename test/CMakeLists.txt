# LaminPie Test CMakeLists.txt - Independent Executable Project

cmake_minimum_required(VERSION 3.10)
project(laminpie_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For GCC 9.x, use c++2a instead of c++20
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10.0")
    set(CMAKE_CXX_STANDARD 2a)
endif()

# Find Google Test
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTEST REQUIRED gtest)

# Include Google Test
include_directories(${GTEST_INCLUDE_DIRS})

# Set up LaminPie root directory
set(LAMINPIE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Build LaminPie library first
add_subdirectory(${LAMINPIE_ROOT_DIR} laminpie_lib EXCLUDE_FROM_ALL)

# Get LaminPie include directories from the built library
get_target_property(LAMINPIE_INCLUDE_DIRS laminpie INTERFACE_INCLUDE_DIRECTORIES)
if(NOT LAMINPIE_INCLUDE_DIRS)
    # Fallback: manually set include directories
    set(LAMINPIE_INCLUDE_DIRS 
        ${LAMINPIE_ROOT_DIR}
        ${LAMINPIE_ROOT_DIR}/src/common
        ${LAMINPIE_ROOT_DIR}/src/device
        ${LAMINPIE_ROOT_DIR}/src/device/buses
        ${LAMINPIE_ROOT_DIR}/src/device/drivers
        ${LAMINPIE_ROOT_DIR}/src/device/drivers/i2c
        ${LAMINPIE_ROOT_DIR}/src/device/interface
        ${LAMINPIE_ROOT_DIR}/src/core/systems
        ${LAMINPIE_ROOT_DIR}/src/core/systems/app
        ${LAMINPIE_ROOT_DIR}/src/core/systems/framework
        ${LAMINPIE_ROOT_DIR}/src/core/gui
        ${LAMINPIE_ROOT_DIR}/src/core/gui/lvgl
        ${LAMINPIE_ROOT_DIR}/src/porting
    )
endif()

# Find LVGL
set(LVGL_ROOT_DIR ${LAMINPIE_ROOT_DIR}/../managed_components/lvgl__lvgl)
if(EXISTS ${LVGL_ROOT_DIR})
    message(STATUS "Using LVGL from: ${LVGL_ROOT_DIR}")
    # If LVGL has CMakeLists.txt, build it as a subdirectory
    if(EXISTS ${LVGL_ROOT_DIR}/CMakeLists.txt)
        # Disable LVGL examples to avoid configuration issues
        set(LVGL_BUILD_EXAMPLES OFF CACHE BOOL "Build LVGL examples" FORCE)
        set(LVGL_USE_EXAMPLES OFF CACHE BOOL "Use LVGL examples" FORCE)
        add_subdirectory(${LVGL_ROOT_DIR} lvgl_build)
        message(STATUS "LVGL library will be built from: ${LVGL_ROOT_DIR}")
    else()
        message(STATUS "LVGL found but no CMakeLists.txt - using as header-only library")
    endif()
    set(LVGL_FOUND TRUE)
else()
    message(FATAL_ERROR "LVGL not found! Please ensure LVGL is available.")
endif()

# Include Kconfig support
include(${LAMINPIE_ROOT_DIR}/env_support/cmake/kconfig.cmake)

# Generate configuration from Kconfig
create_kconfig_target(laminpie_config
    KCONFIG_FILE ${LAMINPIE_ROOT_DIR}/Kconfig
    DEFAULTS_FILE ${LAMINPIE_ROOT_DIR}/tools/default_config.txt
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/config
    VERBOSE
)

# Test source files
set(TEST_SOURCES
    main.cpp
    test_common_gtest.cpp
    unit/test_log_system_gtest.cpp
    performance/test_memory_performance_gtest.cpp
    integration/test_device_system_gtest.cpp
    integration/test_thread_system_gtest.cpp
    integration/test_app_manager_integration_gtest.cpp
    integration/test_event_system_gtest.cpp
    integration/test_event_stress_gtest.cpp
)

# Create test executable
add_executable(laminpie_tests ${TEST_SOURCES})

# Add Kconfig dependency
add_kconfig_dependency(laminpie_tests laminpie_config)

# Include directories
target_include_directories(laminpie_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/config
    ${LAMINPIE_INCLUDE_DIRS}
    ${LVGL_ROOT_DIR}
    ${LVGL_ROOT_DIR}/src
    ${GTEST_INCLUDE_DIRS}
)

# Ensure sdkconfig.h is found
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/config/sdkconfig.h)
    message(STATUS "Found sdkconfig.h in build config directory")
else()
    message(WARNING "sdkconfig.h not found in ${CMAKE_CURRENT_BINARY_DIR}/config/")
endif()

# Link libraries
target_link_libraries(laminpie_tests
    laminpie
    lvgl
    ${GTEST_LIBRARIES}
    pthread
)

# Compile definitions
target_compile_definitions(laminpie_tests PRIVATE
    PLATFORM_GENERIC
    LAMINPIE_CONF_SKIP
    ENABLE_LOG_TESTS=1
    ENABLE_EVENT_SYSTEM_TEST=1
    ENABLE_DEVICE_SYSTEM_TEST=1
    ENABLE_THREAD_SYSTEM_TEST=1
    ENABLE_MEMORY_PERFORMANCE_TEST=1
)

# Compile options
target_compile_options(laminpie_tests PRIVATE
    ${GTEST_CFLAGS_OTHER}
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# Print configuration
message(STATUS "=== LaminPie Test Configuration ===")
message(STATUS "GTEST_INCLUDE_DIRS: ${GTEST_INCLUDE_DIRS}")
message(STATUS "GTEST_LIBRARIES: ${GTEST_LIBRARIES}")
message(STATUS "GTEST_LIBRARY_DIRS: ${GTEST_LIBRARY_DIRS}")
message(STATUS "LAMINPIE_INCLUDE_DIRS: ${LAMINPIE_INCLUDE_DIRS}")
message(STATUS "LAMINPIE_LIBRARIES: ${LAMINPIE_LIBRARIES}")
message(STATUS "===================================")