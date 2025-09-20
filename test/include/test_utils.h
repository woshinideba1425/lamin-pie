#ifndef LAMINPIE_TEST_UTILS_H
#define LAMINPIE_TEST_UTILS_H

#include "test_common.h"
#include <vector>
#include <string>
#include "laminpie_log.hpp"

class TestUtils {
public:
    // 内存测试工具
    static bool check_memory_leak(void);
    static size_t get_allocated_memory(void);
    static void print_memory_stats(void);
    
    // 性能测试工具
    static uint32_t measure_execution_time(TestFunction func);
    static void stress_test(TestFunction func, int iterations);
    
    // 字符串工具
    static std::string format_test_name(const char* module, const char* test);
    static void print_test_header(const char* test_name);
    static void print_test_footer(TestResult result, uint32_t duration_ms);
    
    // 数据生成工具
    static std::vector<uint8_t> generate_test_data(size_t size);
    static std::string generate_random_string(size_t length);
    
    // 系统信息工具
    static void print_system_info(void);
    static bool check_system_resources(void);
};

#endif // LAMINPIE_TEST_UTILS_H