#ifndef LAMINPIE_TEST_COMMON_H
#define LAMINPIE_TEST_COMMON_H

#include "test_platform.h"
#include <assert.h>
#include "laminpie_log.hpp"

// 测试配置
#define TEST_MAX_ITERATIONS 1000
#define TEST_TIMEOUT_MS 5000
#define TEST_STACK_SIZE 4096

// 测试结果枚举
enum class TestResult {
    kPass,
    kFail,
    kSkip,
    kTimeout
};

// 测试统计结构
struct TestStats {
    int total_tests;
    int passed_tests;
    int failed_tests;
    int skipped_tests;
    uint32_t total_time_ms;
};

// 测试宏定义
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            LP_LOG_ERROR("TEST", "Assertion failed: %s", #condition); \
            return TestResult::kFail; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            LP_LOG_ERROR("TEST", "Expected %d, got %d", (expected), (actual)); \
            return TestResult::kFail; \
        } \
    } while(0)

#define TEST_ASSERT_STR_EQUAL(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            LP_LOG_ERROR("TEST", "Expected '%s', got '%s'", (expected), (actual)); \
            return TestResult::kFail; \
        } \
    } while(0)

// 测试函数类型定义
typedef TestResult (*TestFunction)(void);

// 测试用例结构
struct TestCase {
    const char* name;
    TestFunction function;
    const char* description;
    bool enabled;
};

// 全局测试统计
extern TestStats g_test_stats;

// 测试工具函数
void test_init(void);
void test_cleanup(void);
TestResult run_test_case(const TestCase* test_case);
void print_test_results(void);
uint32_t get_tick_count_ms(void);

#endif // LAMINPIE_TEST_COMMON_H