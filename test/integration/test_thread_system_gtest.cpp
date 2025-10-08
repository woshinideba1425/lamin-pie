/**
 * @file test_thread_system_gtest.cpp
 * @brief 线程系统测试 - Google Test版本
 * @author LaminPie Team
 * @date 2024
 */

#include "test_common_gtest.h"
#include "test_platform.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

/**
 * @brief 线程系统测试类
 */
class ThreadSystemTest : public LaminPieIntegrationTest {
public:
    void SetUp() override {
        LaminPieIntegrationTest::SetUp();
    }
    
    void TearDown() override {
        LaminPieIntegrationTest::TearDown();
    }
};

/**
 * @brief 测试线程创建和销毁
 */
TEST_F(ThreadSystemTest, TestThreadCreationAndDestruction) {
    std::atomic<bool> thread_started{false};
    std::atomic<bool> thread_finished{false};
    std::atomic<int> thread_counter{0};
    
    // 创建线程
    std::thread test_thread([&]() {
        thread_started = true;
        
        // 模拟一些工作
        for (int i = 0; i < 1000; i++) {
            thread_counter.fetch_add(1);
        }
        
        thread_finished = true;
    });
    
    // 等待线程启动
    bool started = WaitForCondition([&]() { return thread_started.load(); }, 1000);
    EXPECT_TRUE(started) << "Thread should start within timeout";
    
    // 等待线程完成
    test_thread.join();
    
    EXPECT_TRUE(thread_finished.load()) << "Thread should finish execution";
    EXPECT_EQ(thread_counter.load(), 1000) << "Thread should complete all work";
}

/**
 * @brief 测试线程同步
 */
TEST_F(ThreadSystemTest, TestThreadSynchronization) {
    const int thread_count = 4;
    const int work_per_thread = 1000;
    
    std::atomic<int> shared_counter{0};
    std::vector<std::thread> threads;
    
    // 创建多个线程
    for (int t = 0; t < thread_count; t++) {
        threads.emplace_back([&, t, work_per_thread]() {
            for (int i = 0; i < work_per_thread; i++) {
                shared_counter.fetch_add(1);
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    int expected_count = thread_count * work_per_thread;
    EXPECT_EQ(shared_counter.load(), expected_count) 
        << "All threads should contribute to shared counter";
}

/**
 * @brief 测试互斥锁
 */
TEST_F(ThreadSystemTest, TestMutexSynchronization) {
    const int thread_count = 4;
    const int operations_per_thread = 1000;
    
    std::mutex counter_mutex;
    int shared_counter = 0;
    std::vector<std::thread> threads;
    
    // 创建多个线程使用互斥锁
    for (int t = 0; t < thread_count; t++) {
        threads.emplace_back([&, t, operations_per_thread]() {
            for (int i = 0; i < operations_per_thread; i++) {
                std::lock_guard<std::mutex> lock(counter_mutex);
                shared_counter++;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    int expected_count = thread_count * operations_per_thread;
    EXPECT_EQ(shared_counter, expected_count) 
        << "Mutex should protect shared counter correctly";
}

/**
 * @brief 测试条件变量
 */
TEST_F(ThreadSystemTest, TestConditionVariable) {
    std::mutex cv_mutex;
    std::condition_variable cv;
    bool ready = false;
    std::atomic<bool> worker_finished{false};
    
    // 创建工作线程
    std::thread worker_thread([&]() {
        std::unique_lock<std::mutex> lock(cv_mutex);
        
        // 等待条件
        cv.wait(lock, [&]() { return ready; });
        
        // 执行工作
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        worker_finished = true;
    });
    
    // 主线程等待一段时间后通知
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    {
        std::lock_guard<std::mutex> lock(cv_mutex);
        ready = true;
    }
    cv.notify_one();
    
    // 等待工作线程完成
    worker_thread.join();
    
    EXPECT_TRUE(worker_finished.load()) << "Worker thread should finish after notification";
}

/**
 * @brief 测试线程性能
 */
TEST_F(ThreadSystemTest, TestThreadPerformance) {
    const int thread_count = 8;
    const int work_per_thread = 10000;
    const uint32_t max_time_us = 100000; // 100ms
    
    std::atomic<int> shared_counter{0};
    std::vector<std::thread> threads;
    
    // 测量多线程性能
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 创建多个线程
    for (int t = 0; t < thread_count; t++) {
        threads.emplace_back([&, t, work_per_thread]() {
            for (int i = 0; i < work_per_thread; i++) {
                shared_counter.fetch_add(1);
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "Thread performance: " << duration.count() << "μs for " 
              << thread_count << " threads, " << work_per_thread << " operations each" << std::endl;
    
    EXPECT_LT(duration.count(), max_time_us) 
        << "Thread performance test exceeded time limit";
    
    int expected_count = thread_count * work_per_thread;
    EXPECT_EQ(shared_counter.load(), expected_count) 
        << "All threads should complete their work";
}

/**
 * @brief 测试线程池模拟
 */
TEST_F(ThreadSystemTest, TestThreadPoolSimulation) {
    const int task_count = 100;
    const int max_concurrent_threads = 4;
    
    std::atomic<int> completed_tasks{0};
    std::atomic<int> active_threads{0};
    std::vector<std::thread> threads;
    
    // 模拟线程池
    for (int t = 0; t < max_concurrent_threads; t++) {
        threads.emplace_back([&]() {
            while (completed_tasks.load() < task_count) {
                active_threads.fetch_add(1);
                
                // 模拟任务执行
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                
                int current_task = completed_tasks.fetch_add(1);
                if (current_task >= task_count) {
                    completed_tasks.fetch_sub(1); // 回退，因为已经超过了
                    break;
                }
                
                active_threads.fetch_sub(1);
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_GE(completed_tasks.load(), task_count) 
        << "All tasks should be completed";
    
    EXPECT_EQ(active_threads.load(), 0) 
        << "No threads should be active after completion";
    
    LOGI("Thread pool simulation completed: %d tasks processed", 
                completed_tasks.load());
}
