#include "test_common.h"
#include "laminpie_thread.h"

class PlatformCompatibilityTest {
public:
    TestResult test_thread_creation_cross_platform() {
        // 测试跨平台线程创建
        bool thread_created = false;
        bool thread_executed = false;
        
        auto result = laminpie_thread_create(
            LAMINPIE_THREAD_PRIO_NORMAL,
            [](void* data) {
                bool* flag = static_cast<bool*>(data);
                *flag = true;
                PLATFORM_DELAY_MS(100);
            },
            4096,
            &thread_executed
        );
        
        TEST_ASSERT(result.is_ok());
        TEST_ASSERT(thread_executed);
        
        return TestResult::kPass;
    }
    
    TestResult test_log_system_cross_platform() {
        // 测试跨平台日志系统
        LP_LOG_INFO("TEST", "Testing cross-platform logging");
        LP_LOG_DEBUG("TEST", "Debug message on %s", PLATFORM_NAME);
        LP_LOG_WARN("TEST", "Warning message");
        LP_LOG_ERROR("TEST", "Error message");
        
        return TestResult::kPass;
    }
    
    TestResult test_conditional_compilation() {
        // 测试条件编译
        #ifdef CONFIG_LAMINPIE_LOG_LEVEL_DEBUG
            TEST_ASSERT(true); // 调试日志已启用
        #else
            TEST_ASSERT(false); // 调试日志未启用
        #endif
        
        #ifdef CONFIG_LAMINPIE_SYSTEM_ENABLE_DEBUG_LOG
            TEST_ASSERT(true); // 系统调试日志已启用
        #else
            TEST_ASSERT(false); // 系统调试日志未启用
        #endif
        
        return TestResult::kPass;
    }
};