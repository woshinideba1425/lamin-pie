/**
 * @file test_main_gtest.cpp
 * @brief LaminPie测试主入口 - Google Test版本
 * @author LaminPie Team
 * @date 2024
 */

#include <gtest/gtest.h>
#include <iostream>
#include <memory>

// 包含测试头文件
#include "test_common_gtest.h"

// 包含所有测试模块
#include "unit/test_log_system_gtest.cpp"
#include "unit/test_platform_compatibility_gtest.cpp"
#include "unit/test_kconfig_gtest.cpp"
#include "integration/test_event_system_gtest.cpp"
#include "integration/test_event_stress_gtest.cpp"
#include "integration/test_app_manager_integration_gtest.cpp"
#include "integration/test_device_system_gtest.cpp"
#include "integration/test_thread_system_gtest.cpp"
#include "performance/test_memory_performance_gtest.cpp"

/**
 * @brief 测试环境设置
 */
class LaminPieTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        std::cout << "Setting up LaminPie test environment..." << std::endl;
        // 初始化测试环境
        test_init();
    }
    
    void TearDown() override {
        std::cout << "Tearing down LaminPie test environment..." << std::endl;
        // 清理测试环境
        test_cleanup();
    }
};

/**
 * @brief 主函数
 */
int main(int argc, char **argv) {
    LogSetModuleLevel("laminpie_event_dispatcher", LOG_INFO);
    // 初始化Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // 添加全局测试环境
    ::testing::AddGlobalTestEnvironment(new LaminPieTestEnvironment);
    
    // 设置测试输出格式
    ::testing::FLAGS_gtest_output = "xml:test_results.xml";
    ::testing::FLAGS_gtest_color = "yes";
    
    std::cout << "==========================================" << std::endl;
    std::cout << "LaminPie Test Suite - Google Test Version" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    // 运行所有测试
    int result = RUN_ALL_TESTS();
    
    std::cout << "==========================================" << std::endl;
    std::cout << "Test execution completed" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    return result;
}
