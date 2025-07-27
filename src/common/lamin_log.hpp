#pragma once

#include <string>
#include <iostream>

#define LAMINPIE_USE_LOG 1

// 定义日志级别的数值常量，供预处理器使用
#define LP_LOG_LEVEL_TRACE 0
#define LP_LOG_LEVEL_DEBUG 1
#define LP_LOG_LEVEL_INFO 2
#define LP_LOG_LEVEL_WARN 3
#define LP_LOG_LEVEL_ERROR 4
#define LP_LOG_LEVEL_NONE 5

#if LAMINPIE_USE_LOG
// 定义不同的日志级别
enum class LogLevel {
    TRACE = LP_LOG_LEVEL_TRACE,
    DEBUG = LP_LOG_LEVEL_DEBUG,
    INFO = LP_LOG_LEVEL_INFO,
    WARN = LP_LOG_LEVEL_WARN,
    ERROR = LP_LOG_LEVEL_ERROR,
    NONE = LP_LOG_LEVEL_NONE
};

namespace laminpie {
namespace log {

// 平台检测
#if defined(ESP_PLATFORM)
    #include "esp_log.h"
    #define PLATFORM_ESP32
#elif defined(__RTTHREAD__)
    #include "rtthread.h"
    #define PLATFORM_RTTHREAD
#else
    #include <cstdio>
    #define PLATFORM_GENERIC
#endif

// 全局日志级别设置
#ifndef LAMINPIE_LOG_LEVEL
    #define LAMINPIE_LOG_LEVEL LogLevel::INFO
    #define LAMINPIE_LOG_LEVEL_VALUE LP_LOG_LEVEL_INFO
#endif

// 确保LAMINPIE_LOG_LEVEL_VALUE总是被定义
#ifndef LAMINPIE_LOG_LEVEL_VALUE
    #if LAMINPIE_LOG_LEVEL == LogLevel::TRACE
        #define LAMINPIE_LOG_LEVEL_VALUE LP_LOG_LEVEL_TRACE
    #elif LAMINPIE_LOG_LEVEL == LogLevel::DEBUG
        #define LAMINPIE_LOG_LEVEL_VALUE LP_LOG_LEVEL_DEBUG
    #elif LAMINPIE_LOG_LEVEL == LogLevel::INFO
        #define LAMINPIE_LOG_LEVEL_VALUE LP_LOG_LEVEL_INFO
    #elif LAMINPIE_LOG_LEVEL == LogLevel::WARN
        #define LAMINPIE_LOG_LEVEL_VALUE LP_LOG_LEVEL_WARN
    #elif LAMINPIE_LOG_LEVEL == LogLevel::ERROR
        #define LAMINPIE_LOG_LEVEL_VALUE LP_LOG_LEVEL_ERROR
    #else
        #define LAMINPIE_LOG_LEVEL_VALUE LP_LOG_LEVEL_NONE
    #endif
#endif

// 元函数-检查是否应该打印该级别的日志
template<LogLevel level>
struct ShouldLog {
    static constexpr bool value = level >= static_cast<LogLevel>(LAMINPIE_LOG_LEVEL);
};

// 声明LogImpl基本模板
template<LogLevel level>
struct LogImpl {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args);
};

#if defined(PLATFORM_GENERIC)
template<>
struct LogImpl<LogLevel::ERROR> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::ERROR>::value) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
            printf("[ERROR][%s] %s\n", tag, buffer);
        }
    }
};

template<>
struct LogImpl<LogLevel::WARN> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::WARN>::value) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
            printf("[WARN][%s] %s\n", tag, buffer);
        }
    }
};

template<>
struct LogImpl<LogLevel::INFO> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::INFO>::value) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
            printf("[INFO][%s] %s\n", tag, buffer);
        }
    }
};

template<>
struct LogImpl<LogLevel::DEBUG> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::DEBUG>::value) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
            printf("[DEBUG][%s] %s\n", tag, buffer);
        }
    }
};

template<>
struct LogImpl<LogLevel::TRACE> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::TRACE>::value) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
            printf("[TRACE][%s] %s\n", tag, buffer);
        }
    }
};
#endif

// ESP32平台特化
#ifdef PLATFORM_ESP32
template<>
struct LogImpl<LogLevel::ERROR> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::ERROR>::value) {
            ESP_LOGE(tag, fmt, std::forward<Args>(args)...);
        }
    }
};

template<>
struct LogImpl<LogLevel::WARN> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::WARN>::value) {
            ESP_LOGW(tag, fmt, std::forward<Args>(args)...);
        }
    }
};

template<>
struct LogImpl<LogLevel::INFO> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::INFO>::value) {
            ESP_LOGI(tag, fmt, std::forward<Args>(args)...);
        }
    }
};

template<>
struct LogImpl<LogLevel::DEBUG> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::DEBUG>::value) {
            ESP_LOGD(tag, fmt, std::forward<Args>(args)...);
        }
    }
};

template<>
struct LogImpl<LogLevel::TRACE> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::TRACE>::value) {
            ESP_LOGV(tag, fmt, std::forward<Args>(args)...);
        }
    }
};
#endif

// RTThread平台特化
#ifdef PLATFORM_RTTHREAD
template<>
struct LogImpl<LogLevel::ERROR> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::ERROR>::value) {
            rt_kprintf("[ERROR][%s] ", tag);
            rt_kprintf(fmt, std::forward<Args>(args)...);
            rt_kprintf("\n");
        }
    }
};

template<>
struct LogImpl<LogLevel::WARN> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::WARN>::value) {
            rt_kprintf("[WARN][%s] ", tag);
            rt_kprintf(fmt, std::forward<Args>(args)...);
            rt_kprintf("\n");
        }
    }
};

template<>
struct LogImpl<LogLevel::INFO> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::INFO>::value) {
            rt_kprintf("[INFO][%s] ", tag);
            rt_kprintf(fmt, std::forward<Args>(args)...);
            rt_kprintf("\n");
        }
    }
};

template<>
struct LogImpl<LogLevel::DEBUG> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::DEBUG>::value) {
            rt_kprintf("[DEBUG][%s] ", tag);
            rt_kprintf(fmt, std::forward<Args>(args)...);
            rt_kprintf("\n");
        }
    }
};

template<>
struct LogImpl<LogLevel::TRACE> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel::TRACE>::value) {
            rt_kprintf("[TRACE][%s] ", tag);
            rt_kprintf(fmt, std::forward<Args>(args)...);
            rt_kprintf("\n");
        }
    }
};
#endif

// 对外的日志API
template<LogLevel level>
struct Logger {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        LogImpl<level>::log(tag, fmt, std::forward<Args>(args)...);
    }
};

// 日志接口宏定义 - 零开销实现
#if LAMINPIE_LOG_LEVEL_VALUE <= LP_LOG_LEVEL_TRACE
#define LP_LOG_TRACE(tag, fmt, ...) laminpie::log::Logger<LogLevel::TRACE>::log(tag, fmt, ##__VA_ARGS__)
#else
#define LP_LOG_TRACE(tag, fmt, ...) ((void)0)
#endif

#if LAMINPIE_LOG_LEVEL_VALUE <= LP_LOG_LEVEL_DEBUG
#define LP_LOG_DEBUG(tag, fmt, ...) laminpie::log::Logger<LogLevel::DEBUG>::log(tag, fmt, ##__VA_ARGS__)
#else
#define LP_LOG_DEBUG(tag, fmt, ...) ((void)0)
#endif

#if LAMINPIE_LOG_LEVEL_VALUE <= LP_LOG_LEVEL_INFO
#define LP_LOG_INFO(tag, fmt, ...) laminpie::log::Logger<LogLevel::INFO>::log(tag, fmt, ##__VA_ARGS__)
#else
#define LP_LOG_INFO(tag, fmt, ...) ((void)0)
#endif

#if LAMINPIE_LOG_LEVEL_VALUE <= LP_LOG_LEVEL_WARN
#define LP_LOG_WARN(tag, fmt, ...) laminpie::log::Logger<LogLevel::WARN>::log(tag, fmt, ##__VA_ARGS__)
#else
#define LP_LOG_WARN(tag, fmt, ...) ((void)0)
#endif

#if LAMINPIE_LOG_LEVEL_VALUE <= LP_LOG_LEVEL_ERROR
#define LP_LOG_ERROR(tag, fmt, ...) laminpie::log::Logger<LogLevel::ERROR>::log(tag, fmt, ##__VA_ARGS__)
#else
#define LP_LOG_ERROR(tag, fmt, ...) ((void)0)
#endif

// 模块输出日志定义，接受TAG和标志位控制是否编译 - 零开销实现
#if LAMINPIE_LOG_LEVEL_VALUE <= LP_LOG_LEVEL_TRACE
#define LP_MOD_LOG_TRACE(tag, enable, fmt, ...) do { if (enable) { laminpie::log::Logger<LogLevel::TRACE>::log(tag, fmt, ##__VA_ARGS__); } } while(0)
#else
#define LP_MOD_LOG_TRACE(tag, enable, fmt, ...) ((void)0)
#endif

#if LAMINPIE_LOG_LEVEL_VALUE <= LP_LOG_LEVEL_DEBUG
#define LP_MOD_LOG_DEBUG(tag, enable, fmt, ...) do { if (enable) { laminpie::log::Logger<LogLevel::DEBUG>::log(tag, fmt, ##__VA_ARGS__); } } while(0)
#else
#define LP_MOD_LOG_DEBUG(tag, enable, fmt, ...) ((void)0)
#endif

#if LAMINPIE_LOG_LEVEL_VALUE <= LP_LOG_LEVEL_INFO
#define LP_MOD_LOG_INFO(tag, enable, fmt, ...) do { if (enable) { laminpie::log::Logger<LogLevel::INFO>::log(tag, fmt, ##__VA_ARGS__); } } while(0)
#else
#define LP_MOD_LOG_INFO(tag, enable, fmt, ...) ((void)0)
#endif

#if LAMINPIE_LOG_LEVEL_VALUE <= LP_LOG_LEVEL_WARN
#define LP_MOD_LOG_WARN(tag, enable, fmt, ...) do { if (enable) { laminpie::log::Logger<LogLevel::WARN>::log(tag, fmt, ##__VA_ARGS__); } } while(0)
#else
#define LP_MOD_LOG_WARN(tag, enable, fmt, ...) ((void)0)
#endif

#if LAMINPIE_LOG_LEVEL_VALUE <= LP_LOG_LEVEL_ERROR
#define LP_MOD_LOG_ERROR(tag, enable, fmt, ...) do { if (enable) { laminpie::log::Logger<LogLevel::ERROR>::log(tag, fmt, ##__VA_ARGS__); } } while(0)
#else
#define LP_MOD_LOG_ERROR(tag, enable, fmt, ...) ((void)0)
#endif

} // namespace log
} // namespace laminpie

#endif