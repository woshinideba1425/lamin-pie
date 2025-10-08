/**
 * @file laminpie_thread_porting.h
 * @brief 跨平台线程抽象层接口定义
 * @author LaminPie Team
 * @date 2024
 * 
 * 本文件定义了统一的跨平台线程、互斥锁和信号量接口，支持多种操作系统：
 * - 标准C++线程库 (std::thread)
 * - POSIX线程 (pthread)
 * - FreeRTOS
 * - RT-Thread
 * - 自定义实现
 * 
 * 所有接口都使用Result<T>类型进行错误处理，提供类型安全的错误管理。
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "laminpie_result.h"
#include "laminpie_thread_errors.h"

/** @} */

/**
 * @brief 操作系统选择
 * 
 * 优先使用Kconfig配置，如果没有则使用默认值
 */
#ifndef LAMINPIE_USE_OS
    #if defined(CONFIG_LAMINPIE_USE_OS_VALUE)
        #define LAMINPIE_USE_OS CONFIG_LAMINPIE_USE_OS_VALUE
    #else
        #define LAMINPIE_USE_OS LAMINPIE_OS_STD_THREAD
    #endif
#endif

/**
 * @defgroup FORWARD_DECLARATIONS 前向声明
 * @{
 */

/** @brief 线程句柄结构体 */
typedef struct laminpie_thread_t laminpie_thread_t;
/** @brief 互斥锁句柄结构体 */
typedef struct laminpie_mutex_t laminpie_mutex_t;
/** @brief 信号量句柄结构体 */
typedef struct laminpie_semaphore_t laminpie_semaphore_t;

/** @} */

/**
 * @defgroup THREAD_API 线程管理接口
 * @{
 */

/**
 * @brief 创建新线程
 * 
 * @param[in] prio 线程优先级
 * @param[in] callback 线程执行函数指针
 * @param[in] stack_size 线程栈大小（字节）
 * @param[in] user_data 传递给线程函数的用户数据
 * 
 * @return Result<laminpie_thread_t*> 成功时返回线程句柄，失败时返回错误信息
 * 
 * @retval ThreadCreationError 线程创建失败
 * @retval InvalidParameterError 参数无效
 * @retval ResourceExhaustedError 系统资源不足
 * 
 * @note 线程创建后立即开始执行
 * @note 调用者负责在适当时机调用laminpie_thread_delete释放资源
 */
laminate::Result<laminpie_thread_t*> laminpie_thread_create(
    laminpie_thread_prio_t prio,
    void (*callback)(void *), 
    size_t stack_size, 
    void *user_data);

/**
 * @brief 删除线程并释放相关资源
 * 
 * @param[in] thread 要删除的线程句柄
 * 
 * @return Result<void> 成功时无返回值，失败时返回错误信息
 * 
 * @retval ThreadDeletionError 线程删除失败
 * @retval InvalidParameterError 线程句柄无效
 * 
 * @note 如果线程仍在运行，此函数会等待线程结束
 * @note 删除后线程句柄变为无效，不应再次使用
 */
laminate::Result<void> laminpie_thread_delete(laminpie_thread_t *thread);

/**
 * @brief 等待线程结束
 * 
 * @param[in] thread 要等待的线程句柄
 * @param[in] timeout_ms 超时时间（毫秒），0表示无限等待
 * 
 * @return Result<void> 成功时无返回值，失败时返回错误信息
 * 
 * @retval TimeoutError 等待超时
 * @retval InvalidParameterError 线程句柄无效
 * 
 * @note 超时时间为0表示无限等待
 * @note 线程已结束的情况下会立即返回成功
 */
laminate::Result<void> laminpie_thread_join(laminpie_thread_t *thread, uint32_t timeout_ms);

/**
 * @brief 检查线程是否正在运行
 * 
 * @param[in] thread 要检查的线程句柄
 * 
 * @return Result<bool> 成功时返回运行状态，失败时返回错误信息
 * 
 * @retval true 线程正在运行
 * @retval false 线程未运行或已结束
 * @retval InvalidParameterError 线程句柄无效
 */
laminate::Result<bool> laminpie_thread_is_running(laminpie_thread_t *thread);

/** @} */

/**
 * @defgroup MUTEX_API 互斥锁接口
 * @{
 */

/**
 * @brief 创建互斥锁
 * 
 * @return Result<laminpie_mutex_t*> 成功时返回互斥锁句柄，失败时返回错误信息
 * 
 * @retval MutexError 互斥锁创建失败
 * @retval ResourceExhaustedError 系统资源不足
 * 
 * @note 创建的互斥锁初始状态为未锁定
 * @note 调用者负责在适当时机调用laminpie_mutex_delete释放资源
 */
laminate::Result<laminpie_mutex_t*> laminpie_mutex_create(void);

/**
 * @brief 锁定互斥锁
 * 
 * @param[in] mutex 要锁定的互斥锁句柄
 * 
 * @return Result<void> 成功时无返回值，失败时返回错误信息
 * 
 * @retval MutexError 互斥锁操作失败
 * @retval InvalidParameterError 互斥锁句柄无效
 * 
 * @note 如果互斥锁已被锁定，当前线程会阻塞等待
 * @note 同一线程可以多次锁定同一个互斥锁（递归锁）
 */
laminate::Result<void> laminpie_mutex_lock(laminpie_mutex_t *mutex);

/**
 * @brief 解锁互斥锁
 * 
 * @param[in] mutex 要解锁的互斥锁句柄
 * 
 * @return Result<void> 成功时无返回值，失败时返回错误信息
 * 
 * @retval MutexError 互斥锁操作失败
 * @retval InvalidParameterError 互斥锁句柄无效
 * 
 * @note 只有锁定互斥锁的线程才能解锁
 * @note 递归锁需要相同次数的解锁调用
 */
laminate::Result<void> laminpie_mutex_unlock(laminpie_mutex_t *mutex);

/**
 * @brief 删除互斥锁并释放相关资源
 * 
 * @param[in] mutex 要删除的互斥锁句柄
 * 
 * @return Result<void> 成功时无返回值，失败时返回错误信息
 * 
 * @retval MutexError 互斥锁删除失败
 * @retval InvalidParameterError 互斥锁句柄无效
 * 
 * @note 删除前确保没有线程持有该互斥锁
 * @note 删除后互斥锁句柄变为无效，不应再次使用
 */
laminate::Result<void> laminpie_mutex_delete(laminpie_mutex_t *mutex);

/** @} */

/**
 * @defgroup SEMAPHORE_API 信号量接口
 * @{
 */

/**
 * @brief 创建信号量
 * 
 * @param[in] initial_count 信号量初始计数值
 * 
 * @return Result<laminpie_semaphore_t*> 成功时返回信号量句柄，失败时返回错误信息
 * 
 * @retval SemaphoreError 信号量创建失败
 * @retval InvalidParameterError 初始计数值无效
 * @retval ResourceExhaustedError 系统资源不足
 * 
 * @note 初始计数值必须大于等于0
 * @note 调用者负责在适当时机调用laminpie_semaphore_delete释放资源
 */
laminate::Result<laminpie_semaphore_t*> laminpie_semaphore_create(uint32_t initial_count);

/**
 * @brief 等待信号量（P操作）
 * 
 * @param[in] sem 要等待的信号量句柄
 * @param[in] timeout_ms 超时时间（毫秒），0表示无限等待
 * 
 * @return Result<void> 成功时无返回值，失败时返回错误信息
 * 
 * @retval TimeoutError 等待超时
 * @retval SemaphoreError 信号量操作失败
 * @retval InvalidParameterError 信号量句柄无效
 * 
 * @note 超时时间为0表示无限等待
 * @note 信号量计数值减1，如果为0则阻塞等待
 */
laminate::Result<void> laminpie_semaphore_wait(laminpie_semaphore_t *sem, uint32_t timeout_ms);

/**
 * @brief 释放信号量（V操作）
 * 
 * @param[in] sem 要释放的信号量句柄
 * 
 * @return Result<void> 成功时无返回值，失败时返回错误信息
 * 
 * @retval SemaphoreError 信号量操作失败
 * @retval InvalidParameterError 信号量句柄无效
 * 
 * @note 信号量计数值加1
 * @note 如果有线程在等待，会唤醒其中一个
 */
laminate::Result<void> laminpie_semaphore_post(laminpie_semaphore_t *sem);

/**
 * @brief 删除信号量并释放相关资源
 * 
 * @param[in] sem 要删除的信号量句柄
 * 
 * @return Result<void> 成功时无返回值，失败时返回错误信息
 * 
 * @retval SemaphoreError 信号量删除失败
 * @retval InvalidParameterError 信号量句柄无效
 * 
 * @note 删除前确保没有线程在等待该信号量
 * @note 删除后信号量句柄变为无效，不应再次使用
 */
laminate::Result<void> laminpie_semaphore_delete(laminpie_semaphore_t *sem);

/** @} */

/**
 * @defgroup GLOBAL_LOCK_API 全局锁接口
 * @{
 */

/**
 * @brief 获取全局锁
 * 
 * 用于保护全局资源的简单互斥锁，适用于中断上下文。
 * 
 * @note 此函数不返回错误，调用者必须确保正确配对调用
 * @note 在中断服务程序中应使用laminpie_global_lock_isr
 */
void laminpie_global_lock(void);

/**
 * @brief 释放全局锁
 * 
 * 释放之前获取的全局锁。
 * 
 * @note 必须与laminpie_global_lock配对调用
 * @note 在中断服务程序中应使用对应的解锁函数
 */
void laminpie_global_unlock(void);

/**
 * @brief 在中断服务程序中获取全局锁
 * 
 * @return Result<void> 成功时无返回值，失败时返回错误信息
 * 
 * @retval MutexError 全局锁操作失败
 * 
 * @note 专门用于中断服务程序中的全局锁操作
 * @note 必须与对应的中断解锁函数配对使用
 */
laminate::Result<void> laminpie_global_lock_isr(void);

/** @} */

/**
 * @defgroup INIT_CLEANUP_API 初始化和清理接口
 * @{
 */

/**
 * @brief 初始化线程porting系统
 * 
 * 初始化底层操作系统的线程相关资源，必须在调用其他线程接口前调用。
 * 
 * @return Result<void> 成功时无返回值，失败时返回错误信息
 * 
 * @retval ThreadError 系统初始化失败
 * @retval ResourceExhaustedError 系统资源不足
 * 
 * @note 可以多次调用，但只有第一次调用会生效
 * @note 必须在程序退出前调用laminpie_porting_cleanup
 */
laminate::Result<void> laminpie_porting_init(void);

/**
 * @brief 清理线程porting系统
 * 
 * 清理底层操作系统的线程相关资源，释放所有分配的资源。
 * 
 * @return Result<void> 成功时无返回值，失败时返回错误信息
 * 
 * @retval ThreadError 系统清理失败
 * 
 * @note 调用后不应再使用任何线程相关接口
 * @note 必须与laminpie_porting_init配对调用
 */
laminate::Result<void> laminpie_porting_cleanup(void);

/** @} */