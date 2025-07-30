# CMakeLists.txt
cmake_minimum_required(VERSION 3.19.0)

set(LAMINPIE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

if (ESP_PLATFORM)
    include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/idf.cmake)
    message(STATUS "ESP_PLATFORM")
else()
    include(${CMAKE_CURRENT_LIST_DIR}env_support/cmake/basic.cmake)
endif()
 