#pragma once

#include <stdio.h>
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