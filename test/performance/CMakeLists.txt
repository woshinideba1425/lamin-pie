# Performance Tests CMakeLists.txt - 细粒度版本
# 性能测试配置 - 每个测试文件独立编译

cmake_minimum_required(VERSION 3.16)

# 包含测试配置
include(../test_config.cmake)

# 设置包含目录
set(PERFORMANCE_INCLUDE_DIRS
    ${TEST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# 内存性能测试
if(ENABLE_MEMORY_PERFORMANCE_TEST)
    add_executable(test_memory_performance test_memory_performance.cpp)
    target_include_directories(test_memory_performance PRIVATE ${PERFORMANCE_INCLUDE_DIRS})
    target_link_libraries(test_memory_performance PRIVATE ${TEST_DEPENDENCIES} pthread)
    target_compile_features(test_memory_performance PRIVATE cxx_std_17)
    
    # 性能测试特殊编译选项
    target_compile_options(test_memory_performance PRIVATE -O2)
    target_compile_definitions(test_memory_performance PRIVATE ENABLE_PERFORMANCE_TESTS=1)
    
    enable_testing()
    add_test(NAME MemoryPerformanceTest COMMAND test_memory_performance)
    message(STATUS "Memory Performance Test: ENABLED")
endif()

# 打印性能测试配置
message(STATUS "=== Performance Tests Configuration (Fine-grained) ===")
message(STATUS "Memory Performance Test: ${ENABLE_MEMORY_PERFORMANCE_TEST}")
message(STATUS "=====================================================")