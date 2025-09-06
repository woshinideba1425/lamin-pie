#ifndef LAMINPIE_ERROR_HANDLE_H
#define LAMINPIE_ERROR_HANDLE_H

#include "laminpie_log.hpp"
/* 线程优先级定义 */
typedef enum {
    LAMINPIE_THREAD_PRIO_LOWEST = 0,
    LAMINPIE_THREAD_PRIO_LOW,
    LAMINPIE_THREAD_PRIO_MID,
    LAMINPIE_THREAD_PRIO_HIGH,
    LAMINPIE_THREAD_PRIO_HIGHEST,
} laminpie_thread_prio_t;

/* 统一的结果类型 */
typedef enum {
    LAMINPIE_RESULT_OK = 0,
    LAMINPIE_RESULT_INVALID,
    LAMINPIE_RESULT_TIMEOUT,
    LAMINPIE_RESULT_ERROR,
} laminpie_result_t;

#endif /* LAMINPIE_ERROR_HANDLE_H */