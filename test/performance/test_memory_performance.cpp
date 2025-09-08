#include "test_common.h"

class MemoryPerformanceTest {
public:
    TestResult test_memory_allocation_speed() {
        // 测试内存分配速度
        auto start_time = PLATFORM_GET_TICK_COUNT();
        
        std::vector<std::unique_ptr<system::event::DeviceEvent>> events;
        for (int i = 0; i < 10000; i++) {
            events.push_back(std::make_unique<system::event::DeviceEvent>(
                system::event::Laminpie_Device_Event_Type::kDeviceAdd,
                "perf_device_" + std::to_string(i)
            ));
        }
        
        auto end_time = PLATFORM_GET_TICK_COUNT();
        auto duration = end_time - start_time;
        
        // 10000个对象应该在50ms内分配完成
        TEST_ASSERT(duration < 50);
        
        return TestResult::kPass;
    }
    
    TestResult test_memory_fragmentation() {
        // 测试内存碎片化
        size_t initial_free = PLATFORM_GET_FREE_HEAP();
        
        // 分配和释放不同大小的内存块
        std::vector<std::unique_ptr<char[]>> blocks;
        for (int i = 0; i < 1000; i++) {
            size_t size = (i % 10 + 1) * 100; // 100-1000字节
            blocks.push_back(std::make_unique<char[]>(size));
        }
        
        // 释放一半
        for (size_t i = 0; i < blocks.size(); i += 2) {
            blocks[i].reset();
        }
        
        // 分配新的大块内存
        auto large_block = std::make_unique<char[]>(50000);
        
        size_t final_free = PLATFORM_GET_FREE_HEAP();
        
        // 验证内存使用合理
        TEST_ASSERT(final_free > initial_free * 0.8);
        
        return TestResult::kPass;
    }
};