# Kconfig Integration for CMake
# This module provides functions to integrate Kconfig with CMake build system

# Find Python
find_package(Python3 REQUIRED)

# Set Kconfig tools directory
set(KCONFIG_TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools)

# Function to generate configuration from Kconfig
function(generate_kconfig_config)
    set(options VERBOSE)
    set(oneValueArgs KCONFIG_FILE OUTPUT_FILE DEFAULTS_FILE)
    set(multiValueArgs)
    
    cmake_parse_arguments(KCONFIG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT KCONFIG_KCONFIG_FILE)
        message(FATAL_ERROR "KCONFIG_FILE is required")
    endif()
    
    if(NOT KCONFIG_OUTPUT_FILE)
        set(KCONFIG_OUTPUT_FILE "sdkconfig.h")
    endif()
    
    # Create output directory if it doesn't exist
    get_filename_component(OUTPUT_DIR ${KCONFIG_OUTPUT_FILE} DIRECTORY)
    if(OUTPUT_DIR)
        file(MAKE_DIRECTORY ${OUTPUT_DIR})
    endif()
    
    # Build command
    get_filename_component(LAMINPIE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
    set(COMMAND ${Python3_EXECUTABLE} ${LAMINPIE_ROOT_DIR}/tools/config_generator.py)
    list(APPEND COMMAND ${KCONFIG_KCONFIG_FILE})
    list(APPEND COMMAND -o ${KCONFIG_OUTPUT_FILE})
    
    if(KCONFIG_DEFAULTS_FILE)
        list(APPEND COMMAND -d ${KCONFIG_DEFAULTS_FILE})
    endif()
    
    if(KCONFIG_VERBOSE)
        list(APPEND COMMAND -v)
    endif()
    
    # Execute command
    execute_process(
        COMMAND ${COMMAND}
        RESULT_VARIABLE RESULT
        OUTPUT_VARIABLE OUTPUT
        ERROR_VARIABLE ERROR
    )
    
    if(RESULT EQUAL 0)
        message(STATUS "Generated configuration: ${KCONFIG_OUTPUT_FILE}")
        if(KCONFIG_VERBOSE AND OUTPUT)
            message(STATUS "Output: ${OUTPUT}")
        endif()
    else()
        message(FATAL_ERROR "Failed to generate configuration: ${ERROR}")
    endif()
    
    # Add generated file to source list
    set(KCONFIG_GENERATED_FILES ${KCONFIG_OUTPUT_FILE} PARENT_SCOPE)
endfunction()

# Function to setup Kconfig for target
function(setup_kconfig_for_target TARGET_NAME)
    set(options VERBOSE)
    set(oneValueArgs KCONFIG_FILE DEFAULTS_FILE OUTPUT_DIR)
    set(multiValueArgs)
    
    cmake_parse_arguments(KCONFIG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT KCONFIG_KCONFIG_FILE)
        message(FATAL_ERROR "KCONFIG_FILE is required for target ${TARGET_NAME}")
    endif()
    
    if(NOT KCONFIG_OUTPUT_DIR)
        set(KCONFIG_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    
    set(OUTPUT_FILE ${KCONFIG_OUTPUT_DIR}/sdkconfig.h)
    
    # Generate configuration
    generate_kconfig_config(
        KCONFIG_FILE ${KCONFIG_KCONFIG_FILE}
        OUTPUT_FILE ${OUTPUT_FILE}
        DEFAULTS_FILE ${KCONFIG_DEFAULTS_FILE}
        VERBOSE ${KCONFIG_VERBOSE}
    )
    
    # Add include directory to target
    target_include_directories(${TARGET_NAME} PRIVATE ${KCONFIG_OUTPUT_DIR})
    
    # Add dependency on generated file
    set_target_properties(${TARGET_NAME} PROPERTIES
        KCONFIG_GENERATED_FILE ${OUTPUT_FILE}
    )
endfunction()

# Function to create Kconfig target
function(create_kconfig_target TARGET_NAME)
    set(options VERBOSE)
    set(oneValueArgs KCONFIG_FILE DEFAULTS_FILE OUTPUT_DIR)
    set(multiValueArgs)
    
    cmake_parse_arguments(KCONFIG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT KCONFIG_KCONFIG_FILE)
        message(FATAL_ERROR "KCONFIG_FILE is required for target ${TARGET_NAME}")
    endif()
    
    if(NOT KCONFIG_OUTPUT_DIR)
        set(KCONFIG_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    
    set(OUTPUT_FILE ${KCONFIG_OUTPUT_DIR}/sdkconfig.h)
    
    # Create custom target for configuration generation
    get_filename_component(LAMINPIE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
    
    # Build command list
    set(COMMAND_LIST ${Python3_EXECUTABLE} ${LAMINPIE_ROOT_DIR}/tools/config_generator.py)
    list(APPEND COMMAND_LIST ${KCONFIG_KCONFIG_FILE})
    list(APPEND COMMAND_LIST -o ${OUTPUT_FILE})
    
    if(KCONFIG_DEFAULTS_FILE)
        list(APPEND COMMAND_LIST -d ${KCONFIG_DEFAULTS_FILE})
    endif()
    
    if(KCONFIG_VERBOSE)
        list(APPEND COMMAND_LIST -v)
    endif()
    
    add_custom_target(${TARGET_NAME}
        COMMAND ${COMMAND_LIST}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating configuration from Kconfig"
        VERBATIM
    )
    
    # Set output file property
    set_target_properties(${TARGET_NAME} PROPERTIES
        KCONFIG_OUTPUT_FILE ${OUTPUT_FILE}
    )
endfunction()

# Function to add Kconfig dependency to target
function(add_kconfig_dependency TARGET_NAME KCONFIG_TARGET_NAME)
    # Get the output file from Kconfig target
    get_target_property(OUTPUT_FILE ${KCONFIG_TARGET_NAME} KCONFIG_OUTPUT_FILE)
    
    if(OUTPUT_FILE)
        # Add dependency
        add_dependencies(${TARGET_NAME} ${KCONFIG_TARGET_NAME})
        
        # Add include directory
        get_filename_component(OUTPUT_DIR ${OUTPUT_FILE} DIRECTORY)
        target_include_directories(${TARGET_NAME} PRIVATE ${OUTPUT_DIR})
    else()
        message(WARNING "Kconfig target ${KCONFIG_TARGET_NAME} has no output file")
    endif()
endfunction()

# Print Kconfig configuration
function(print_kconfig_info)
    message(STATUS "=== Kconfig Configuration ===")
    message(STATUS "Python executable: ${Python3_EXECUTABLE}")
    message(STATUS "Kconfig tools directory: ${KCONFIG_TOOLS_DIR}")
    message(STATUS "=============================")
endfunction()
