# Unit Tests CMakeLists.txt - 细粒度版本
# 单元测试配置 - 每个测试文件独立编译

cmake_minimum_required(VERSION 3.16)

# 包含测试配置
include(../test_config.cmake)

# 设置包含目录
set(UNIT_INCLUDE_DIRS
    ${TEST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# 日志系统测试
if(ENABLE_LOG_TESTS AND ENABLE_LOG_UNIT_TESTS)
    add_executable(test_log_system test_log_system.cpp)
    target_include_directories(test_log_system PRIVATE ${UNIT_INCLUDE_DIRS})
    target_link_libraries(test_log_system PRIVATE ${TEST_DEPENDENCIES} pthread)
    target_compile_features(test_log_system PRIVATE cxx_std_17)
    enable_testing()
    add_test(NAME LogSystemTest COMMAND test_log_system)
    message(STATUS "Log System Test: ENABLED")
endif()

# 平台兼容性测试
if(ENABLE_PLATFORM_COMPATIBILITY_TEST)
    add_executable(test_platform_compatibility test_platform_compatibility.cpp)
    target_include_directories(test_platform_compatibility PRIVATE ${UNIT_INCLUDE_DIRS})
    target_link_libraries(test_platform_compatibility PRIVATE ${TEST_DEPENDENCIES} pthread)
    target_compile_features(test_platform_compatibility PRIVATE cxx_std_17)
    enable_testing()
    add_test(NAME PlatformCompatibilityTest COMMAND test_platform_compatibility)
    message(STATUS "Platform Compatibility Test: ENABLED")
endif()

# KConfig测试
if(ENABLE_KCONFIG_TEST)
    add_executable(test_kconfig test_kconfig.cpp)
    target_include_directories(test_kconfig PRIVATE ${UNIT_INCLUDE_DIRS})
    target_link_libraries(test_kconfig PRIVATE ${TEST_DEPENDENCIES} pthread)
    target_compile_features(test_kconfig PRIVATE cxx_std_17)
    enable_testing()
    add_test(NAME KConfigTest COMMAND test_kconfig)
    message(STATUS "KConfig Test: ENABLED")
endif()

# 打印单元测试配置
message(STATUS "=== Unit Tests Configuration (Fine-grained) ===")
message(STATUS "Log System Test: ${ENABLE_LOG_TESTS}")
message(STATUS "Platform Compatibility Test: ${ENABLE_PLATFORM_COMPATIBILITY_TEST}")
message(STATUS "KConfig Test: ${ENABLE_KCONFIG_TEST}")
message(STATUS "=============================================")