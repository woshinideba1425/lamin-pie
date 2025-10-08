# CMakeLists.txt
cmake_minimum_required(VERSION 3.19.0)

set(LAMINPIE_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

if (ESP_PLATFORM)
    include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/idf.cmake)
    message(STATUS "ESP_PLATFORM")
    
    # 包含测试配置（仅在ESP平台）
    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/test/CMakeLists.txt)
        include(${CMAKE_CURRENT_LIST_DIR}/test/CMakeLists.txt)
        message(STATUS "Including test configuration for ESP-IDF platform")
    endif()
else()
    include(${CMAKE_CURRENT_LIST_DIR}/env_support/cmake/basic.cmake)
endif()
 