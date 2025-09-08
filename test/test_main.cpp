#include "test_common.h"
#include "test_utils.h"
#include "test_config.h"

// 包含所有测试模块
#include "unit/test_kconfig.h"
#include "unit/test_log_system.h"
#include "unit/test_conditional_compilation.h"
#include "unit/test_system_info.h"
#include "integration/test_event_system.h"
#include "integration/test_event_stress.h"
#include "integration/test_device_system.h"
#include "integration/test_app_scheduler.h"
#include "integration/test_thread_system.h"
#include "performance/test_memory_performance.h"
#include "performance/test_cpu_performance.h"
#include "performance/test_io_performance.h"

// 全局测试统计
TestStats g_test_stats = {0};

// 测试用例数组
static const TestCase unit_tests[] = {
    {"test_kconfig_flags", test_kconfig_flags, "Test Kconfig flags functionality", true},
    {"test_laminpie_log", test_laminpie_log, "Test laminpie log system", true},
    {"test_conditional_compilation", test_conditional_compilation, "Test conditional compilation", true},
    {"test_system_info", test_system_info, "Test system information", true},
    {nullptr, nullptr, nullptr, false} // 结束标记
};

static const TestCase integration_tests[] = {
    {"test_event_system", test_event_system, "Test event system functionality", true},
    {"test_event_stress", test_event_stress, "Test event system stress", true},
    {"test_device_system", test_device_system, "Test device system", true},
    {"test_app_scheduler", test_app_scheduler, "Test app scheduler", true},
    {"test_thread_system", test_thread_system, "Test thread system", true},
    {nullptr, nullptr, nullptr, false} // 结束标记
};

static const TestCase performance_tests[] = {
    {"test_memory_performance", test_memory_performance, "Test memory performance", true},
    {"test_cpu_performance", test_cpu_performance, "Test CPU performance", true},
    {"test_io_performance", test_io_performance, "Test I/O performance", true},
    {nullptr, nullptr, nullptr, false} // 结束标记
};

void test_init(void) {
    printf("\n========================================\n");
    printf("Laminpie Test Suite Initialization\n");
    printf("Platform: %s\n", PLATFORM_NAME);
    printf("========================================\n");
    
    // 初始化测试统计
    memset(&g_test_stats, 0, sizeof(TestStats));
    
    // 打印系统信息
    TestUtils::print_system_info();
    
    LP_LOG_INFO("TEST", "Test suite initialized on %s", PLATFORM_NAME);
}

void test_cleanup(void) {
    printf("\n========================================\n");
    printf("Laminpie Test Suite Cleanup\n");
    printf("========================================\n");
    
    // 打印最终统计
    print_test_results();
    
    LP_LOG_INFO("TEST", "Test suite cleanup completed");
}

TestResult run_test_case(const TestCase* test_case) {
    if (!test_case || !test_case->enabled) {
        return TestResult::kSkip;
    }
    
    TestUtils::print_test_header(test_case->name);
    
    uint32_t start_time = PLATFORM_GET_TICK_COUNT();
    TestResult result = test_case->function();
    uint32_t duration = PLATFORM_GET_TICK_COUNT() - start_time;
    
    TestUtils::print_test_footer(result, duration);
    
    // 更新统计
    g_test_stats.total_tests++;
    g_test_stats.total_time_ms += duration;
    
    switch (result) {
        case TestResult::kPass:
            g_test_stats.passed_tests++;
            break;
        case TestResult::kFail:
            g_test_stats.failed_tests++;
            break;
        case TestResult::kSkip:
            g_test_stats.skipped_tests++;
            break;
        default:
            break;
    }
    
    return result;
}

void print_test_results(void) {
    printf("\n========================================\n");
    printf("Test Results Summary\n");
    printf("========================================\n");
    printf("Total Tests: %d\n", g_test_stats.total_tests);
    printf("Passed: %d\n", g_test_stats.passed_tests);
    printf("Failed: %d\n", g_test_stats.failed_tests);
    printf("Skipped: %d\n", g_test_stats.skipped_tests);
    printf("Total Time: %lu ms\n", g_test_stats.total_time_ms);
    printf("Success Rate: %.1f%%\n", 
           g_test_stats.total_tests > 0 ? 
           (float)g_test_stats.passed_tests / g_test_stats.total_tests * 100.0f : 0.0f);
    printf("========================================\n");
}

uint32_t get_tick_count_ms(void) {
    return PLATFORM_GET_TICK_COUNT();
}

// 主函数 - 根据平台使用不同的入口点
#if defined(LAMINPIE_PLATFORM_ESP_IDF)
extern "C" void app_main(void) {
#else
int main(void) {
#endif
    test_init();
    
    // 运行单元测试
    #if ENABLE_UNIT_TESTS
    printf("\n--- Running Unit Tests ---\n");
    for (const TestCase* test = unit_tests; test->name; test++) {
        run_test_case(test);
        PLATFORM_DELAY_MS(100); // 短暂延迟
    }
    #endif
    
    // 运行集成测试
    #if ENABLE_INTEGRATION_TESTS
    printf("\n--- Running Integration Tests ---\n");
    for (const TestCase* test = integration_tests; test->name; test++) {
        run_test_case(test);
        PLATFORM_DELAY_MS(100);
    }
    #endif
    
    // 运行性能测试
    #if ENABLE_PERFORMANCE_TESTS
    printf("\n--- Running Performance Tests ---\n");
    for (const TestCase* test = performance_tests; test->name; test++) {
        run_test_case(test);
        PLATFORM_DELAY_MS(100);
    }
    #endif
    
    test_cleanup();
    
    #if defined(LAMINPIE_PLATFORM_ESP_IDF)
    // ESP-IDF持续运行
    int counter = 0;
    while (1) {
        LP_LOG_INFO("TEST", "Test suite running - Counter: %d, Free heap: %lu bytes", 
                    counter++, PLATFORM_GET_FREE_HEAP());
        PLATFORM_DELAY_MS(10000); // 每10秒输出一次
    }
    #else
    // 其他平台正常退出
    return 0;
    #endif
}