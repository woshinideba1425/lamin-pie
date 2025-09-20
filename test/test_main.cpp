/**
 * @file test_main.cpp
 * @brief LaminPie测试主入口 - ESP-IDF平台集成版本
 * @author LaminPie Team
 * @date 2024
 */

#include "test_common.h"
#include <cstdio>
#include <cstring>
#include "sdkconfig.h"


#ifdef CONFIG_LAMINPIE_TEST_ENABLE_LOG_SYSTEM
#include "unit/test_log_system.cpp"
class LogSystemTest;

bool TestLogSystem() {
    LogSystemTest test;
    return test.test_module_logging() == TestResult::kPass &&
           test.test_log_levels() == TestResult::kPass &&
           test.test_log_performance() == TestResult::kPass;
}

#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_EVENT_SYSTEM
#include "integration/test_event_system.cpp"
class EventSystemTest;

bool TestEventSystem() {
    EventSystemTest test;
    return test.test_event_registration() == TestResult::kPass &&
           test.test_event_dispatch() == TestResult::kPass &&
           test.test_async_event_processing() == TestResult::kPass;
}
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_EVENT_STRESS
#include "integration/test_event_stress.cpp"
class EventStressTest;

bool TestEventStress() {
    EventStressTest test;
    return test.test_high_frequency_events() == TestResult::kPass &&
           test.test_concurrent_event_handling() == TestResult::kPass;
}
#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_APP_SCHEDULER
#include "integration/test_app_scheduler.cpp"
class AppSchedulerTest;

bool TestAppScheduler() {
    AppSchedulerTest test;
    return test.test_app_startup() == TestResult::kPass &&
           test.test_app_lifecycle() == TestResult::kPass &&
           test.test_multiple_apps() == TestResult::kPass;
}

#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_DEVICE_SYSTEM
#include "integration/test_device_system.cpp"
class DeviceSystemTest;

bool TestDeviceSystem() {
    DeviceSystemTest test;
    return test.test_device_registration() == TestResult::kPass &&
           test.test_device_discovery() == TestResult::kPass &&
           test.test_device_lifecycle() == TestResult::kPass;
}

#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_THREAD_SYSTEM
#include "integration/test_thread_system.cpp"
class ThreadSystemTest;

bool TestThreadSystem() {
    ThreadSystemTest test;
    return test.test_thread_creation_and_destruction() == TestResult::kPass;
}

#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_PLATFORM_COMPATIBILITY
#include "unit/test_platform_compatibility.cpp"
class PlatformCompatibilityTest;

bool TestPlatformCompatibility() {
    PlatformCompatibilityTest test;
    return test.test_thread_creation_cross_platform() == TestResult::kPass &&
           test.test_log_system_cross_platform() == TestResult::kPass &&
           test.test_conditional_compilation() == TestResult::kPass;
}

#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_KCONFIG
#include "unit/test_kconfig.cpp"
class KConfigTest;

bool TestKConfig() {
    KConfigTest test;
    return test.test_kconfig_loading() == TestResult::kPass &&
           test.test_kconfig_values() == TestResult::kPass;
}

#endif

#ifdef CONFIG_LAMINPIE_TEST_ENABLE_MEMORY_PERFORMANCE
#include "performance/test_memory_performance.cpp"
class MemoryPerformanceTest;

bool TestMemoryPerformance() {
    MemoryPerformanceTest test;
    return test.test_memory_allocation_speed() == TestResult::kPass &&
           test.test_memory_fragmentation() == TestResult::kPass;
}

#endif

namespace laminpie::test {
/**
 * @brief 运行所有启用的测试
 * @return 测试结果：0表示成功，非0表示失败
 */
int RunAllTests() {
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    LP_LOG_INFO("TEST_MAIN", "Starting LaminPie Test Suite...");
    LP_LOG_INFO("TEST_MAIN", "==========================================");
    
    // 日志系统测试
    #ifdef CONFIG_LAMINPIE_TEST_ENABLE_LOG_SYSTEM
    LP_LOG_INFO("TEST_MAIN", "Running Log System Tests...");
    if (TestLogSystem()) {
        LP_LOG_INFO("TEST_MAIN", "✓ Log System Tests PASSED");
        passed_tests++;
    } else {
        LP_LOG_ERROR("TEST_MAIN", "✗ Log System Tests FAILED");
        failed_tests++;
    }
    total_tests++;
    #endif
    
    // 事件系统测试
    #ifdef CONFIG_LAMINPIE_TEST_ENABLE_EVENT_SYSTEM
    LP_LOG_INFO("TEST_MAIN", "Running Event System Tests...");
    if (TestEventSystem()) {
        LP_LOG_INFO("TEST_MAIN", "✓ Event System Tests PASSED");
        passed_tests++;
    } else {
        LP_LOG_ERROR("TEST_MAIN", "✗ Event System Tests FAILED");
        failed_tests++;
    }
    total_tests++;
    #endif
    
    // 事件压力测试
    #ifdef CONFIG_LAMINPIE_TEST_ENABLE_EVENT_STRESS
    LP_LOG_INFO("TEST_MAIN", "Running Event Stress Tests...");
    if (TestEventStress()) {
        LP_LOG_INFO("TEST_MAIN", "✓ Event Stress Tests PASSED");
        passed_tests++;
    } else {
        LP_LOG_ERROR("TEST_MAIN", "✗ Event Stress Tests FAILED");
        failed_tests++;
    }
    total_tests++;
    #endif
    
    // 应用调度器测试
    #ifdef CONFIG_LAMINPIE_TEST_ENABLE_APP_SCHEDULER
    LP_LOG_INFO("TEST_MAIN", "Running App Scheduler Tests...");
    if (TestAppScheduler()) {
        LP_LOG_INFO("TEST_MAIN", "✓ App Scheduler Tests PASSED");
        passed_tests++;
    } else {
        LP_LOG_ERROR("TEST_MAIN", "✗ App Scheduler Tests FAILED");
        failed_tests++;
    }
    total_tests++;
    #endif
    
    // 设备系统测试
    #ifdef CONFIG_LAMINPIE_TEST_ENABLE_DEVICE_SYSTEM
    LP_LOG_INFO("TEST_MAIN", "Running Device System Tests...");
    if (TestDeviceSystem()) {
        LP_LOG_INFO("TEST_MAIN", "✓ Device System Tests PASSED");
        passed_tests++;
    } else {
        LP_LOG_ERROR("TEST_MAIN", "✗ Device System Tests FAILED");
        failed_tests++;
    }
    total_tests++;
    #endif
    
    // 线程系统测试
    #ifdef CONFIG_LAMINPIE_TEST_ENABLE_THREAD_SYSTEM
    LP_LOG_INFO("TEST_MAIN", "Running Thread System Tests...");
    if (TestThreadSystem()) {
        LP_LOG_INFO("TEST_MAIN", "✓ Thread System Tests PASSED");
        passed_tests++;
    } else {
        LP_LOG_ERROR("TEST_MAIN", "✗ Thread System Tests FAILED");
        failed_tests++;
    }
    total_tests++;
    #endif
    
    // 平台兼容性测试
    #ifdef CONFIG_LAMINPIE_TEST_ENABLE_PLATFORM_COMPATIBILITY
    LP_LOG_INFO("TEST_MAIN", "Running Platform Compatibility Tests...");
    if (TestPlatformCompatibility()) {
        LP_LOG_INFO("TEST_MAIN", "✓ Platform Compatibility Tests PASSED");
        passed_tests++;
    } else {
        LP_LOG_ERROR("TEST_MAIN", "✗ Platform Compatibility Tests FAILED");
        failed_tests++;
    }
    total_tests++;
    #endif
    
    // KConfig测试
    #ifdef CONFIG_LAMINPIE_TEST_ENABLE_KCONFIG
    LP_LOG_INFO("TEST_MAIN", "Running KConfig Tests...");
    if (TestKConfig()) {
        LP_LOG_INFO("TEST_MAIN", "✓ KConfig Tests PASSED");
        passed_tests++;
    } else {
        LP_LOG_ERROR("TEST_MAIN", "✗ KConfig Tests FAILED");
        failed_tests++;
    }
    total_tests++;
    #endif
    
    // 内存性能测试
    #ifdef CONFIG_LAMINPIE_TEST_ENABLE_MEMORY_PERFORMANCE
    LP_LOG_INFO("TEST_MAIN", "Running Memory Performance Tests...");
    if (TestMemoryPerformance()) {
        LP_LOG_INFO("TEST_MAIN", "✓ Memory Performance Tests PASSED");
        passed_tests++;
    } else {
        LP_LOG_ERROR("TEST_MAIN", "✗ Memory Performance Tests FAILED");
        failed_tests++;
    }
    total_tests++;
    #endif
    
    // 打印测试结果摘要
    LP_LOG_INFO("TEST_MAIN", "==========================================");
    LP_LOG_INFO("TEST_MAIN", "Test Results Summary:");
    LP_LOG_INFO("TEST_MAIN", "  Total Tests: %d", total_tests);
    LP_LOG_INFO("TEST_MAIN", "  Passed: %d", passed_tests);
    LP_LOG_INFO("TEST_MAIN", "  Failed: %d", failed_tests);
    LP_LOG_INFO("TEST_MAIN", "  Success Rate: %.1f%%", 
                total_tests > 0 ? (float)passed_tests / total_tests * 100.0f : 0.0f);
    
    if (failed_tests == 0) {
        LP_LOG_INFO("TEST_MAIN", "🎉 All tests PASSED!");
        return 0;
    } else {
        LP_LOG_ERROR("TEST_MAIN", "❌ %d test(s) FAILED!", failed_tests);
        return failed_tests;
    }
}

/**
 * @brief 运行特定模块的测试
 * @param module_name 模块名称
 * @return 测试结果：0表示成功，非0表示失败
 */
int RunModuleTest(const char* module_name) {
    if (strcmp(module_name, "log") == 0) {
        #ifdef ENABLE_LOG_TESTS
        return TestLogSystem() ? 0 : 1;
        #else
        LP_LOG_WARN("TEST_MAIN", "Log tests not enabled");
        return 1;
        #endif
    }
    else if (strcmp(module_name, "event") == 0) {
        #ifdef ENABLE_EVENT_SYSTEM_TEST
        return TestEventSystem() ? 0 : 1;
        #else
        LP_LOG_WARN("TEST_MAIN", "Event tests not enabled");
        return 1;
        #endif
    }
    else if (strcmp(module_name, "event_stress") == 0) {
        #ifdef ENABLE_EVENT_STRESS_TEST
        return TestEventStress() ? 0 : 1;
        #else
        LP_LOG_WARN("TEST_MAIN", "Event stress tests not enabled");
        return 1;
        #endif
    }
    else if (strcmp(module_name, "app") == 0) {
        #ifdef ENABLE_APP_SCHEDULER_TEST
        return TestAppScheduler() ? 0 : 1;
        #else
        LP_LOG_WARN("TEST_MAIN", "App tests not enabled");
        return 1;
        #endif
    }
    else if (strcmp(module_name, "device") == 0) {
        #ifdef ENABLE_DEVICE_SYSTEM_TEST
        return TestDeviceSystem() ? 0 : 1;
        #else
        LP_LOG_WARN("TEST_MAIN", "Device tests not enabled");
        return 1;
        #endif
    }
    else if (strcmp(module_name, "thread") == 0) {
        #ifdef ENABLE_THREAD_SYSTEM_TEST
        return TestThreadSystem() ? 0 : 1;
        #else
        LP_LOG_WARN("TEST_MAIN", "Thread tests not enabled");
        return 1;
        #endif
    }
    else if (strcmp(module_name, "platform") == 0) {
        #ifdef ENABLE_PLATFORM_COMPATIBILITY_TEST
        return TestPlatformCompatibility() ? 0 : 1;
        #else
        LP_LOG_WARN("TEST_MAIN", "Platform tests not enabled");
        return 1;
        #endif
    }
    else if (strcmp(module_name, "kconfig") == 0) {
        #ifdef ENABLE_KCONFIG_TEST
        return TestKConfig() ? 0 : 1;
        #else
        LP_LOG_WARN("TEST_MAIN", "KConfig tests not enabled");
        return 1;
        #endif
    }
    else if (strcmp(module_name, "memory") == 0) {
        #ifdef ENABLE_MEMORY_PERFORMANCE_TEST
        return TestMemoryPerformance() ? 0 : 1;
        #else
        LP_LOG_WARN("TEST_MAIN", "Memory tests not enabled");
        return 1;
        #endif
    }
    else {
        LP_LOG_ERROR("TEST_MAIN", "Unknown module: %s", module_name);
        return 1;
    }
}

/**
 * @brief 打印可用的测试模块列表
 */
void PrintAvailableTests() {
    LP_LOG_INFO("TEST_MAIN", "Available test modules:");
    #ifdef ENABLE_LOG_TESTS
    LP_LOG_INFO("TEST_MAIN", "  - log");
    #endif
    #ifdef ENABLE_EVENT_SYSTEM_TEST
    LP_LOG_INFO("TEST_MAIN", "  - event");
    #endif
    #ifdef ENABLE_EVENT_STRESS_TEST
    LP_LOG_INFO("TEST_MAIN", "  - event_stress");
    #endif
    #ifdef ENABLE_APP_SCHEDULER_TEST
    LP_LOG_INFO("TEST_MAIN", "  - app");
    #endif
    #ifdef ENABLE_DEVICE_SYSTEM_TEST
    LP_LOG_INFO("TEST_MAIN", "  - device");
    #endif
    #ifdef ENABLE_THREAD_SYSTEM_TEST
    LP_LOG_INFO("TEST_MAIN", "  - thread");
    #endif
    #ifdef ENABLE_PLATFORM_COMPATIBILITY_TEST
    LP_LOG_INFO("TEST_MAIN", "  - platform");
    #endif
    #ifdef ENABLE_KCONFIG_TEST
    LP_LOG_INFO("TEST_MAIN", "  - kconfig");
    #endif
    #ifdef ENABLE_MEMORY_PERFORMANCE_TEST
    LP_LOG_INFO("TEST_MAIN", "  - memory");
    #endif
}

} // namespace laminpie::test

// C接口函数，供main.cpp调用
extern "C" {
    int laminpie_run_all_tests() {
        return laminpie::test::RunAllTests();
    }
    
    int laminpie_run_module_test(const char* module_name) {
        return laminpie::test::RunModuleTest(module_name);
    }
    
    void laminpie_print_available_tests() {
        laminpie::test::PrintAvailableTests();
    }
}