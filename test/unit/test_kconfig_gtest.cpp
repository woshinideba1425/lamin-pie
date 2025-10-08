/**
 * @file test_kconfig_gtest.cpp
 * @brief KConfig测试 - Google Test版本
 * @author LaminPie Team
 * @date 2024
 */

#include "test_common_gtest.h"
#include "sdkconfig.h"

/**
 * @brief KConfig测试类
 */
class KConfigTest : public LaminPieTestBase {
public:
    void SetUp() override {
        LaminPieTestBase::SetUp();
    }
    
    void TearDown() override {
        LaminPieTestBase::TearDown();
    }
};

/**
 * @brief 测试KConfig加载
 */
TEST_F(KConfigTest, TestKConfigLoading) {
    // 测试KConfig宏是否正确定义
#ifdef CONFIG_LAMINPIE_TEST_ENABLE_LOG_SYSTEM
    LOGI( "Log system test is enabled");
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_EVENT_SYSTEM
    LOGI( "Event system test is enabled");
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_EVENT_STRESS
    LOGI( "Event stress test is enabled");
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_APP_SCHEDULER
    LOGI( "App scheduler test is enabled");
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_DEVICE_SYSTEM
    LOGI( "Device system test is enabled");
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_THREAD_SYSTEM
    LOGI( "Thread system test is enabled");
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_PLATFORM_COMPATIBILITY
    LOGI( "Platform compatibility test is enabled");
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_MEMORY_PERFORMANCE
    LOGI( "Memory performance test is enabled");
#endif

    SUCCEED();
}

/**
 * @brief 测试KConfig值
 */
TEST_F(KConfigTest, TestKConfigValues) {
    // 测试一些基本的KConfig值
    EXPECT_GT(TEST_MAX_ITERATIONS, 0) << "TEST_MAX_ITERATIONS should be positive";
    EXPECT_GT(TEST_TIMEOUT_MS, 0) << "TEST_TIMEOUT_MS should be positive";
    EXPECT_GT(TEST_STACK_SIZE, 0) << "TEST_STACK_SIZE should be positive";
    
    // 测试KConfig宏的值
    LOGI( "TEST_MAX_ITERATIONS: %d", TEST_MAX_ITERATIONS);
    LOGI( "TEST_TIMEOUT_MS: %d", TEST_TIMEOUT_MS);
    LOGI( "TEST_STACK_SIZE: %d", TEST_STACK_SIZE);
    
    SUCCEED();
}

/**
 * @brief 测试条件编译功能
 */
TEST_F(KConfigTest, TestConditionalCompilation) {
    int enabled_tests = 0;
    
    // 统计启用的测试模块
#ifdef CONFIG_LAMINPIE_TEST_ENABLE_LOG_SYSTEM
    enabled_tests++;
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_EVENT_SYSTEM
    enabled_tests++;
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_EVENT_STRESS
    enabled_tests++;
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_APP_SCHEDULER
    enabled_tests++;
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_DEVICE_SYSTEM
    enabled_tests++;
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_THREAD_SYSTEM
    enabled_tests++;
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_PLATFORM_COMPATIBILITY
    enabled_tests++;
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_MEMORY_PERFORMANCE
    enabled_tests++;
#endif

    LOGI( "Total enabled test modules: %d", enabled_tests);
    
    // 至少应该有一个测试模块被启用
    EXPECT_GT(enabled_tests, 0) << "At least one test module should be enabled";
}
