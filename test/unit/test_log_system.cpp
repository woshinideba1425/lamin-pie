#include "test_common.h"
#include "test_platform.h"
#include "laminpie_log.hpp"
#include "src/core/systems/laminpie_system_internal.h"

class LogSystemTest {
public:
    TestResult test_log_levels() {
        // 测试日志级别
        LP_LOG_TRACE("TEST", "Trace message");
        LP_LOG_DEBUG("TEST", "Debug message");
        LP_LOG_INFO("TEST", "Info message");
        LP_LOG_WARN("TEST", "Warning message");
        LP_LOG_ERROR("TEST", "Error message");
        
        return TestResult::kPass;
    }
    
    TestResult test_module_logging() {
        // 测试模块日志
        SYSTEM_APP_LOG_INFO("Module log test");
        SYSTEM_EVENT_LOG_DEBUG("Event module log test");
        SYSTEM_MANAGER_LOG_WARN("Manager module log test");
        SYSTEM_CORE_LOG_ERROR("Core module log test");
        
        return TestResult::kPass;
    }
    
    TestResult test_log_performance() {
        // 测试日志性能
        auto start_time = PLATFORM_GET_TICK_COUNT();
        
        for (int i = 0; i < 1000; i++) {
            LP_LOG_INFO("PERF", "Performance test message %d", i);
        }
        
        auto end_time = PLATFORM_GET_TICK_COUNT();
        auto duration = end_time - start_time;
        
        // 1000条日志应该在100ms内完成
        TEST_ASSERT(duration < 100);
        
        return TestResult::kPass;
    }
};