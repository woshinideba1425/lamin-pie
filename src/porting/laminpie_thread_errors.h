// thread_errors.h - 线程相关错误类型定义
#pragma once
#include "laminpie_error.h"
#include <string>

/**
 * @brief 线程优先级枚举
 * 
 * 定义了5个优先级级别，从最低到最高。
 * 具体实现会根据底层操作系统进行映射。
 */
typedef enum {
    LAMINPIE_THREAD_PRIO_LOWEST = 0,  /**< 最低优先级 */
    LAMINPIE_THREAD_PRIO_LOW,         /**< 低优先级 */
    LAMINPIE_THREAD_PRIO_MID,         /**< 中等优先级 */
    LAMINPIE_THREAD_PRIO_HIGH,        /**< 高优先级 */
    LAMINPIE_THREAD_PRIO_HIGHEST,     /**< 最高优先级 */
} laminpie_thread_prio_t;

namespace laminate::threading {

// 线程错误基类
class ThreadError : public Error {
public:
    ThreadError(std::string message, int code = 0) 
        : message_(std::move(message)), code_(code) {}
    
    std::string message() const override { return message_; }
    std::string type_name() const override { return "ThreadError"; }
    int code() const override { return code_; }

protected:
    std::string message_;
    int code_;
};

// 线程创建错误
class ThreadCreationError : public ThreadError {
public:
    ThreadCreationError(std::string message, int code = 0) 
        : ThreadError(std::move(message), code) {}
    
    std::string type_name() const override { return "ThreadCreationError"; }
};

// 线程删除错误
class ThreadDeletionError : public ThreadError {
public:
    ThreadDeletionError(std::string message, int code = 0) 
        : ThreadError(std::move(message), code) {}
    
    std::string type_name() const override { return "ThreadDeletionError"; }
};

// 互斥锁错误
class MutexError : public ThreadError {
public:
    MutexError(std::string message, int code = 0) 
        : ThreadError(std::move(message), code) {}
    
    std::string type_name() const override { return "MutexError"; }
};

// 信号量错误
class SemaphoreError : public ThreadError {
public:
    SemaphoreError(std::string message, int code = 0) 
        : ThreadError(std::move(message), code) {}
    
    std::string type_name() const override { return "SemaphoreError"; }
};

// 超时错误
class TimeoutError : public ThreadError {
public:
    TimeoutError(std::string message, uint32_t timeout_ms) 
        : ThreadError(std::move(message), static_cast<int>(timeout_ms)), timeout_ms_(timeout_ms) {}
    
    std::string type_name() const override { return "TimeoutError"; }
    uint32_t timeout_ms() const { return timeout_ms_; }

private:
    uint32_t timeout_ms_;
};

// 参数错误
class InvalidParameterError : public ThreadError {
public:
    InvalidParameterError(std::string message) 
        : ThreadError(std::move(message), -1) {}
    
    std::string type_name() const override { return "InvalidParameterError"; }
};

// 资源不足错误
class ResourceExhaustedError : public ThreadError {
public:
    ResourceExhaustedError(std::string message) 
        : ThreadError(std::move(message), -2) {}
    
    std::string type_name() const override { return "ResourceExhaustedError"; }
};

} // namespace laminate::threading
