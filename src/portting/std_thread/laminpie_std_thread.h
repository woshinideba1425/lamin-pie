// components/lamin-pie/src/porting/std_thread/laminpie_std_thread.h
#ifndef LAMINPIE_STD_THREAD_H
#define LAMINPIE_STD_THREAD_H
#include <thread>
#include <mutex>
#include <semaphore>
#include <memory>
#include <atomic>
#include "erro_handle.h"
#ifdef __cplusplus
extern "C" {
#endif



typedef struct {
    std::unique_ptr<std::thread> thread;
    void (*callback)(void *);
    void *user_data;
    std::atomic<bool> running;
} laminpie_thread_t;

typedef std::mutex laminpie_mutex_t;

typedef struct {
    std::binary_semaphore sem;
    std::mutex mutex;
    std::atomic<bool> value;
} laminpie_semaphore_t;

#ifdef __cplusplus
}
#endif

#endif /* LAMINPIE_STD_THREAD_H */
