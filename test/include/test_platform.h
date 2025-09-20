#ifndef LAMINPIE_TEST_PLATFORM_H
#define LAMINPIE_TEST_PLATFORM_H

#include <chrono>
#include <thread>

// 平台抽象宏定义
#define PLATFORM_GET_TICK_COUNT() std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()
#define PLATFORM_DELAY_MS(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms))

// 测试平台配置
#define TEST_PLATFORM_NAME "ESP-IDF"
#define TEST_MAX_THREADS 8
#define TEST_STACK_SIZE 4096

#endif // LAMINPIE_TEST_PLATFORM_H


