#pragma once

/**
 * @file laminpie_log.hpp
 * @brief LaminPie日志系统 - 使用lalog.h作为底层实现
 * @author LaminPie Team
 * @date 2024
 * 
 * 本文件提供LaminPie项目的日志功能，基于lalog.h实现
 * 所有LP_LOG_XXXX和LP_MOD_LOG_XXXX宏已被完全废弃
 * 请直接使用lalog.h中的LOGX宏：
 * - LOGV(...)  - Verbose级别日志
 * - LOGD(...)  - Debug级别日志  
 * - LOGI(...)  - Info级别日志
 * - LOGW(...)  - Warning级别日志
 * - LOGE(...)  - Error级别日志
 * - FATAL(...) - Fatal级别日志
 */

#include "lalog.h" 

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
    
        size_t parenStart = func.find('(');
        if (parenStart != std::string::npos) func = func.substr(0, parenStart);
    
        auto lpos = func.find_first_not_of(' ');
        auto rpos = func.find_last_not_of(' ');
        if (lpos == std::string::npos) return "???";
        func = func.substr(lpos, rpos - lpos + 1);
    
        size_t lastSpace = func.rfind(' ');
        if (lastSpace != std::string::npos) func = func.substr(lastSpace + 1);
        size_t lastColon = func.rfind("::");
        if (lastColon != std::string::npos) func = func.substr(lastColon + 2);
        size_t lt = func.find('<');
        if (lt != std::string::npos) func = func.substr(0, lt);
    
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
                LOGD(_tag, "[%s:%04d](%s): (@%p) Enter", 
                           _fileName.c_str(), _line, _funcName.c_str(), _thisPtr);
            } else {
                LOGD(_tag, "[%s:%04d](%s): Enter", 
                           _fileName.c_str(), _line, _funcName.c_str());
            }
        }

        ~LogTraceGuard() {
            if (_thisPtr) {
                LOGD(_tag, "[%s:%04d](%s): (@%p) Exit", 
                           _fileName.c_str(), _line, _funcName.c_str(), _thisPtr);
            } else {
                LOGD(_tag, "[%s:%04d](%s): Exit", 
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
        
        LOGE(fmt, args);
        
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
        return returnValue;
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
        return returnValue;
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
        return returnValue;
    }

    /**
     * @brief Standard null check and return
     */
    template<typename T, typename RetT>
    inline RetT CheckNullAndReturn(T value, RetT returnValue, const char* format, ...) {
        // 检查指针类型
        if constexpr (std::is_pointer_v<T>) {
            if (value == nullptr) {
                va_list args;
                va_start(args, format);
                char buffer[256];
                vsnprintf(buffer, sizeof(buffer), format, args);
                va_end(args);
                
                DefaultErrorLog("%s: value is nullptr", buffer);
                return returnValue;
            }
        }
        // 检查字符串类型
        else if constexpr (std::is_same_v<T, std::string>) {
            if (value.empty()) {
                va_list args;
                va_start(args, format);
                char buffer[256];
                vsnprintf(buffer, sizeof(buffer), format, args);
                va_end(args);
                
                DefaultErrorLog("%s: string is empty", buffer);
                return returnValue;
            }
        }
        // 其他类型，检查是否为假值
        else {
            if (!value) {
                va_list args;
                va_start(args, format);
                char buffer[256];
                vsnprintf(buffer, sizeof(buffer), format, args);
                va_end(args);
                
                DefaultErrorLog("%s: value is invalid", buffer);
                return returnValue;
            }
        }
        
        // 如果检查通过，返回默认值（通常是成功值）
        return RetT{};
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