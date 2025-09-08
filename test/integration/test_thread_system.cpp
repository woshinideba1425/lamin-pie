#include "test_common.h"
#include "laminpie_thread.h"

class ThreadSystemTest {
public:
    TestResult test_thread_creation_and_destruction() {
        // 测试线程创建和销毁
        std::atomic<bool> thread_executed{false};
        
        auto result = laminpie_thread_create(
            LAMINPIE_THREAD_PRIO_NORMAL,
            [](void* data) {
                std::atomic<bool>* flag = static_cast<std::atomic<bool>*>(data);
                *flag = true;
            },
            4096,
            &thread_executed
        );
        
        TEST_ASSERT(result.is_ok());
        
        // 等待线程执行
        while (!thread_executed.load()) {
            PLATFORM_DELAY_MS(10);
        }
        
        // 销毁线程
        auto delete_result = laminpie_thread_delete(result.value());
        TEST_ASSERT(delete_result.is_ok());
        
        return TestResult::kPass;
    }
    
    TestResult test_thread_priority() {
        // 测试线程优先级
        std::vector<int> execution_order;
        std::mutex order_mutex;
        
        // 创建低优先级线程
        auto low_prio_result = laminpie_thread_create(
            LAMINPIE_THREAD_PRIO_LOW,
            [&execution_order, &order_mutex](void* data) {
                std::lock_guard<std::mutex> lock(order_mutex);
                execution_order.push_back(1);
            },
            4096,
            nullptr
        );
        
        // 创建高优先级线程
        auto high_prio_result = laminpie_thread_create(
            LAMINPIE_THREAD_PRIO_HIGH,
            [&execution_order, &order_mutex](void* data) {
                std::lock_guard<std::mutex> lock(order_mutex);
                execution_order.push_back(2);
            },
            4096,
            nullptr
        );
        
        // 等待线程完成
        PLATFORM_DELAY_MS(100);
        
        // 验证执行顺序（高优先级应该先执行）
        TEST_ASSERT(execution_order.size() == 2);
        TEST_ASSERT(execution_order[0] == 2); // 高优先级先执行
        TEST_ASSERT(execution_order[1] == 1); // 低优先级后执行
        
        // 清理
        laminpie_thread_delete(low_prio_result.value());
        laminpie_thread_delete(high_prio_result.value());
        
        return TestResult::kPass;
    }
};