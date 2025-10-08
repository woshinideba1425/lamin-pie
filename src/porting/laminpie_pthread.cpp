// laminpie_pthread.cpp - 基于pthread的线程移植层实现
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <cstring>
#include <memory>
#include <atomic>
#include <chrono>
#include <thread>
#include <cinttypes>

// 假设的错误类型和Result定义
#include "laminpie_thread_errors.h"
#include "laminpie_result.h"
#include "../../common/laminpie_log.hpp"

using namespace laminate;
using namespace laminate::threading;

// 添加通用的Err函数模板
template<typename T, typename E>
Result<T> Err(E error) {
    return Result<T>(std::move(error));
}

// 线程结构体
struct laminpie_thread_t {
    pthread_t thread;
    void (*callback)(void *);
    void *user_data;
    std::atomic<bool> running;
    std::atomic<bool> should_stop;
    pthread_mutex_t state_mutex;
    pthread_cond_t state_cv;
    size_t stack_size;
    
    laminpie_thread_t() : callback(nullptr), user_data(nullptr), 
                         running(false), should_stop(false), stack_size(0) {
        pthread_mutex_init(&state_mutex, nullptr);
        pthread_cond_init(&state_cv, nullptr);
    }
    
    ~laminpie_thread_t() {
        if (running.load()) {
            should_stop = true;
            pthread_cond_signal(&state_cv);
            pthread_join(thread, nullptr);
        }
        pthread_mutex_destroy(&state_mutex);
        pthread_cond_destroy(&state_cv);
    }
};

// 互斥锁结构体
struct laminpie_mutex_t {
    pthread_mutex_t mutex;
    std::atomic<bool> locked;
    
    laminpie_mutex_t() : locked(false) {
        pthread_mutex_init(&mutex, nullptr);
    }
    
    ~laminpie_mutex_t() {
        pthread_mutex_destroy(&mutex);
    }
};

// 信号量结构体
struct laminpie_semaphore_t {
    sem_t semaphore;
    std::atomic<uint32_t> count;
    pthread_mutex_t count_mutex;
    
    laminpie_semaphore_t(uint32_t initial_count) : count(initial_count) {
        sem_init(&semaphore, 0, initial_count);
        pthread_mutex_init(&count_mutex, nullptr);
    }
    
    ~laminpie_semaphore_t() {
        sem_destroy(&semaphore);
        pthread_mutex_destroy(&count_mutex);
    }
};

// 全局互斥锁
static pthread_mutex_t g_global_mutex = PTHREAD_MUTEX_INITIALIZER;

// 线程创建函数
Result<laminpie_thread_t*> laminpie_thread_create(
    laminpie_thread_prio_t prio,
    void (*callback)(void *), 
    size_t stack_size, 
    void *user_data) {
    
    LOGI( "Creating pthread with priority: %d, stack size: %zu", prio, stack_size);
    
    // 参数验证
    if (callback == nullptr) {
        LOGE( "Thread callback is null");
        return Err<laminpie_thread_t*>(InvalidParameterError("Thread callback cannot be null"));
    }
    
    // 设置默认栈大小
    if (stack_size == 0) {
        stack_size = 8192; // 默认8KB栈
        LOGW( "Using default stack size: %zu", stack_size);
    }
    
    try {
        // 创建线程对象
        auto thread_obj = std::make_unique<laminpie_thread_t>();
        thread_obj->callback = callback;
        thread_obj->user_data = user_data;
        thread_obj->running = false;
        thread_obj->should_stop = false;
        thread_obj->stack_size = stack_size;
        
        // 设置线程属性
        pthread_attr_t attr;
        if (pthread_attr_init(&attr) != 0) {
            LOGE( "Failed to initialize pthread attributes");
            return Err<laminpie_thread_t*>(ThreadCreationError("Failed to initialize pthread attributes"));
        }
        
        // 设置栈大小
        if (pthread_attr_setstacksize(&attr, stack_size) != 0) {
            LOGE( "Failed to set stack size: %zu", stack_size);
            pthread_attr_destroy(&attr);
            return Err<laminpie_thread_t*>(ThreadCreationError("Failed to set stack size"));
        }
        
        // 设置线程分离状态
        if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) != 0) {
            LOGE( "Failed to set thread detach state");
            pthread_attr_destroy(&attr);
            return Err<laminpie_thread_t*>(ThreadCreationError("Failed to set thread detach state"));
        }
        
        // 创建线程
        auto thread_ptr = thread_obj.get();
        int result = pthread_create(&thread_ptr->thread, &attr, [](void* arg) -> void* {
            auto* t = static_cast<laminpie_thread_t*>(arg);
            
            LOGD( "Thread started");
            
            // 设置运行状态
            {
                pthread_mutex_lock(&t->state_mutex);
                t->running = true;
                pthread_cond_signal(&t->state_cv);
                pthread_mutex_unlock(&t->state_mutex);
            }
            
            // 执行用户回调
            try {
                t->callback(t->user_data);
            } catch (const std::exception& e) {
                LOGE( "Thread callback exception: %s", e.what());
            } catch (...) {
                LOGE( "Thread callback unknown exception");
            }
            
            // 线程结束
            {
                pthread_mutex_lock(&t->state_mutex);
                t->running = false;
                pthread_cond_signal(&t->state_cv);
                pthread_mutex_unlock(&t->state_mutex);
            }
            
            LOGD( "Thread finished");
            return nullptr;
        }, thread_ptr);
        
        pthread_attr_destroy(&attr);
        
        if (result != 0) {
            LOGE( "Failed to create pthread: %s", strerror(result));
            return Err<laminpie_thread_t*>(ThreadCreationError("Failed to create pthread: " + std::string(strerror(result)), result));
        }
        
        // 等待线程启动
        {
            pthread_mutex_lock(&thread_ptr->state_mutex);
            struct timespec timeout;
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += 1; // 1秒超时
            
            while (!thread_ptr->running.load()) {
                int wait_result = pthread_cond_timedwait(&thread_ptr->state_cv, &thread_ptr->state_mutex, &timeout);
                if (wait_result == ETIMEDOUT) {
                    pthread_mutex_unlock(&thread_ptr->state_mutex);
                    LOGE( "Thread failed to start within timeout");
                    return Err<laminpie_thread_t*>(ThreadCreationError("Thread failed to start within timeout"));
                }
            }
            pthread_mutex_unlock(&thread_ptr->state_mutex);
        }
        
        LOGI( "Thread created successfully with stack size: %zu", stack_size);
        return Ok(thread_obj.release());
        
    } catch (const std::exception& e) {
        LOGE( "Exception creating thread: %s", e.what());
        return Err<laminpie_thread_t*>(ThreadCreationError("Exception: " + std::string(e.what())));
    } catch (...) {
        LOGE( "Unknown error creating thread");
        return Err<laminpie_thread_t*>(ThreadCreationError("Unknown error creating thread"));
    }
}

// 线程删除函数
Result<void> laminpie_thread_delete(laminpie_thread_t *thread) {
    if (thread == nullptr) {
        LOGW( "Attempting to delete null thread");
        return Err<void>(InvalidParameterError("Thread pointer is null"));
    }
    
    LOGD( "Deleting thread");
    
    try {
        // 设置停止标志
        thread->should_stop = true;
        pthread_cond_signal(&thread->state_cv);
        
        // 等待线程结束
        if (thread->running.load()) {
            pthread_join(thread->thread, nullptr);
        }
        
        // 删除线程对象
        delete thread;
        
        LOGD( "Thread deleted successfully");
        return Ok();
        
    } catch (const std::exception& e) {
        LOGE( "Exception deleting thread: %s", e.what());
        return Err<void>(ThreadDeletionError("Exception: " + std::string(e.what())));
    } catch (...) {
        LOGE( "Unknown error deleting thread");
        return Err<void>(ThreadDeletionError("Unknown error deleting thread"));
    }
}

// 线程等待函数
Result<void> laminpie_thread_join(laminpie_thread_t *thread, uint32_t timeout_ms) {
    if (thread == nullptr) {
        return Err<void>(InvalidParameterError("Thread pointer is null"));
    }
    
    if (!thread->running.load()) {
        return Err<void>(ThreadDeletionError("Thread is not running"));
    }
    
    try {
        if (timeout_ms == 0) {
            // 无限等待
            pthread_join(thread->thread, nullptr);
        } else {
            // 超时等待 - pthread没有直接超时join，使用轮询
            auto start = std::chrono::steady_clock::now();
            auto timeout_duration = std::chrono::milliseconds(timeout_ms);
            
            while (thread->running.load()) {
                auto elapsed = std::chrono::steady_clock::now() - start;
                if (elapsed >= timeout_duration) {
                    LOGW( "Thread join timeout after %" PRIu32 " ms", timeout_ms);
                    return Err<void>(TimeoutError("Thread join timeout", timeout_ms));
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            pthread_join(thread->thread, nullptr);
        }
        
        return Ok();
        
    } catch (const std::exception& e) {
        LOGE( "Exception joining thread: %s", e.what());
        return Err<void>(ThreadDeletionError("Exception: " + std::string(e.what())));
    }
}

// 检查线程是否运行
Result<bool> laminpie_thread_is_running(laminpie_thread_t *thread) {
    if (thread == nullptr) {
        return Err<bool>(InvalidParameterError("Thread pointer is null"));
    }
    
    return Ok(thread->running.load());
}

// 互斥锁创建
Result<laminpie_mutex_t*> laminpie_mutex_create(void) {
    try {
        auto mutex = new laminpie_mutex_t();
        LOGD( "Mutex created");
        return Ok(mutex);
    } catch (const std::exception& e) {
        LOGE( "Exception creating mutex: %s", e.what());
        return Err<laminpie_mutex_t*>(MutexError("Exception: " + std::string(e.what())));
    }
}

// 互斥锁加锁
Result<void> laminpie_mutex_lock(laminpie_mutex_t *mutex) {
    if (mutex == nullptr) {
        return Err<void>(InvalidParameterError("Mutex pointer is null"));
    }
    
    int result = pthread_mutex_lock(&mutex->mutex);
    if (result != 0) {
        LOGE( "Failed to lock mutex: %s", strerror(result));
        return Err<void>(MutexError("Failed to lock mutex: " + std::string(strerror(result)), result));
    }
    
    mutex->locked = true;
    return Ok();
}

// 互斥锁解锁
Result<void> laminpie_mutex_unlock(laminpie_mutex_t *mutex) {
    if (mutex == nullptr) {
        return Err<void>(InvalidParameterError("Mutex pointer is null"));
    }
    
    int result = pthread_mutex_unlock(&mutex->mutex);
    if (result != 0) {
        LOGE( "Failed to unlock mutex: %s", strerror(result));
        return Err<void>(MutexError("Failed to unlock mutex: " + std::string(strerror(result)), result));
    }
    
    mutex->locked = false;
    return Ok();
}

// 互斥锁删除
Result<void> laminpie_mutex_delete(laminpie_mutex_t *mutex) {
    if (mutex == nullptr) {
        return Err<void>(InvalidParameterError("Mutex pointer is null"));
    }
    
    try {
        delete mutex;
        LOGD( "Mutex deleted");
        return Ok();
    } catch (const std::exception& e) {
        LOGE( "Exception deleting mutex: %s", e.what());
        return Err<void>(MutexError("Exception: " + std::string(e.what())));
    }
}

// 信号量创建
Result<laminpie_semaphore_t*> laminpie_semaphore_create(uint32_t initial_count) {
    try {
        auto semaphore = new laminpie_semaphore_t(initial_count);
        LOGD( "Semaphore created with initial count: %" PRIu32, initial_count);
        return Ok(semaphore);
    } catch (const std::exception& e) {
        LOGE( "Exception creating semaphore: %s", e.what());
        return Err<laminpie_semaphore_t*>(SemaphoreError("Exception: " + std::string(e.what())));
    }
}

// 信号量等待
Result<void> laminpie_semaphore_wait(laminpie_semaphore_t *sem, uint32_t timeout_ms) {
    if (sem == nullptr) {
        return Err<void>(InvalidParameterError("Semaphore pointer is null"));
    }
    
    try {
        int result;
        if (timeout_ms == 0) {
            // 无限等待
            result = sem_wait(&sem->semaphore);
        } else {
            // 超时等待
            struct timespec timeout;
            clock_gettime(CLOCK_REALTIME, &timeout);
            timeout.tv_sec += timeout_ms / 1000;
            timeout.tv_nsec += (timeout_ms % 1000) * 1000000;
            if (timeout.tv_nsec >= 1000000000) {
                timeout.tv_sec++;
                timeout.tv_nsec -= 1000000000;
            }
            
            result = sem_timedwait(&sem->semaphore, &timeout);
        }
        
        if (result != 0) {
            if (errno == ETIMEDOUT) {
                LOGW( "Semaphore wait timeout after %" PRIu32 " ms", timeout_ms);
                return Err<void>(TimeoutError("Semaphore wait timeout", timeout_ms));
            } else {
                LOGE( "Failed to wait for semaphore: %s", strerror(errno));
                return Err<void>(SemaphoreError("Failed to wait for semaphore: " + std::string(strerror(errno)), errno));
            }
        }
        
        {
            pthread_mutex_lock(&sem->count_mutex);
            sem->count--;
            pthread_mutex_unlock(&sem->count_mutex);
        }
        
        return Ok();
        
    } catch (const std::exception& e) {
        LOGE( "Exception waiting for semaphore: %s", e.what());
        return Err<void>(SemaphoreError("Exception: " + std::string(e.what())));
    }
}

// 信号量释放
Result<void> laminpie_semaphore_post(laminpie_semaphore_t *sem) {
    if (sem == nullptr) {
        return Err<void>(InvalidParameterError("Semaphore pointer is null"));
    }
    
    int result = sem_post(&sem->semaphore);
    if (result != 0) {
        LOGE( "Failed to post semaphore: %s", strerror(errno));
        return Err<void>(SemaphoreError("Failed to post semaphore: " + std::string(strerror(errno)), errno));
    }
    
    {
        pthread_mutex_lock(&sem->count_mutex);
        sem->count++;
        pthread_mutex_unlock(&sem->count_mutex);
    }
    
    return Ok();
}

// 信号量删除
Result<void> laminpie_semaphore_delete(laminpie_semaphore_t *sem) {
    if (sem == nullptr) {
        return Err<void>(InvalidParameterError("Semaphore pointer is null"));
    }
    
    try {
        delete sem;
        LOGD( "Semaphore deleted");
        return Ok();
    } catch (const std::exception& e) {
        LOGE( "Exception deleting semaphore: %s", e.what());
        return Err<void>(SemaphoreError("Exception: " + std::string(e.what())));
    }
}

// 全局锁函数
void laminpie_global_lock(void) {
    pthread_mutex_lock(&g_global_mutex);
}

void laminpie_global_unlock(void) {
    pthread_mutex_unlock(&g_global_mutex);
}

Result<void> laminpie_global_lock_isr(void) {
    // 在pthread实现中，ISR锁与普通锁相同
    pthread_mutex_lock(&g_global_mutex);
    return Ok();
}

// 初始化函数
Result<void> laminpie_porting_init(void) {
    LOGI( "Initializing pthread porting layer");
    return Ok();
}

// 清理函数
Result<void> laminpie_porting_cleanup(void) {
    LOGI( "Cleaning up pthread porting layer");
    return Ok();
}