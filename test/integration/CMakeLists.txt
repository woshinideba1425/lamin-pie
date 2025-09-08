# Integration Tests CMakeLists.txt - 细粒度版本
# 集成测试配置 - 每个测试文件独立编译

cmake_minimum_required(VERSION 3.16)

# 包含测试配置
include(../test_config.cmake)

# 设置包含目录
set(INTEGRATION_INCLUDE_DIRS
    ${TEST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# 事件系统测试
if(ENABLE_EVENT_SYSTEM_TEST)
    add_executable(test_event_system test_event_system.cpp)
    target_include_directories(test_event_system PRIVATE ${INTEGRATION_INCLUDE_DIRS})
    target_link_libraries(test_event_system PRIVATE ${TEST_DEPENDENCIES} pthread)
    target_compile_features(test_event_system PRIVATE cxx_std_17)
    enable_testing()
    add_test(NAME EventSystemTest COMMAND test_event_system)
    message(STATUS "Event System Test: ENABLED")
endif()

# 事件压力测试
if(ENABLE_EVENT_STRESS_TEST)
    add_executable(test_event_stress test_event_stress.cpp)
    target_include_directories(test_event_stress PRIVATE ${INTEGRATION_INCLUDE_DIRS})
    target_link_libraries(test_event_stress PRIVATE ${TEST_DEPENDENCIES} pthread)
    target_compile_features(test_event_stress PRIVATE cxx_std_17)
    enable_testing()
    add_test(NAME EventStressTest COMMAND test_event_stress)
    message(STATUS "Event Stress Test: ENABLED")
endif()

# 应用调度器测试
if(ENABLE_APP_SCHEDULER_TEST)
    add_executable(test_app_scheduler test_app_scheduler.cpp)
    target_include_directories(test_app_scheduler PRIVATE ${INTEGRATION_INCLUDE_DIRS})
    target_link_libraries(test_app_scheduler PRIVATE ${TEST_DEPENDENCIES} pthread)
    target_compile_features(test_app_scheduler PRIVATE cxx_std_17)
    enable_testing()
    add_test(NAME AppSchedulerTest COMMAND test_app_scheduler)
    message(STATUS "App Scheduler Test: ENABLED")
endif()

# 设备系统测试
if(ENABLE_DEVICE_SYSTEM_TEST)
    add_executable(test_device_system test_device_system.cpp)
    target_include_directories(test_device_system PRIVATE ${INTEGRATION_INCLUDE_DIRS})
    target_link_libraries(test_device_system PRIVATE ${TEST_DEPENDENCIES} pthread)
    target_compile_features(test_device_system PRIVATE cxx_std_17)
    enable_testing()
    add_test(NAME DeviceSystemTest COMMAND test_device_system)
    message(STATUS "Device System Test: ENABLED")
endif()

# 线程系统测试
if(ENABLE_THREAD_SYSTEM_TEST)
    add_executable(test_thread_system test_thread_system.cpp)
    target_include_directories(test_thread_system PRIVATE ${INTEGRATION_INCLUDE_DIRS})
    target_link_libraries(test_thread_system PRIVATE ${TEST_DEPENDENCIES} pthread)
    target_compile_features(test_thread_system PRIVATE cxx_std_17)
    enable_testing()
    add_test(NAME ThreadSystemTest COMMAND test_thread_system)
    message(STATUS "Thread System Test: ENABLED")
endif()

# 打印集成测试配置
message(STATUS "=== Integration Tests Configuration (Fine-grained) ===")
message(STATUS "Event System Test: ${ENABLE_EVENT_SYSTEM_TEST}")
message(STATUS "Event Stress Test: ${ENABLE_EVENT_STRESS_TEST}")
message(STATUS "App Scheduler Test: ${ENABLE_APP_SCHEDULER_TEST}")
message(STATUS "Device System Test: ${ENABLE_DEVICE_SYSTEM_TEST}")
message(STATUS "Thread System Test: ${ENABLE_THREAD_SYSTEM_TEST}")
message(STATUS "=====================================================")