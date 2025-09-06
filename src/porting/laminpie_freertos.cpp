// laminpie_freertos.cpp - 基于FreeRTOS的线程移植层实现
#include "laminpie_thread.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include <cstring>
#include <atomic>
#include <memory>

using namespace laminate;
using namespace laminate::threading;

// 添加通用的Err函数模板
template<typename T, typename E>
Result<T> Err(E error) {
    return Result<T>(std::move(error));
}

// 线程结构体定义
struct laminpie_thread_t {
    TaskHandle_t task_handle;
    void (*callback)(void *);
    void *user_data;
    std::atomic<bool> running;
    std::atomic<bool> should_stop;
    UBaseType_t priority;
    size_t stack_size;
    char task_name[16];
    
    laminpie_thread_t() : task_handle(nullptr), callback(nullptr), user_data(nullptr), 
                         running(false), should_stop(false), priority(0), stack_size(0) {
        memset(task_name, 0, sizeof(task_name));
    }
    
    ~laminpie_thread_t() {
        if (task_handle != nullptr) {
            should_stop = true;
            vTaskDelete(task_handle);
            task_handle = nullptr;
        }
    }
};

// 互斥锁结构体定义
struct laminpie_mutex_t {
    SemaphoreHandle_t mutex_handle;
    std::atomic<bool> locked;
    
    laminpie_mutex_t() : mutex_handle(nullptr), locked(false) {
        mutex_handle = xSemaphoreCreateMutex();
    }
    
    ~laminpie_mutex_t() {
        if (mutex_handle != nullptr) {
            vSemaphoreDelete(mutex_handle);
            mutex_handle = nullptr;
        }
    }
};

// 信号量结构体定义
struct laminpie_semaphore_t {
    SemaphoreHandle_t semaphore_handle;
    std::atomic<uint32_t> count;
    
    laminpie_semaphore_t(uint32_t initial_count) : count(initial_count) {
        semaphore_handle = xSemaphoreCreateCounting(UINT32_MAX, initial_count);
    }
    
    ~laminpie_semaphore_t() {
        if (semaphore_handle != nullptr) {
            vSemaphoreDelete(semaphore_handle);
            semaphore_handle = nullptr;
        }
    }
};

// 全局互斥锁
static SemaphoreHandle_t g_global_mutex = nullptr;

// 内部函数声明
static void thread_wrapper(void *pvParameters);
static UBaseType_t map_priority(laminpie_thread_prio_t prio);

// 线程包装函数
static void thread_wrapper(void *pvParameters) {
    laminpie_thread_t *thread = static_cast<laminpie_thread_t*>(pvParameters);
    
    if (thread && thread->callback) {
        thread->running = true;
        thread->callback(thread->user_data);
        thread->running = false;
    }
    
    // 删除任务
    vTaskDelete(nullptr);
}

// 优先级映射函数
static UBaseType_t map_priority(laminpie_thread_prio_t prio) {
    switch (prio) {
        case LAMINPIE_THREAD_PRIO_LOWEST:
            return 1; 
        case LAMINPIE_THREAD_PRIO_LOW:
            return 3;
        case LAMINPIE_THREAD_PRIO_MID:
            return 5;
        case LAMINPIE_THREAD_PRIO_HIGH:
            return 7;
        case LAMINPIE_THREAD_PRIO_HIGHEST:
            return 9; 
        default:
            return 5; 
    }
}

// 线程管理函数实现
Result<laminpie_thread_t*> laminpie_thread_create(
    laminpie_thread_prio_t prio,
    void (*callback)(void *), 
    size_t stack_size, 
    void *user_data) {
    
    if (callback == nullptr) {
        return Err<laminpie_thread_t*>(InvalidParameterError("Callback function cannot be null"));
    }
    
    if (stack_size == 0) {
        stack_size = 4096;  // 默认栈大小
    }
    
    // 创建线程结构体
    auto thread = new(std::nothrow) laminpie_thread_t();
    if (thread == nullptr) {
        return Err<laminpie_thread_t*>(ResourceExhaustedError("Failed to allocate memory for thread"));
    }
    
    thread->callback = callback;
    thread->user_data = user_data;
    thread->stack_size = stack_size;
    thread->priority = map_priority(prio);
    snprintf(thread->task_name, sizeof(thread->task_name), "laminpie_%p", thread);
    
    // 创建FreeRTOS任务
    BaseType_t result = xTaskCreate(
        thread_wrapper,
        thread->task_name,
        stack_size / sizeof(StackType_t),
        thread,
        thread->priority,
        &thread->task_handle
    );
    
    if (result != pdPASS) {
        delete thread;
        return Err<laminpie_thread_t*>(ThreadCreationError("Failed to create FreeRTOS task"));
    }
    
    return Ok(thread);
}

Result<void> laminpie_thread_delete(laminpie_thread_t *thread) {
    if (thread == nullptr) {
        return Err<void>(InvalidParameterError("Thread handle cannot be null"));
    }
    
    if (thread->task_handle != nullptr) {
        thread->should_stop = true;
        vTaskDelete(thread->task_handle);
        thread->task_handle = nullptr;
    }
    
    delete thread;
    return Ok();
}

Result<void> laminpie_thread_join(laminpie_thread_t *thread, uint32_t timeout_ms) {
    if (thread == nullptr) {
        return Err<void>(InvalidParameterError("Thread handle cannot be null"));
    }
    
    if (thread->task_handle == nullptr) {
        return Ok();  // 线程已经结束
    }
    
    // FreeRTOS没有直接的join功能，我们通过检查任务状态来实现
    if (timeout_ms == 0) {
        // 无限等待
        while (thread->running.load()) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    } else {
        // 超时等待
        uint32_t start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        while (thread->running.load()) {
            if ((xTaskGetTickCount() * portTICK_PERIOD_MS - start_time) >= timeout_ms) {
                return Err<void>(TimeoutError("Thread join timeout", 0));
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    
    return Ok();
}

Result<bool> laminpie_thread_is_running(laminpie_thread_t *thread) {
    if (thread == nullptr) {
        return Err<bool>(InvalidParameterError("Thread handle cannot be null"));
    }
    
    return Ok(thread->running.load());
}

// 互斥锁函数实现
Result<laminpie_mutex_t*> laminpie_mutex_create(void) {
    auto mutex = new(std::nothrow) laminpie_mutex_t();
    if (mutex == nullptr) {
        return Err<laminpie_mutex_t*>(ResourceExhaustedError("Failed to allocate memory for mutex"));
    }
    
    if (mutex->mutex_handle == nullptr) {
        delete mutex;
        return Err<laminpie_mutex_t*>(MutexError("Failed to create FreeRTOS mutex"));
    }
    
    return Ok(mutex);
}

Result<void> laminpie_mutex_lock(laminpie_mutex_t *mutex) {
    if (mutex == nullptr) {
        return Err<void>(InvalidParameterError("Mutex handle cannot be null"));
    }
    
    if (mutex->mutex_handle == nullptr) {
        return Err<void>(InvalidParameterError("Mutex handle is invalid"));
    }
    
    BaseType_t result = xSemaphoreTake(mutex->mutex_handle, portMAX_DELAY);
    if (result != pdTRUE) {
        return Err<void>(MutexError("Failed to lock mutex"));
    }
    
    mutex->locked = true;
    return Ok();
}

Result<void> laminpie_mutex_unlock(laminpie_mutex_t *mutex) {
    if (mutex == nullptr) {
        return Err<void>(InvalidParameterError("Mutex handle cannot be null"));
    }
    
    if (mutex->mutex_handle == nullptr) {
        return Err<void>(InvalidParameterError("Mutex handle is invalid"));
    }
    
    BaseType_t result = xSemaphoreGive(mutex->mutex_handle);
    if (result != pdTRUE) {
        return Err<void>(MutexError("Failed to unlock mutex"));
    }
    
    mutex->locked = false;
    return Ok();
}

Result<void> laminpie_mutex_delete(laminpie_mutex_t *mutex) {
    if (mutex == nullptr) {
        return Err<void>(InvalidParameterError("Mutex handle cannot be null"));
    }
    
    delete mutex;
    return Ok();
}

// 信号量函数实现
Result<laminpie_semaphore_t*> laminpie_semaphore_create(uint32_t initial_count) {
    auto semaphore = new(std::nothrow) laminpie_semaphore_t(initial_count);
    if (semaphore == nullptr) {
        return Err<laminpie_semaphore_t*>(ResourceExhaustedError("Failed to allocate memory for semaphore"));
    }
    
    if (semaphore->semaphore_handle == nullptr) {
        delete semaphore;
        return Err<laminpie_semaphore_t*>(SemaphoreError("Failed to create FreeRTOS semaphore"));
    }
    
    return Ok(semaphore);
}

Result<void> laminpie_semaphore_wait(laminpie_semaphore_t *sem, uint32_t timeout_ms) {
    if (sem == nullptr) {
        return Err<void>(InvalidParameterError("Semaphore handle cannot be null"));
    }
    
    if (sem->semaphore_handle == nullptr) {
        return Err<void>(InvalidParameterError("Semaphore handle is invalid"));
    }
    
    TickType_t timeout_ticks = (timeout_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    BaseType_t result = xSemaphoreTake(sem->semaphore_handle, timeout_ticks);
    
    if (result != pdTRUE) {
        if (timeout_ms > 0) {
            return Err<void>(TimeoutError("Semaphore wait timeout", 0));
        } else {
            return Err<void>(SemaphoreError("Failed to wait for semaphore", 0));
        }
    }
    
    sem->count--;
    return Ok();
}

Result<void> laminpie_semaphore_post(laminpie_semaphore_t *sem) {
    if (sem == nullptr) {
        return Err<void>(InvalidParameterError("Semaphore handle cannot be null"));
    }
    
    if (sem->semaphore_handle == nullptr) {
        return Err<void>(InvalidParameterError("Semaphore handle is invalid"));
    }
    
    BaseType_t result = xSemaphoreGive(sem->semaphore_handle);
    if (result != pdTRUE) {
        return Err<void>(SemaphoreError("Failed to post semaphore"));
    }
    
    sem->count++;
    return Ok();
}

Result<void> laminpie_semaphore_delete(laminpie_semaphore_t *sem) {
    if (sem == nullptr) {
        return Err<void>(InvalidParameterError("Semaphore handle cannot be null"));
    }
    
    delete sem;
    return Ok();
}

// 全局锁函数实现
void laminpie_global_lock(void) {
    if (g_global_mutex != nullptr) {
        xSemaphoreTake(g_global_mutex, portMAX_DELAY);
    }
}

void laminpie_global_unlock(void) {
    if (g_global_mutex != nullptr) {
        xSemaphoreGive(g_global_mutex);
    }
}

Result<void> laminpie_global_lock_isr(void) {
    if (g_global_mutex != nullptr) {
        BaseType_t result = xSemaphoreTakeFromISR(g_global_mutex, nullptr);
        if (result != pdTRUE) {
            return Err<void>(MutexError("Failed to lock global mutex from ISR"));
        }
    }
    return Ok();
}

// 初始化和清理函数实现
Result<void> laminpie_porting_init(void) {
    static bool initialized = false;
    if (initialized) {
        return Ok();
    }
    
    // 创建全局互斥锁
    g_global_mutex = xSemaphoreCreateMutex();
    if (g_global_mutex == nullptr) {
        return Err<void>(ThreadError("Failed to create global mutex"));
    }
    
    initialized = true;
    return Ok();
}

Result<void> laminpie_porting_cleanup(void) {
    if (g_global_mutex != nullptr) {
        vSemaphoreDelete(g_global_mutex);
        g_global_mutex = nullptr;
    }
    
    return Ok();
}
