# CMakeLists.txt - LaminPie Library
cmake_minimum_required(VERSION 3.19.0)
project(LaminPie LANGUAGES C CXX)

set(LAMINPIE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

option(LAMINPIE_ENABLE_TEST "Enable LaminPie tests" ON)

if (ESP_PLATFORM)
    include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/idf.cmake)
    message(STATUS "ESP_PLATFORM")
else()
    include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/basic.cmake)
    message(STATUS "Building LaminPie as library for generic platform")
endif()
 