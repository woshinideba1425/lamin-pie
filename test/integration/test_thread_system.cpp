#include "test_common.h"
#include "test_platform.h"
#include "laminpie_thread.h"
#include <atomic>
#include <vector>
#include <mutex>
#include <chrono>
#include <thread>

// 线程回调函数
void thread_callback_1(void* data) {
    std::atomic<bool>* flag = static_cast<std::atomic<bool>*>(data);
    *flag = true;
}

void thread_callback_2(void* data) {
    std::vector<int>* execution_order = static_cast<std::vector<int>*>(data);
    execution_order->push_back(1);
}

void thread_callback_3(void* data) {
    std::vector<int>* execution_order = static_cast<std::vector<int>*>(data);
    execution_order->push_back(2);
}

class ThreadSystemTest {
public:
    TestResult test_thread_creation_and_destruction() {
        // 测试线程创建和销毁
        std::atomic<bool> thread_executed{false};
        
        auto result = laminpie_thread_create(
            LAMINPIE_THREAD_PRIO_MID,
            thread_callback_1,
            4096,
            &thread_executed
        );
        
        TEST_ASSERT(result.is_ok());
        
        // 等待线程执行
        int timeout_count = 0;
        while (!thread_executed.load() && timeout_count < 100) {
            PLATFORM_DELAY_MS(10);
            timeout_count++;
        }
        
        TEST_ASSERT(thread_executed.load());
        
        // 等待线程结束
        auto join_result = laminpie_thread_join(result.unwrap(), 1000);
        TEST_ASSERT(join_result.is_ok());
        
        // 销毁线程
        auto delete_result = laminpie_thread_delete(result.unwrap());
        TEST_ASSERT(delete_result.is_ok());
        
        return TestResult::kPass;
    }
    
    TestResult test_thread_priority() {
        // 测试线程优先级
        std::vector<int> execution_order;
        
        // 创建低优先级线程
        auto low_prio_result = laminpie_thread_create(
            LAMINPIE_THREAD_PRIO_LOW,
            thread_callback_2,
            4096,
            &execution_order
        );
        
        TEST_ASSERT(low_prio_result.is_ok());
        
        // 创建高优先级线程
        auto high_prio_result = laminpie_thread_create(
            LAMINPIE_THREAD_PRIO_HIGH,
            thread_callback_3,
            4096,
            &execution_order
        );
        
        TEST_ASSERT(high_prio_result.is_ok());
        
        // 等待线程完成
        PLATFORM_DELAY_MS(200);
        
        // 验证执行顺序（高优先级应该先执行）
        TEST_ASSERT(execution_order.size() == 2);
        TEST_ASSERT(execution_order[0] == 2); // 高优先级先执行
        TEST_ASSERT(execution_order[1] == 1); // 低优先级后执行
        
        // 等待线程结束
        auto low_join_result = laminpie_thread_join(low_prio_result.unwrap(), 1000);
        TEST_ASSERT(low_join_result.is_ok());
        
        auto high_join_result = laminpie_thread_join(high_prio_result.unwrap(), 1000);
        TEST_ASSERT(high_join_result.is_ok());
        
        // 清理
        auto low_delete_result = laminpie_thread_delete(low_prio_result.unwrap());
        TEST_ASSERT(low_delete_result.is_ok());
        
        auto high_delete_result = laminpie_thread_delete(high_prio_result.unwrap());
        TEST_ASSERT(high_delete_result.is_ok());
        
        return TestResult::kPass;
    }
};