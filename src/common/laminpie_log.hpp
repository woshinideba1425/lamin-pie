#pragma once

#include <stdio.h>
#include <stdarg.h>

#if defined(ESP_PLATFORM)
    #include "esp_log.h"
    #define PLATFORM_ESP32
#elif defined(__RTTHREAD__)
    #include "rtthread.h"
    #define PLATFORM_RTTHREAD
#else
    #define PLATFORM_GENERIC
    #include <iostream>
#endif

// 日志级别定义
#define LP_LOG_LEVEL_TRACE 0
#define LP_LOG_LEVEL_DEBUG 1
#define LP_LOG_LEVEL_INFO  2
#define LP_LOG_LEVEL_WARN  3
#define LP_LOG_LEVEL_ERROR 4
#define LP_LOG_LEVEL_NONE  5

// 默认日志级别
#ifndef LP_LOG_LEVEL
#define LP_LOG_LEVEL LP_LOG_LEVEL_INFO
#endif

#if defined(PLATFORM_ESP32)
    #if LP_LOG_LEVEL <= LP_LOG_LEVEL_TRACE
    #define LP_LOG_TRACE(tag, fmt, ...) ESP_LOGV(tag, fmt, ##__VA_ARGS__)
    #else
    #define LP_LOG_TRACE(tag, fmt, ...) ((void)0)
    #endif

    #if LP_LOG_LEVEL <= LP_LOG_LEVEL_DEBUG
    #define LP_LOG_DEBUG(tag, fmt, ...) ESP_LOGD(tag, fmt, ##__VA_ARGS__)
    #else
    #define LP_LOG_DEBUG(tag, fmt, ...) ((void)0)
    #endif

    #if LP_LOG_LEVEL <= LP_LOG_LEVEL_INFO
    #define LP_LOG_INFO(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
    #else
    #define LP_LOG_INFO(tag, fmt, ...) ((void)0)
    #endif

    #if LP_LOG_LEVEL <= LP_LOG_LEVEL_WARN
    #define LP_LOG_WARN(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
    #else
    #define LP_LOG_WARN(tag, fmt, ...) ((void)0)
    #endif

    #if LP_LOG_LEVEL <= LP_LOG_LEVEL_ERROR
    #define LP_LOG_ERROR(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)
    #else
    #define LP_LOG_ERROR(tag, fmt, ...) ((void)0)
    #endif

#elif defined(PLATFORM_GENERIC)
    #define LP_LOG_TRACE(tag, fmt, ...) printf("[TRACE][%s] " fmt "\n", tag, ##__VA_ARGS__)
    #define LP_LOG_DEBUG(tag, fmt, ...) printf("[DEBUG][%s] " fmt "\n", tag, ##__VA_ARGS__)
    #define LP_LOG_INFO(tag, fmt, ...) printf("[INFO][%s] " fmt "\n", tag, ##__VA_ARGS__)
    #define LP_LOG_WARN(tag, fmt, ...) printf("[WARN][%s] " fmt "\n", tag, ##__VA_ARGS__)
    #define LP_LOG_ERROR(tag, fmt, ...) printf("[ERROR][%s] " fmt "\n", tag, ##__VA_ARGS__)

#elif defined(PLATFORM_RTTHREAD)
    #define LP_LOG_TRACE(tag, fmt, ...) rt_kprintf("[TRACE][%s] " fmt "\n", tag, ##__VA_ARGS__)
    #define LP_LOG_DEBUG(tag, fmt, ...) rt_kprintf("[DEBUG][%s] " fmt "\n", tag, ##__VA_ARGS__)
    #define LP_LOG_INFO(tag, fmt, ...) rt_kprintf("[INFO][%s] " fmt "\n", tag, ##__VA_ARGS__)
    #define LP_LOG_WARN(tag, fmt, ...) rt_kprintf("[WARN][%s] " fmt "\n", tag, ##__VA_ARGS__)
    #define LP_LOG_ERROR(tag, fmt, ...) rt_kprintf("[ERROR][%s] " fmt "\n", tag, ##__VA_ARGS__)
#endif

// 模块日志宏（带开关控制）
#if LP_LOG_LEVEL <= LP_LOG_LEVEL_TRACE
#define LP_MOD_LOG_TRACE(tag, enable, fmt, ...) do { if (enable) LP_LOG_TRACE(tag, fmt, ##__VA_ARGS__); } while(0)
#else
#define LP_MOD_LOG_TRACE(tag, enable, fmt, ...) ((void)0)
#endif

#if LP_LOG_LEVEL <= LP_LOG_LEVEL_DEBUG
#define LP_MOD_LOG_DEBUG(tag, enable, fmt, ...) do { if (enable) LP_LOG_DEBUG(tag, fmt, ##__VA_ARGS__); } while(0)
#else
#define LP_MOD_LOG_DEBUG(tag, enable, fmt, ...) ((void)0)
#endif

#if LP_LOG_LEVEL <= LP_LOG_LEVEL_INFO
#define LP_MOD_LOG_INFO(tag, enable, fmt, ...) do { if (enable) LP_LOG_INFO(tag, fmt, ##__VA_ARGS__); } while(0)
#else
#define LP_MOD_LOG_INFO(tag, enable, fmt, ...) ((void)0)
#endif

#if LP_LOG_LEVEL <= LP_LOG_LEVEL_WARN
#define LP_MOD_LOG_WARN(tag, enable, fmt, ...) do { if (enable) LP_LOG_WARN(tag, fmt, ##__VA_ARGS__); } while(0)
#else
#define LP_MOD_LOG_WARN(tag, enable, fmt, ...) ((void)0)
#endif

#if LP_LOG_LEVEL <= LP_LOG_LEVEL_ERROR
#define LP_MOD_LOG_ERROR(tag, enable, fmt, ...) do { if (enable) LP_LOG_ERROR(tag, fmt, ##__VA_ARGS__); } while(0)
#else
#define LP_MOD_LOG_ERROR(tag, enable, fmt, ...) ((void)0)
#endif

#ifdef CONFIG_LAMINPIE_LOG_LEVEL_DEBUG
#undef LP_LOG_LEVEL
#define LP_LOG_LEVEL LP_LOG_LEVEL_DEBUG
#elif defined(CONFIG_LAMINPIE_LOG_LEVEL_INFO)
#undef LP_LOG_LEVEL
#define LP_LOG_LEVEL LP_LOG_LEVEL_INFO
#elif defined(CONFIG_LAMINPIE_LOG_LEVEL_WARN)
#undef LP_LOG_LEVEL
#define LP_LOG_LEVEL LP_LOG_LEVEL_WARN
#elif defined(CONFIG_LAMINPIE_LOG_LEVEL_ERROR)
#undef LP_LOG_LEVEL
#define LP_LOG_LEVEL LP_LOG_LEVEL_ERROR
#endif 

#ifdef __cplusplus
namespace laminpie::utils {
    using LogFuncType = void (*)(const char* fmt, ...);
    
    inline void DefaultErrorLog(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        
        #if defined(PLATFORM_ESP32)
            char buffer[256];
            vsnprintf(buffer, sizeof(buffer), fmt, args);
            ESP_LOGE("LAMINPIE", "%s", buffer);
        #elif defined(PLATFORM_GENERIC)
            printf("[ERROR][LAMINPIE] ");
            vprintf(fmt, args);
            printf("\n");
        #elif defined(PLATFORM_RTTHREAD)
            rt_kprintf("[ERROR][LAMINPIE] ");
            rt_vprintf(fmt, args);
            rt_kprintf("\n");
        #endif
        
        va_end(args);
    }

    /**
     * @brief Universal value check and return
     */
    template<typename T, typename RetT>
    inline RetT CheckValueAndReturn(T value, T min, T max, RetT returnValue, LogFuncType logFunc, const char* format, ...) {
        if (!(value >= min && value <= max)) {
            
            logFunc("%s: value %d not in range [%d, %d]", format, value, min, max);
            return returnValue;
        }
        return true;
    }

    /**
     * @brief Universal null check and return
     */
    template<typename T, typename RetT>
    inline RetT CheckNullAndReturn(T value, RetT returnValue, LogFuncType logFunc, const char* format, ...) {
        if (value == nullptr) {
            logFunc("%s: value is nullptr", format);
            return returnValue;
        }
        return true;
    }

    template <typename RetT>
    inline RetT CheckFalseReturn(bool condition, RetT returnValue, LogFuncType logFunc, const char* format) {
        if (!condition) {
            logFunc("%s", format);
            return returnValue;
        }
        return returnValue ? returnValue : RetT{}; // 返回一个合适的默认值
    }

    template<typename T>
    inline void CheckFalseExit(T value, LogFuncType logFunc, const char *format) {
        if (!value) {
            logFunc("%s", format);
            return;
        }
    }

    /**
     * @brief Standard value check and return
     */
    template<typename T, typename RetT>
    inline RetT CheckValueAndReturn(T value, T min, T max, RetT returnValue, const char* format, ...) {
        if (!(value >= min && value <= max)) {
            va_list args;
            va_start(args, format);
            char buffer[256];
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            DefaultErrorLog("%s: value %d not in range [%d, %d]", buffer, value, min, max);
            return returnValue;
        }
        return true;
    }

    /**
     * @brief Standard null check and return
     */
    template<typename T, typename RetT>
    inline RetT CheckNullAndReturn(T value, RetT returnValue, const char* format, ...) {
        if (value == nullptr) {
            va_list args;
            va_start(args, format);
            char buffer[256];
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            
            DefaultErrorLog("%s: value is nullptr", buffer);
            return returnValue;
        }
        return true;
    }

    /**
     * @brief 标准条件检查函数，使用DefaultErrorLog作为日志输出
     */
    template<typename RetT>
    inline RetT CheckFalseReturn(bool condition, RetT returnValue, const char* format, ...) {
        if (!condition) {
            va_list args;
            va_start(args, format);
            char buffer[256];
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            
            DefaultErrorLog("%s", buffer);
            return returnValue;
        }
        return returnValue ? returnValue : RetT{};
    }

    template<typename T>
    inline void CheckFalseExit(T value, const char *format, ...) {
        if (!value) {
            va_list args;
            va_start(args, format);
            char buffer[256];
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            
            DefaultErrorLog("%s", buffer);
            return;
        }
    }


}
#endif