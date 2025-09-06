// components/lamin-pie/src/porting/laminpie_porting.h
#ifndef LAMINPIE_PORTING_H
#define LAMINPIE_PORTING_H

#include <stdint.h>
#include <stdbool.h>
#include "erro_handle.h"
#include "../laminpie_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 操作系统选择 */
#define LAMINPIE_OS_NONE        0
#define LAMINPIE_OS_STD_THREAD  1
#define LAMINPIE_OS_PTHREAD     2
#define LAMINPIE_OS_FREERTOS    3
#define LAMINPIE_OS_RTTHREAD    4
#define LAMINPIE_OS_CUSTOM      5

/* 默认使用标准库线程 */
#ifndef LAMINPIE_USE_OS
#define LAMINPIE_USE_OS LAMINPIE_OS_STD_THREAD
#endif


/* 条件编译选择具体实现 */
#if LAMINPIE_USE_OS == LAMINPIE_OS_NONE
#include "none/laminpie_none.h"
#elif LAMINPIE_USE_OS == LAMINPIE_OS_STD_THREAD
#include "std_thread/laminpie_std_thread.h"
#elif LAMINPIE_USE_OS == LAMINPIE_OS_PTHREAD
#include "pthread/laminpie_pthread.h"
#elif LAMINPIE_USE_OS == LAMINPIE_OS_FREERTOS
#include "freertos/laminpie_freertos.h"
#elif LAMINPIE_USE_OS == LAMINPIE_OS_RTTHREAD
#include "rtthread/laminpie_rtthread.h"
#elif LAMINPIE_USE_OS == LAMINPIE_OS_CUSTOM
#include LAMINPIE_OS_CUSTOM_INCLUDE
#endif

/* 统一的函数接口 */
laminpie_result_t laminpie_thread_init(laminpie_thread_t *thread, 
                                      laminpie_thread_prio_t prio,
                                      void (*callback)(void *), 
                                      size_t stack_size, 
                                      void *user_data);

laminpie_result_t laminpie_thread_delete(laminpie_thread_t *thread);

laminpie_result_t laminpie_mutex_init(laminpie_mutex_t *mutex);
laminpie_result_t laminpie_mutex_lock(laminpie_mutex_t *mutex);
laminpie_result_t laminpie_mutex_unlock(laminpie_mutex_t *mutex);
laminpie_result_t laminpie_mutex_delete(laminpie_mutex_t *mutex);

laminpie_result_t laminpie_semaphore_init(laminpie_semaphore_t *sem, uint32_t initial_count);
laminpie_result_t laminpie_semaphore_wait(laminpie_semaphore_t *sem, uint32_t timeout_ms);
laminpie_result_t laminpie_semaphore_post(laminpie_semaphore_t *sem);
laminpie_result_t laminpie_semaphore_delete(laminpie_semaphore_t *sem);

/* 全局锁接口 */
void laminpie_lock(void);
void laminpie_unlock(void);
laminpie_result_t laminpie_lock_isr(void);

/* 初始化函数 */
void laminpie_porting_init(void);

#ifdef __cplusplus
}
#endif

#endif /* LAMINPIE_PORTING_H */