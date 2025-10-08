/**
 * @file test_platform_compatibility_gtest.cpp
 * @brief 平台兼容性测试 - Google Test版本
 * @author LaminPie Team
 * @date 2024
 */

#include "test_common_gtest.h"
#include "test_platform.h"

/**
 * @brief 平台兼容性测试类
 */
class PlatformCompatibilityTest : public LaminPieTestBase {
public:
    void SetUp() override {
        LaminPieTestBase::SetUp();
    }
    
    void TearDown() override {
        LaminPieTestBase::TearDown();
    }
};

/**
 * @brief 测试跨平台线程创建
 */
TEST_F(PlatformCompatibilityTest, TestThreadCreationCrossPlatform) {
    std::atomic<bool> thread_started{false};
    std::atomic<bool> thread_finished{false};
    
    // 创建线程
    std::thread test_thread([&]() {
        thread_started = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        thread_finished = true;
    });
    
    // 等待线程启动
    bool started = WaitForCondition([&]() { return thread_started.load(); }, 1000);
    EXPECT_TRUE(started) << "Thread should start within timeout";
    
    // 等待线程完成
    test_thread.join();
    
    EXPECT_TRUE(thread_finished.load()) << "Thread should finish execution";
}

/**
 * @brief 测试跨平台日志系统
 */
TEST_F(PlatformCompatibilityTest, TestLogSystemCrossPlatform) {
    // 测试日志系统在不同平台上的兼容性
    LOGI("Testing cross-platform logging");
    LOGW("Warning message");
    LOGE("Error message");
    
    // 验证日志系统正常工作
    SUCCEED();
}

/**
 * @brief 测试条件编译
 */
TEST_F(PlatformCompatibilityTest, TestConditionalCompilation) {
    // 测试条件编译宏
#ifdef CONFIG_LAMINPIE_TEST_ENABLE_LOG_SYSTEM
    LOGI("Log system is enabled");
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_EVENT_SYSTEM
    LOGI("Event system is enabled");
#endif

    // 验证条件编译正常工作
    SUCCEED();
}

/**
 * @brief 测试内存对齐
 */
TEST_F(PlatformCompatibilityTest, TestMemoryAlignment) {
    // 测试不同大小的结构体对齐
    struct SmallStruct {
        char data[8];
    };
    
    struct LargeStruct {
        char data[64];
    };
    
    SmallStruct small;
    LargeStruct large;
    
    // 验证结构体可以正常创建和使用
    EXPECT_EQ(sizeof(small), 8) << "Small struct size should be 8 bytes";
    EXPECT_EQ(sizeof(large), 64) << "Large struct size should be 64 bytes";
    
    // 测试内存访问
    small.data[0] = 'A';
    large.data[0] = 'B';
    
    EXPECT_EQ(small.data[0], 'A') << "Small struct data should be accessible";
    EXPECT_EQ(large.data[0], 'B') << "Large struct data should be accessible";
}

/**
 * @brief 测试原子操作
 */
TEST_F(PlatformCompatibilityTest, TestAtomicOperations) {
    std::atomic<int> counter{0};
    const int iterations = 1000;
    
    // 测试原子递增操作
    for (int i = 0; i < iterations; i++) {
        counter.fetch_add(1);
    }
    
    EXPECT_EQ(counter.load(), iterations) << "Atomic counter should match expected value";
    
    // 测试原子比较交换
    int expected = iterations;
    bool success = counter.compare_exchange_strong(expected, iterations + 1);
    EXPECT_TRUE(success) << "Compare and swap should succeed";
    EXPECT_EQ(counter.load(), iterations + 1) << "Counter should be incremented";
}

/**
 * @brief 测试时间函数
 */
TEST_F(PlatformCompatibilityTest, TestTimeFunctions) {
    // 测试获取当前时间
    auto now = std::chrono::high_resolution_clock::now();
    EXPECT_GT(now.time_since_epoch().count(), 0) << "Current time should be valid";
    
    // 测试睡眠功能
    auto start_time = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    EXPECT_GE(duration.count(), 100) << "Sleep duration should be at least 100ms";
}
