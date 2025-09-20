#include "laminpie_log.hpp"
#include <system_error>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <semaphore>
#include <memory>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cinttypes>
#include "laminpie_thread_errors.h"
#include "laminpie_result.h"

using namespace laminate;
using namespace laminate::threading;

// 添加通用的Err函数模板 - 修复模板参数问题
template<typename T, typename E>
Result<T> Err(E error) {
    return Result<T>(std::make_shared<E>(std::move(error)));
}

struct laminpie_thread_t {
    std::unique_ptr<std::thread> thread;
    void (*callback)(void *);
    void *user_data;
    std::atomic<bool> running;
    std::atomic<bool> should_stop;
    std::mutex state_mutex;
    std::condition_variable state_cv;
    
    laminpie_thread_t() : callback(nullptr), user_data(nullptr), 
                         running(false), should_stop(false) {}
    
    ~laminpie_thread_t() {
        if (thread && thread->joinable()) {
            should_stop = true;
            state_cv.notify_all();
            thread->join();
        }
    }
};

struct laminpie_mutex_t {
    std::mutex mutex;
    std::atomic<bool> locked;
    
    laminpie_mutex_t() : locked(false) {}
};

struct laminpie_semaphore_t {
    std::counting_semaphore<> semaphore;
    std::atomic<uint32_t> count;
    std::mutex count_mutex;
    
    laminpie_semaphore_t(uint32_t initial_count) 
        : semaphore(static_cast<ptrdiff_t>(initial_count)), count(initial_count) {}
};
std::mutex g_global_mutex;

// 线程创建函数
Result<laminpie_thread_t*> laminpie_thread_create(
    laminpie_thread_prio_t prio,
    void (*callback)(void *), 
    size_t stack_size, 
    void *user_data) {
    
    LP_LOG_INFO("THREAD_PORTING", "Creating thread with priority: %d", prio);
    
    // 参数验证
    if (callback == nullptr) {
        LP_LOG_ERROR("THREAD_PORTING", "Thread callback is null");
        return Err<laminpie_thread_t*>(InvalidParameterError("Thread callback cannot be null"));
    }
    
    if (stack_size == 0) {
        stack_size = 8192; // 默认栈大小
        LP_LOG_WARN("THREAD_PORTING", "Using default stack size: %zu", stack_size);
    }
    
    try {
        // 创建线程对象
        auto thread_obj = std::make_unique<laminpie_thread_t>();
        thread_obj->callback = callback;
        thread_obj->user_data = user_data;
        thread_obj->running = false;
        thread_obj->should_stop = false;
        
        // 创建线程 - 使用移动语义避免拷贝问题
        auto thread_ptr = thread_obj.get();
        LP_LOG_DEBUG("THREAD_PORTING", "About to create thread, thread_ptr=%p", thread_ptr);
        LP_LOG_DEBUG("THREAD_PORTING", "Thread callback=%p, user_data=%p", 
                    (void*)thread_ptr->callback, thread_ptr->user_data);
        
        thread_obj->thread = std::make_unique<std::thread>([thread_ptr]() {
            LP_LOG_DEBUG("THREAD_PORTING", "Thread lambda started, thread_ptr=%p", thread_ptr);
            
            // 设置运行状态
            LP_LOG_DEBUG("THREAD_PORTING", "Setting running=true");
            thread_ptr->running = true;
            LP_LOG_DEBUG("THREAD_PORTING", "Running flag set, value=%d", thread_ptr->running.load());
            
            // 执行用户回调
            LP_LOG_DEBUG("THREAD_PORTING", "About to call user callback");
            try {
                if (thread_ptr->callback) {
                    thread_ptr->callback(thread_ptr->user_data);
                    LP_LOG_DEBUG("THREAD_PORTING", "User callback completed successfully");
                } else {
                    LP_LOG_ERROR("THREAD_PORTING", "Callback is null!");
                }
            } catch (const std::exception& e) {
                LP_LOG_ERROR("THREAD_PORTING", "Thread callback exception: %s", e.what());
            } catch (...) {
                LP_LOG_ERROR("THREAD_PORTING", "Thread callback unknown exception");
            }
            
            // 等待被显式停止，而不是立即退出
            LP_LOG_DEBUG("THREAD_PORTING", "Callback completed, waiting for stop signal...");
            std::unique_lock<std::mutex> lock(thread_ptr->state_mutex);
            thread_ptr->state_cv.wait(lock, [thread_ptr] { 
                return thread_ptr->should_stop.load(); 
            });
            
            // 线程结束
            LP_LOG_DEBUG("THREAD_PORTING", "Setting running=false");
            thread_ptr->running = false;
            
            LP_LOG_DEBUG("THREAD_PORTING", "Thread finished");
        });
        
        LP_LOG_DEBUG("THREAD_PORTING", "Thread object created, checking if joinable: %s", 
                    thread_obj->thread->joinable() ? "true" : "false");
        
        // 等待线程启动 - 使用简单的轮询方式
        LP_LOG_DEBUG("THREAD_PORTING", "Waiting for thread to start...");
        
        // 轮询等待线程启动，最多等待1秒
        int retry_count = 0;
        const int max_retries = 100; // 100 * 10ms = 1秒
        bool started = false;
        
        while (retry_count < max_retries && !started) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            started = thread_ptr->running.load();
            retry_count++;
            
            if (retry_count % 10 == 0) { // 每100ms打印一次状态
                LP_LOG_DEBUG("THREAD_PORTING", "Waiting for thread start... retry %d/%d, running=%d", 
                            retry_count, max_retries, started);
            }
        }
        
        if (!started) {
            LP_LOG_ERROR("THREAD_PORTING", "Thread failed to start after %d retries - running flag is false", retry_count);
            LP_LOG_ERROR("THREAD_PORTING", "Thread joinable: %s", 
                        thread_obj->thread->joinable() ? "true" : "false");
            return Err<laminpie_thread_t*>(ThreadCreationError("Thread failed to start after timeout"));
        }
        
        LP_LOG_DEBUG("THREAD_PORTING", "Thread started successfully after %d retries, running status: %d", 
                    retry_count, thread_ptr->running.load());
        
        LP_LOG_INFO("THREAD_PORTING", "Thread created successfully");
        return Ok(thread_obj.release());
        
    } catch (const std::system_error& e) {
        LP_LOG_ERROR("THREAD_PORTING", "System error creating thread: %s", e.what());
        return Err<laminpie_thread_t*>(ThreadCreationError("System error: " + std::string(e.what()), e.code().value()));
    } catch (const std::exception& e) {
        LP_LOG_ERROR("THREAD_PORTING", "Exception creating thread: %s", e.what());
        return Err<laminpie_thread_t*>(ThreadCreationError("Exception: " + std::string(e.what())));
    } catch (...) {
        LP_LOG_ERROR("THREAD_PORTING", "Unknown error creating thread");
        return Err<laminpie_thread_t*>(ThreadCreationError("Unknown error creating thread"));
    }
}

// 线程删除函数
Result<void> laminpie_thread_delete(laminpie_thread_t *thread) {
    if (thread == nullptr) {
        LP_LOG_WARN("THREAD_PORTING", "Attempting to delete null thread");
        return Err<void>(InvalidParameterError("Thread pointer is null"));
    }
    
    LP_LOG_DEBUG("THREAD_PORTING", "Deleting thread");
    
    try {
        // 设置停止标志
        thread->should_stop = true;
        thread->state_cv.notify_all();
        
        // 等待线程结束
        if (thread->thread && thread->thread->joinable()) {
            thread->thread->join();
        }
        
        // 删除线程对象
        delete thread;
        
        LP_LOG_DEBUG("THREAD_PORTING", "Thread deleted successfully");
        return Ok();
        
    } catch (const std::exception& e) {
        LP_LOG_ERROR("THREAD_PORTING", "Exception deleting thread: %s", e.what());
        return Err<void>(ThreadDeletionError("Exception: " + std::string(e.what())));
    } catch (...) {
        LP_LOG_ERROR("THREAD_PORTING", "Unknown error deleting thread");
        return Err<void>(ThreadDeletionError("Unknown error deleting thread"));
    }
}

// 线程等待函数
Result<void> laminpie_thread_join(laminpie_thread_t *thread, uint32_t timeout_ms) {
    if (thread == nullptr) {
        return Err<void>(InvalidParameterError("Thread pointer is null"));
    }
    
    if (!thread->thread || !thread->thread->joinable()) {
        return Err<void>(ThreadDeletionError("Thread is not joinable"));
    }
    
    try {
        // 发送停止信号
        LP_LOG_DEBUG("THREAD_PORTING", "Sending stop signal to thread");
        thread->should_stop = true;
        thread->state_cv.notify_all();
        
        if (timeout_ms == 0) {
            // 无限等待
            thread->thread->join();
        } else {
            // 超时等待
            auto start = std::chrono::steady_clock::now();
            auto timeout_duration = std::chrono::milliseconds(timeout_ms);
            
            while (thread->thread->joinable() && thread->running.load()) {
                auto elapsed = std::chrono::steady_clock::now() - start;
                if (elapsed >= timeout_duration) {
                    LP_LOG_WARN("THREAD_PORTING", "Thread join timeout after %" PRIu32 " ms", timeout_ms);
                    return Err<void>(TimeoutError("Thread join timeout", timeout_ms));
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (thread->thread->joinable()) {
                thread->thread->join();
            }
        }
        
        return Ok();
        
    } catch (const std::exception& e) {
        LP_LOG_ERROR("THREAD_PORTING", "Exception joining thread: %s", e.what());
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
        LP_LOG_DEBUG("THREAD_PORTING", "Mutex created");
        return Ok(mutex);
    } catch (const std::exception& e) {
        LP_LOG_ERROR("THREAD_PORTING", "Exception creating mutex: %s", e.what());
        return Err<laminpie_mutex_t*>(MutexError("Exception: " + std::string(e.what())));
    }
}

// 互斥锁加锁
Result<void> laminpie_mutex_lock(laminpie_mutex_t *mutex) {
    if (mutex == nullptr) {
        return Err<void>(InvalidParameterError("Mutex pointer is null"));
    }
    
    try {
        mutex->mutex.lock();
        mutex->locked = true;
        LP_LOG_TRACE("THREAD_PORTING", "Mutex locked");
        return Ok();
    } catch (const std::exception& e) {
        LP_LOG_ERROR("THREAD_PORTING", "Exception locking mutex: %s", e.what());
        return Err<void>(MutexError("Exception: " + std::string(e.what())));
    }
}

// 互斥锁解锁
Result<void> laminpie_mutex_unlock(laminpie_mutex_t *mutex) {
    if (mutex == nullptr) {
        return Err<void>(InvalidParameterError("Mutex pointer is null"));
    }
    
    try {
        mutex->mutex.unlock();
        mutex->locked = false;
        LP_LOG_TRACE("THREAD_PORTING", "Mutex unlocked");
        return Ok();
    } catch (const std::exception& e) {
        LP_LOG_ERROR("THREAD_PORTING", "Exception unlocking mutex: %s", e.what());
        return Err<void>(MutexError("Exception: " + std::string(e.what())));
    }
}

// 互斥锁删除
Result<void> laminpie_mutex_delete(laminpie_mutex_t *mutex) {
    if (mutex == nullptr) {
        return Err<void>(InvalidParameterError("Mutex pointer is null"));
    }
    
    try {
        delete mutex;
        LP_LOG_DEBUG("THREAD_PORTING", "Mutex deleted");
        return Ok();
    } catch (const std::exception& e) {
        LP_LOG_ERROR("THREAD_PORTING", "Exception deleting mutex: %s", e.what());
        return Err<void>(MutexError("Exception: " + std::string(e.what())));
    }
}

// 信号量创建
Result<laminpie_semaphore_t*> laminpie_semaphore_create(uint32_t initial_count) {
    try {
        auto semaphore = new laminpie_semaphore_t(initial_count);
        LP_LOG_DEBUG("THREAD_PORTING", "Semaphore created with initial count: %" PRIu32, initial_count);
        return Ok(semaphore);
    } catch (const std::exception& e) {
        LP_LOG_ERROR("THREAD_PORTING", "Exception creating semaphore: %s", e.what());
        return Err<laminpie_semaphore_t*>(SemaphoreError("Exception: " + std::string(e.what())));
    }
}

// 信号量等待
Result<void> laminpie_semaphore_wait(laminpie_semaphore_t *sem, uint32_t timeout_ms) {
    if (sem == nullptr) {
        return Err<void>(InvalidParameterError("Semaphore pointer is null"));
    }
    
    try {
        if (timeout_ms == 0) {
            // 无限等待
            sem->semaphore.acquire();
        } else {
            // 超时等待
            auto timeout_duration = std::chrono::milliseconds(timeout_ms);
            if (!sem->semaphore.try_acquire_for(timeout_duration)) {
                LP_LOG_WARN("THREAD_PORTING", "Semaphore wait timeout after %" PRIu32 " ms", timeout_ms);
                return Err<void>(TimeoutError("Semaphore wait timeout", timeout_ms));
            }
        }
        
        {
            std::lock_guard<std::mutex> lock(sem->count_mutex);
            sem->count--;
        }
        
        LP_LOG_TRACE("THREAD_PORTING", "Semaphore acquired, count: %" PRIu32, sem->count.load());
        return Ok();
        
    } catch (const std::exception& e) {
        LP_LOG_ERROR("THREAD_PORTING", "Exception waiting for semaphore: %s", e.what());
        return Err<void>(SemaphoreError("Exception: " + std::string(e.what())));
    }
}

// 信号量释放
Result<void> laminpie_semaphore_post(laminpie_semaphore_t *sem) {
    if (sem == nullptr) {
        return Err<void>(InvalidParameterError("Semaphore pointer is null"));
    }
    
    try {
        sem->semaphore.release();
        
        {
            std::lock_guard<std::mutex> lock(sem->count_mutex);
            sem->count++;
        }
        
        LP_LOG_TRACE("THREAD_PORTING", "Semaphore released, count: %" PRIu32, sem->count.load());
        return Ok();
        
    } catch (const std::exception& e) {
        LP_LOG_ERROR("THREAD_PORTING", "Exception posting semaphore: %s", e.what());
        return Err<void>(SemaphoreError("Exception: " + std::string(e.what())));
    }
}

// 信号量删除
Result<void> laminpie_semaphore_delete(laminpie_semaphore_t *sem) {
    if (sem == nullptr) {
        return Err<void>(InvalidParameterError("Semaphore pointer is null"));
    }
    
    try {
        delete sem;
        LP_LOG_DEBUG("THREAD_PORTING", "Semaphore deleted");
        return Ok();
    } catch (const std::exception& e) {
        LP_LOG_ERROR("THREAD_PORTING", "Exception deleting semaphore: %s", e.what());
        return Err<void>(SemaphoreError("Exception: " + std::string(e.what())));
    }
}

// 全局锁函数
void laminpie_global_lock(void) {
    g_global_mutex.lock();
    LP_LOG_TRACE("THREAD_PORTING", "Global lock acquired");
}

void laminpie_global_unlock(void) {
    g_global_mutex.unlock();
    LP_LOG_TRACE("THREAD_PORTING", "Global lock released");
}

Result<void> laminpie_global_lock_isr(void) {
    // 在标准库实现中，ISR锁与普通锁相同
    g_global_mutex.lock();
    LP_LOG_TRACE("THREAD_PORTING", "Global ISR lock acquired");
    return Ok();
}

// 初始化函数
Result<void> laminpie_porting_init(void) {
    LP_LOG_INFO("THREAD_PORTING", "Initializing std_thread porting layer");
    return Ok();
}

// 清理函数
Result<void> laminpie_porting_cleanup(void) {
    LP_LOG_INFO("THREAD_PORTING", "Cleaning up std_thread porting layer");
    return Ok();
}

