#pragma once

#include <string>
#include <iostream>


#if LAMINPIE_USE_LOG
// 定义不同的日志级别
enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    NONE = 5
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
#endif

// 元函数-检查是否应该打印该级别的日志
template<LogLevel level>
struct ShouldLog {
    static constexpr bool value = level >= static_cast<LogLevel>(LAMINPIE_LOG_LEVEL);
};

#if defined (PLATFORM_GENERIC)
template<>
struct LogImpl<LogLevel::ERROR> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel level>::value) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
            printf("[ERROR][%s] %s\n", tag, buffer);
        }
    };
};

struct LogImpl<LogLevel::WARN> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel level>::value) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
            printf("[WARN][%s] %s\n", tag, buffer);
        }
    };
};

template<>
struct LogImpl<LogLevel::INFO> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel level>::value) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
            printf("[INFO][%s] %s\n", tag, buffer);
        }
    };
};

template<>
struct LogImpl<LogLevel::DEBUG> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel level>::value) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
            printf("[DEBUG][%s] %s\n", tag, buffer);
        }
    };
};

template<>
struct LogImpl<LogLevel::TRACE> {
    template<typename... Args>
    static void log(const char* tag, const char* fmt, Args&&... args) {
        if constexpr (ShouldLog<LogLevel level>::value) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), fmt, std::forward<Args>(args)...);
            printf("[TRACE][%s] %s\n", tag, buffer);
        }
    };
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

// 日志接口宏定义
#define LP_LOG_TRACE(tag, fmt, ...) LAMINPIE::LOG::Logger<LogLevel::TRACE>::log(tag, fmt, ##__VA_ARGS__)
#define LP_LOG_DEBUG(tag, fmt, ...) LAMINPIE::LOG::Logger<LogLevel::DEBUG>::log(tag, fmt, ##__VA_ARGS__)
#define LP_LOG_INFO(tag, fmt, ...) LAMINPIE::LOG::Logger<LogLevel::INFO>::log(tag, fmt, ##__VA_ARGS__)
#define LP_LOG_WARN(tag, fmt, ...) LAMINPIE::LOG::Logger<LogLevel::WARN>::log(tag, fmt, ##__VA_ARGS__)
#define LP_LOG_ERROR(tag, fmt, ...) LAMINPIE::LOG::Logger<LogLevel::ERROR>::log(tag, fmt, ##__VA_ARGS__)

} // namespace LOG
} // namespace LAMINPIE

#endif