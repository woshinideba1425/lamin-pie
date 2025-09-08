# LaminPie Test CMakeLists.txt - 细粒度版本
# 测试配置 - 支持细粒度的测试模块控制

cmake_minimum_required(VERSION 3.16)

if(ESP_PLATFORM)
    message(STATUS "Building for ESP-IDF platform")
    set(IS_ESP_PLATFORM TRUE)
elseif(UNIX)
    # Linux 平台
    message(STATUS "Building for Linux platform")
    set(IS_LINUX_PLATFORM TRUE)
else()
    message(STATUS "Building for generic platform")
    set(IS_GENERIC_PLATFORM TRUE)
endif()

# =============================================================================
# 细粒度测试模块配置选项
# =============================================================================

# 日志系统测试
option(ENABLE_LOG_TESTS "Enable log system tests" ON)
option(ENABLE_LOG_UNIT_TESTS "Enable log unit tests" ON)

# 事件系统测试 - 细粒度控制
option(ENABLE_EVENT_TESTS "Enable event system tests" ON)
option(ENABLE_EVENT_SYSTEM_TEST "Enable event system integration test" ON)
option(ENABLE_EVENT_STRESS_TEST "Enable event stress test" ON)

# 应用系统测试 - 细粒度控制
option(ENABLE_APP_TESTS "Enable application system tests" OFF)
option(ENABLE_APP_SCHEDULER_TEST "Enable app scheduler test" OFF)

# 设备系统测试 - 细粒度控制
option(ENABLE_DEVICE_TESTS "Enable device system tests" OFF)
option(ENABLE_DEVICE_SYSTEM_TEST "Enable device system test" OFF)

# 线程系统测试 - 细粒度控制
option(ENABLE_THREAD_TESTS "Enable thread system tests" OFF)
option(ENABLE_THREAD_SYSTEM_TEST "Enable thread system test" OFF)

# 平台测试 - 细粒度控制
option(ENABLE_PLATFORM_TESTS "Enable platform compatibility tests" OFF)
option(ENABLE_PLATFORM_COMPATIBILITY_TEST "Enable platform compatibility test" OFF)
option(ENABLE_KCONFIG_TEST "Enable kconfig test" OFF)

# 性能测试 - 细粒度控制
option(ENABLE_MEMORY_TESTS "Enable memory management tests" OFF)
option(ENABLE_MEMORY_PERFORMANCE_TEST "Enable memory performance test" OFF)

# 通用测试选项
option(ENABLE_COVERAGE_ANALYSIS "Enable code coverage analysis" OFF)
option(ENABLE_STRESS_TESTS "Enable stress tests" OFF)
option(ENABLE_PERFORMANCE_TESTS "Enable performance tests" OFF)

# =============================================================================
# 细粒度测试源文件配置
# =============================================================================

# 日志系统测试文件
set(LOG_TEST_SOURCES "")
if(ENABLE_LOG_TESTS AND ENABLE_LOG_UNIT_TESTS)
    list(APPEND LOG_TEST_SOURCES
        unit/test_log_system.cpp
    )
endif()

# 事件系统测试文件
set(EVENT_TEST_SOURCES "")
if(ENABLE_EVENT_TESTS)
    if(ENABLE_EVENT_SYSTEM_TEST)
        list(APPEND EVENT_TEST_SOURCES
            integration/test_event_system.cpp
        )
    endif()
    if(ENABLE_EVENT_STRESS_TEST)
        list(APPEND EVENT_TEST_SOURCES
            integration/test_event_stress.cpp
        )
    endif()
endif()

# 应用系统测试文件
set(APP_TEST_SOURCES "")
if(ENABLE_APP_TESTS AND ENABLE_APP_SCHEDULER_TEST)
    list(APPEND APP_TEST_SOURCES
        integration/test_app_scheduler.cpp
    )
endif()

# 设备系统测试文件
set(DEVICE_TEST_SOURCES "")
if(ENABLE_DEVICE_TESTS AND ENABLE_DEVICE_SYSTEM_TEST)
    list(APPEND DEVICE_TEST_SOURCES
        integration/test_device_system.cpp
    )
endif()

# 线程系统测试文件
set(THREAD_TEST_SOURCES "")
if(ENABLE_THREAD_TESTS AND ENABLE_THREAD_SYSTEM_TEST)
    list(APPEND THREAD_TEST_SOURCES
        integration/test_thread_system.cpp
    )
endif()

# 平台测试文件
set(PLATFORM_TEST_SOURCES "")
if(ENABLE_PLATFORM_TESTS)
    if(ENABLE_PLATFORM_COMPATIBILITY_TEST)
        list(APPEND PLATFORM_TEST_SOURCES
            unit/test_platform_compatibility.cpp
        )
    endif()
    if(ENABLE_KCONFIG_TEST)
        list(APPEND PLATFORM_TEST_SOURCES
            unit/test_kconfig.cpp
        )
    endif()
endif()

# 性能测试文件
set(PERFORMANCE_TEST_SOURCES "")
if(ENABLE_MEMORY_TESTS AND ENABLE_MEMORY_PERFORMANCE_TEST)
    list(APPEND PERFORMANCE_TEST_SOURCES
        performance/test_memory_performance.cpp
    )
endif()

# =============================================================================
# 合并所有测试源文件
# =============================================================================

set(TEST_SOURCES
    test_main.cpp
    ${LOG_TEST_SOURCES}
    ${EVENT_TEST_SOURCES}
    ${APP_TEST_SOURCES}
    ${DEVICE_TEST_SOURCES}
    ${THREAD_TEST_SOURCES}
    ${PLATFORM_TEST_SOURCES}
    ${PERFORMANCE_TEST_SOURCES}
)

# =============================================================================
# 测试包含目录
# =============================================================================

set(TEST_INCLUDE_DIRS
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# =============================================================================
# 测试依赖库配置
# =============================================================================

set(TEST_DEPENDENCIES
    laminpie_log
)

# 根据启用的测试模块添加特定依赖
if(ENABLE_EVENT_TESTS)
    list(APPEND TEST_DEPENDENCIES
        laminpie_event
    )
endif()

if(ENABLE_APP_TESTS)
    list(APPEND TEST_DEPENDENCIES
        laminpie_app
    )
endif()

if(ENABLE_DEVICE_TESTS)
    list(APPEND TEST_DEPENDENCIES
        laminpie_device
    )
endif()

# =============================================================================
# 编译选项配置
# =============================================================================

# 内存检查配置
if(ENABLE_MEMORY_TESTS)
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=leak")
    endif()
endif()

# 覆盖率分析
if(ENABLE_COVERAGE_ANALYSIS)
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()
endif()

# 压力测试配置
if(ENABLE_STRESS_TESTS)
    add_definitions(-DENABLE_STRESS_TESTS=1)
endif()

# =============================================================================
# 根据平台设置不同的配置
# =============================================================================

if(IS_ESP_PLATFORM)
    # ESP-IDF 特定配置
    idf_component_register(
        SRCS ${TEST_SOURCES}
        INCLUDE_DIRS ${TEST_INCLUDE_DIRS}
        REQUIRES lamin-pie

    )
else()
    # 非ESP-IDF平台配置 - 细粒度测试可执行文件
    find_package(Threads REQUIRED)
    
    # 创建独立的测试可执行文件
    if(ENABLE_LOG_TESTS AND ENABLE_LOG_UNIT_TESTS)
        add_executable(test_log_system unit/test_log_system.cpp)
        target_include_directories(test_log_system PRIVATE ${TEST_INCLUDE_DIRS})
        target_link_libraries(test_log_system PRIVATE ${TEST_DEPENDENCIES} Threads::Threads)
        enable_testing()
        add_test(NAME LogSystemTest COMMAND test_log_system)
    endif()
    
    if(ENABLE_EVENT_SYSTEM_TEST)
        add_executable(test_event_system integration/test_event_system.cpp)
        target_include_directories(test_event_system PRIVATE ${TEST_INCLUDE_DIRS})
        target_link_libraries(test_event_system PRIVATE ${TEST_DEPENDENCIES} Threads::Threads)
        enable_testing()
        add_test(NAME EventSystemTest COMMAND test_event_system)
    endif()
    
    if(ENABLE_EVENT_STRESS_TEST)
        add_executable(test_event_stress integration/test_event_stress.cpp)
        target_include_directories(test_event_stress PRIVATE ${TEST_INCLUDE_DIRS})
        target_link_libraries(test_event_stress PRIVATE ${TEST_DEPENDENCIES} Threads::Threads)
        enable_testing()
        add_test(NAME EventStressTest COMMAND test_event_stress)
    endif()
    
    if(ENABLE_APP_SCHEDULER_TEST)
        add_executable(test_app_scheduler integration/test_app_scheduler.cpp)
        target_include_directories(test_app_scheduler PRIVATE ${TEST_INCLUDE_DIRS})
        target_link_libraries(test_app_scheduler PRIVATE ${TEST_DEPENDENCIES} Threads::Threads)
        enable_testing()
        add_test(NAME AppSchedulerTest COMMAND test_app_scheduler)
    endif()
    
    if(ENABLE_DEVICE_SYSTEM_TEST)
        add_executable(test_device_system integration/test_device_system.cpp)
        target_include_directories(test_device_system PRIVATE ${TEST_INCLUDE_DIRS})
        target_link_libraries(test_device_system PRIVATE ${TEST_DEPENDENCIES} Threads::Threads)
        enable_testing()
        add_test(NAME DeviceSystemTest COMMAND test_device_system)
    endif()
    
    if(ENABLE_THREAD_SYSTEM_TEST)
        add_executable(test_thread_system integration/test_thread_system.cpp)
        target_include_directories(test_thread_system PRIVATE ${TEST_INCLUDE_DIRS})
        target_link_libraries(test_thread_system PRIVATE ${TEST_DEPENDENCIES} Threads::Threads)
        enable_testing()
        add_test(NAME ThreadSystemTest COMMAND test_thread_system)
    endif()
    
    if(ENABLE_PLATFORM_COMPATIBILITY_TEST)
        add_executable(test_platform_compatibility unit/test_platform_compatibility.cpp)
        target_include_directories(test_platform_compatibility PRIVATE ${TEST_INCLUDE_DIRS})
        target_link_libraries(test_platform_compatibility PRIVATE ${TEST_DEPENDENCIES} Threads::Threads)
        enable_testing()
        add_test(NAME PlatformCompatibilityTest COMMAND test_platform_compatibility)
    endif()
    
    if(ENABLE_KCONFIG_TEST)
        add_executable(test_kconfig unit/test_kconfig.cpp)
        target_include_directories(test_kconfig PRIVATE ${TEST_INCLUDE_DIRS})
        target_link_libraries(test_kconfig PRIVATE ${TEST_DEPENDENCIES} Threads::Threads)
        enable_testing()
        add_test(NAME KConfigTest COMMAND test_kconfig)
    endif()
    
    if(ENABLE_MEMORY_PERFORMANCE_TEST)
        add_executable(test_memory_performance performance/test_memory_performance.cpp)
        target_include_directories(test_memory_performance PRIVATE ${TEST_INCLUDE_DIRS})
        target_link_libraries(test_memory_performance PRIVATE ${TEST_DEPENDENCIES} Threads::Threads)
        target_compile_options(test_memory_performance PRIVATE -O2)
        target_compile_definitions(test_memory_performance PRIVATE ENABLE_PERFORMANCE_TESTS=1)
        enable_testing()
        add_test(NAME MemoryPerformanceTest COMMAND test_memory_performance)
    endif()
    
    # 创建运行所有测试的目标
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS test_log_system test_event_system test_event_stress 
                test_app_scheduler test_device_system test_thread_system
                test_platform_compatibility test_kconfig test_memory_performance
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# =============================================================================
# 测试统计和报告
# =============================================================================

# 打印测试配置信息
message(STATUS "=== LaminPie Test Configuration (Fine-grained) ===")
message(STATUS "Log Tests: ${ENABLE_LOG_TESTS}")
message(STATUS "Event System Test: ${ENABLE_EVENT_SYSTEM_TEST}")
message(STATUS "Event Stress Test: ${ENABLE_EVENT_STRESS_TEST}")
message(STATUS "App Scheduler Test: ${ENABLE_APP_SCHEDULER_TEST}")
message(STATUS "Device System Test: ${ENABLE_DEVICE_SYSTEM_TEST}")
message(STATUS "Thread System Test: ${ENABLE_THREAD_SYSTEM_TEST}")
message(STATUS "Platform Compatibility Test: ${ENABLE_PLATFORM_COMPATIBILITY_TEST}")
message(STATUS "KConfig Test: ${ENABLE_KCONFIG_TEST}")
message(STATUS "Memory Performance Test: ${ENABLE_MEMORY_PERFORMANCE_TEST}")
message(STATUS "==================================================")