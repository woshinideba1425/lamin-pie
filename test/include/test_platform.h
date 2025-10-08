/**
 * @file test_platform.h
 * @brief 测试平台抽象层
 * @author LaminPie Team
 * @date 2024
 */

#ifndef LAMINPIE_TEST_PLATFORM_H
#define LAMINPIE_TEST_PLATFORM_H

#include <chrono>
#include <cstdint>

// 平台抽象宏定义
#define PLATFORM_GET_TICK_COUNT() \
    std::chrono::duration_cast<std::chrono::milliseconds>( \
        std::chrono::high_resolution_clock::now().time_since_epoch() \
    ).count()

// 测试工具函数
inline uint32_t get_tick_count_ms() {
    return static_cast<uint32_t>(PLATFORM_GET_TICK_COUNT());
}

inline void test_init() {
    // 测试初始化
}

inline void test_cleanup() {
    // 测试清理
}

inline void print_test_results() {
    // 打印测试结果
}

#endif // LAMINPIE_TEST_PLATFORM_H