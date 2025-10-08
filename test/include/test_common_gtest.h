/**
 * @file test_common_gtest.h
 * @brief LaminPie测试通用头文件 - Google Test版本
 * @author LaminPie Team
 * @date 2024
 */

#ifndef LAMINPIE_TEST_COMMON_GTEST_H
#define LAMINPIE_TEST_COMMON_GTEST_H

#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <vector>
#include <memory>
#include <string>

// 包含项目头文件
#include "test_platform.h"
#include "laminpie_log.hpp"

// 测试配置
#define TEST_MAX_ITERATIONS 1000
#define TEST_TIMEOUT_MS 5000
#define TEST_STACK_SIZE 4096

// 测试统计结构
struct TestStats {
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    int skipped_tests = 0;
    uint32_t total_time_ms = 0;
};

// 全局测试统计
extern TestStats g_test_stats;

// 测试工具函数
void test_init(void);
void test_cleanup(void);
void print_test_results(void);
uint32_t get_tick_count_ms(void);

// Google Test辅助宏
#define EXPECT_LOG_LEVEL(level) \
    do { \
        LOGI("Testing " #level " level"); \
    } while(0)

#define EXPECT_PERFORMANCE_TEST(name, iterations, max_time_ms) \
    do { \
        auto start_time = std::chrono::high_resolution_clock::now(); \
        for (int i = 0; i < iterations; i++) { \
            name(); \
        } \
        auto end_time = std::chrono::high_resolution_clock::now(); \
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time); \
        EXPECT_LT(duration.count(), max_time_ms) << "Performance test " #name " took too long"; \
    } while(0)

// 测试基类
class LaminPieTestBase : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前的设置
        test_init();
    }
    
    void TearDown() override {
        // 每个测试后的清理
        test_cleanup();
    }
    
    // 等待条件满足或超时
    template<typename Predicate>
    bool WaitForCondition(Predicate pred, int timeout_ms = TEST_TIMEOUT_MS) {
        auto start_time = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::high_resolution_clock::now() - start_time).count() < timeout_ms) {
            if (pred()) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return false;
    }
    
    // 性能测试辅助函数
    template<typename Func>
    uint32_t MeasureExecutionTime(Func func, int iterations = 100) {
        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            func();
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    }
};

// 集成测试基类
class LaminPieIntegrationTest : public LaminPieTestBase {
protected:
    void SetUp() override {
        LaminPieTestBase::SetUp();
        // 集成测试特定的设置
    }
    
    void TearDown() override {
        // 集成测试特定的清理
        LaminPieTestBase::TearDown();
    }
};

// 性能测试基类
class LaminPiePerformanceTest : public LaminPieTestBase {
protected:
    void SetUp() override {
        LaminPieTestBase::SetUp();
        // 性能测试特定的设置
    }
    
    void TearDown() override {
        // 性能测试特定的清理
        LaminPieTestBase::TearDown();
    }
    
    // 性能基准测试
    void BenchmarkTest(const std::string& test_name, std::function<void()> test_func, 
                      int iterations = 1000, uint32_t max_time_us = 100000) {
        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            test_func();
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        std::cout << "Benchmark [" << test_name << "]: " 
                  << duration.count() << "μs for " << iterations << " iterations" << std::endl;
        
        EXPECT_LT(duration.count(), max_time_us) 
            << "Performance test " << test_name << " exceeded time limit";
    }
};

#endif // LAMINPIE_TEST_COMMON_GTEST_H
