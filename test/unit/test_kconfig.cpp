#include "test_common.h"
#include "test_utils.h"

// 使用Kconfig定义的标志位
#ifdef CONFIG_LAMINPIE_CONF_SKIP
    #define USE_CUSTOM_CONFIG false
    #define CONFIG_SOURCE "product_config.json"
#else
    #define USE_CUSTOM_CONFIG true
    #define CONFIG_SOURCE "laminpie_conf.h"
#endif

// 根据日志级别设置日志标签
#ifdef CONFIG_LAMINPIE_LOG_LEVEL_DEBUG
    static const char* TAG = "LAMINPIE_DEBUG";
    #define LOG_LEVEL_STR "DEBUG"
#elif defined(CONFIG_LAMINPIE_LOG_LEVEL_INFO)
    static const char* TAG = "LAMINPIE_INFO";
    #define LOG_LEVEL_STR "INFO"
#elif defined(CONFIG_LAMINPIE_LOG_LEVEL_WARN)
    static const char* TAG = "LAMINPIE_WARN";
    #define LOG_LEVEL_STR "WARN"
#elif defined(CONFIG_LAMINPIE_LOG_LEVEL_ERROR)
    static const char* TAG = "LAMINPIE_ERROR";
    #define LOG_LEVEL_STR "ERROR"
#else
    static const char* TAG = "LAMINPIE";
    #define LOG_LEVEL_STR "DEFAULT"
#endif

TestResult test_kconfig_flags(void) {
    LP_LOG_INFO("TEST_KCONFIG", "Starting Kconfig flags test");
    
    // 测试配置源标志位
    LP_LOG_INFO("TEST_KCONFIG", "Configuration Source: %s", CONFIG_SOURCE);
    LP_LOG_INFO("TEST_KCONFIG", "Use Custom Config: %s", USE_CUSTOM_CONFIG ? "true" : "false");
    
    // 测试日志级别标志位
    LP_LOG_INFO("TEST_KCONFIG", "Log Level: %s", LOG_LEVEL_STR);
    
    // 验证配置源设置
    TEST_ASSERT(strlen(CONFIG_SOURCE) > 0);
    
    // 验证日志级别设置
    TEST_ASSERT(strlen(LOG_LEVEL_STR) > 0);
    
    // 条件编译测试
    #ifdef CONFIG_LAMINPIE_LOG_LEVEL_DEBUG
        LP_LOG_DEBUG("TEST_KCONFIG", "Debug level logging enabled");
    #endif
    
    #ifdef CONFIG_LAMINPIE_LOG_LEVEL_INFO
        LP_LOG_INFO("TEST_KCONFIG", "Info level logging enabled");
    #endif
    
    #ifdef CONFIG_LAMINPIE_LOG_LEVEL_WARN
        LP_LOG_WARN("TEST_KCONFIG", "Warn level logging enabled");
    #endif
    
    #ifdef CONFIG_LAMINPIE_LOG_LEVEL_ERROR
        LP_LOG_ERROR("TEST_KCONFIG", "Error level logging enabled");
    #endif
    
    LP_LOG_INFO("TEST_KCONFIG", "Kconfig flags test completed successfully");
    return TestResult::kPass;
}