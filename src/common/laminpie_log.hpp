#pragma once

#include <stdio.h>
#include <stdarg.h>

// Check C++20 support
#if __cplusplus >= 202002L && defined(__cpp_nontype_template_args)
#   define LAMINPIE_LOG_CXX20_SUPPORT 1
#   include <source_location>
#else
#   define LAMINPIE_LOG_CXX20_SUPPORT 0
#endif

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
#include <algorithm>
#include <string>
#include <cstring>

namespace laminpie::utils {
    using LogFuncType = void (*)(const char* fmt, ...);
    
    // 提取文件名（去除路径）
    inline std::string extractFileName(const char* filePath) {
        if (!filePath) return "???";
        
        const char* fileName = filePath;
        const char* lastSlash = std::strrchr(filePath, '/');
        if (lastSlash) {
            fileName = lastSlash + 1;
        } else {
            lastSlash = std::strrchr(filePath, '\\');
            if (lastSlash) {
                fileName = lastSlash + 1;
            }
        }
        return std::string(fileName);
    }
    
    // 解析函数名（去除模板参数等）
    inline std::string parseFunctionName(const char* funcName) {
        if (!funcName) return "???";
        
        std::string func(funcName);
        
        // 移除模板参数
        size_t templateStart = func.find('<');
        if (templateStart != std::string::npos) {
            func = func.substr(0, templateStart);
        }
        
        // 移除参数列表
        size_t parenStart = func.find('(');
        if (parenStart != std::string::npos) {
            func = func.substr(0, parenStart);
        }
        
        // 提取最后的函数名
        size_t lastColon = func.rfind("::");
        if (lastColon != std::string::npos) {
            func = func.substr(lastColon + 2);
        }
        
        return func.empty() ? "???" : func;
    }

#if LAMINPIE_LOG_CXX20_SUPPORT
    // C++20 FixedString模板
    template <size_t N>
    struct FixedString {
        char data[N] {};

        constexpr FixedString(const char (&str)[N]) {
            std::copy_n(str, N, data);
        }

        constexpr bool operator==(const FixedString&) const = default;
        constexpr bool operator<(const FixedString& rhs) const {
            for (size_t i = 0; i < N; ++i) {
                if (data[i] != rhs.data[i]) {
                    return data[i] < rhs.data[i];
                }
            }
            return false;
        }

        constexpr const char* c_str() const {
            return data;
        }
        constexpr size_t size() const {
            return N;
        }
    };

    // C++20 Log trace RAII class
    template <FixedString TAG>
    class LogTraceGuard {
    public:
        LogTraceGuard(const void* thisPtr = nullptr, const std::source_location& loc = std::source_location::current())
            : _thisPtr(thisPtr) {
            _line = static_cast<int>(loc.line());
            _funcName = parseFunctionName(loc.function_name());
            if (_funcName.empty()) {
                _funcName = "???";
            }
            _fileName = extractFileName(loc.file_name());
            if (_fileName.empty()) {
                _fileName = "???";
            }

            if (_thisPtr) {
                LP_LOG_DEBUG(TAG.c_str(), "[%s:%04d](%s): (@%p) Enter", 
                           _fileName.c_str(), _line, _funcName.c_str(), _thisPtr);
            } else {
                LP_LOG_DEBUG(TAG.c_str(), "[%s:%04d](%s): Enter", 
                           _fileName.c_str(), _line, _funcName.c_str());
            }
        }

        ~LogTraceGuard() {
            if (_thisPtr) {
                LP_LOG_DEBUG(TAG.c_str(), "[%s:%04d](%s): (@%p) Exit", 
                           _fileName.c_str(), _line, _funcName.c_str(), _thisPtr);
            } else {
                LP_LOG_DEBUG(TAG.c_str(), "[%s:%04d](%s): Exit", 
                           _fileName.c_str(), _line, _funcName.c_str());
            }
        }

        LogTraceGuard(const LogTraceGuard&) = delete;
        LogTraceGuard(LogTraceGuard&&) = delete;
        LogTraceGuard& operator=(const LogTraceGuard&) = delete;
        LogTraceGuard& operator=(LogTraceGuard&&) = delete;

    private:
        int _line = 0;
        std::string _funcName;
        std::string _fileName;
        const void* _thisPtr = nullptr;
    };

#else
    // C++17 fallback implementation
    class LogTraceGuard {
    public:
        LogTraceGuard(const char* tag, const char* func, const char* file, int line, const void* thisPtr = nullptr)
            : _tag(tag), _line(line), _thisPtr(thisPtr) {
            _funcName = parseFunctionName(func);
            if (_funcName.empty()) {
                _funcName = "???";
            }
            _fileName = extractFileName(file);
            if (_fileName.empty()) {
                _fileName = "???";
            }

            if (_thisPtr) {
                LP_LOG_DEBUG(_tag, "[%s:%04d](%s): (@%p) Enter", 
                           _fileName.c_str(), _line, _funcName.c_str(), _thisPtr);
            } else {
                LP_LOG_DEBUG(_tag, "[%s:%04d](%s): Enter", 
                           _fileName.c_str(), _line, _funcName.c_str());
            }
        }

        ~LogTraceGuard() {
            if (_thisPtr) {
                LP_LOG_DEBUG(_tag, "[%s:%04d](%s): (@%p) Exit", 
                           _fileName.c_str(), _line, _funcName.c_str(), _thisPtr);
            } else {
                LP_LOG_DEBUG(_tag, "[%s:%04d](%s): Exit", 
                           _fileName.c_str(), _line, _funcName.c_str());
            }
        }

        LogTraceGuard(const LogTraceGuard&) = delete;
        LogTraceGuard(LogTraceGuard&&) = delete;
        LogTraceGuard& operator=(const LogTraceGuard&) = delete;
        LogTraceGuard& operator=(LogTraceGuard&&) = delete;

    private:
        const char* _tag;
        int _line = 0;
        std::string _funcName;
        std::string _fileName;
        const void* _thisPtr = nullptr;
    };
#endif

    inline void DefaultErrorLog(const char* fmt, ...) {
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
            va_list args;
            va_start(args, format);
            logFunc(format, args);
            va_end(args);
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
            va_list args;
            va_start(args, format);
            logFunc(format, args);
            va_end(args);
            return returnValue;
        }
        return true;
    }

    template <typename RetT>
    inline RetT CheckFalseReturn(bool condition, RetT returnValue, LogFuncType logFunc, const char* format, ...) {
        if (!condition) {
            va_list args;
            va_start(args, format);
            logFunc(format, args);
            va_end(args);
            return returnValue;
        }
        return returnValue ? returnValue : RetT{}; // 返回一个合适的默认值
    }

    template<typename T>
    inline void CheckFalseExit(T value, LogFuncType logFunc, const char *format, ...) {
        if (!value) {
            va_list args;
            va_start(args, format);
            logFunc(format, args);
            va_end(args);
            return;
        }
    }

    template<typename T>
    inline void CheckNullExit(T value, LogFuncType logFunc, const char *format, ...) {
        if (value == nullptr) {
            va_list args;
            va_start(args, format);
            logFunc(format, args);
            va_end(args);
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

    template<typename T>
    inline void CheckNullExit(T value, const char *format, ...) {
        if (value == nullptr) {
            va_list args;
            va_start(args, format);
            DefaultErrorLog(format, args);
            va_end(args);
            return;
        }
    }
}

// Trace Guard宏定义
#if LP_LOG_LEVEL <= LP_LOG_LEVEL_DEBUG
    #if LAMINPIE_LOG_CXX20_SUPPORT
        #define LP_LOG_MAKE_FS(str) []{ constexpr laminpie::utils::FixedString<sizeof(str)> s(str); return s; }()
        #define LP_LOG_TRACE_GUARD(LP_LOG_TAG)           laminpie::utils::LogTraceGuard<LP_LOG_MAKE_FS(LP_LOG_TAG)> _log_trace_guard_{}
        #define LP_LOG_TRACE_GUARD_WITH_THIS(LP_LOG_TAG) laminpie::utils::LogTraceGuard<LP_LOG_MAKE_FS(LP_LOG_TAG)> _log_trace_guard_{this}
    #else
        #define LP_LOG_TRACE_GUARD(LP_LOG_TAG)           laminpie::utils::LogTraceGuard _log_trace_guard_{LP_LOG_TAG, __func__, __FILE__, __LINE__}
        #define LP_LOG_TRACE_GUARD_WITH_THIS(LP_LOG_TAG) laminpie::utils::LogTraceGuard _log_trace_guard_{LP_LOG_TAG, __func__, __FILE__, __LINE__, this}
    #endif
#else
    #define LP_LOG_TRACE_GUARD()
    #define LP_LOG_TRACE_GUARD_WITH_THIS()
#endif

#endif // __cplusplus