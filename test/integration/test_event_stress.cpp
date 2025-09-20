#include "test_common.h"
#include "test_platform.h"
#include "laminpie_event_dispatcher.hpp"
#include "interface/device_types.h"

class EventStressTest {
public:
    TestResult test_high_frequency_events() {
        // 测试高频事件处理
        auto& dispatcher = laminpie::system::event::LaminPie_EventDispatcher::getInstance();
        dispatcher.start();
        
        std::atomic<int> event_count{0};
        const int total_events = 1000; // 减少事件数量，避免内存压力
        
        printf("DEBUG: Starting high frequency test with %d events\n", total_events);
        
        // 注册监听器
        uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [&event_count](const laminpie::system::event::DeviceEvent& event) {
                event_count++;
                return true;
            }
        );
        
        printf("DEBUG: Stress listener registered with ID: %lu\n", listener_id);
        
        // 发送大量事件
        auto start_time = PLATFORM_GET_TICK_COUNT();
        for (int i = 0; i < total_events; i++) {
            auto device_id = std::make_shared<DeviceIdentifier>(
                DeviceIdentifier::BusType::BUS_I2C, 
                "stress_device_" + std::to_string(i)
            );
            auto event = std::make_shared<laminpie::system::event::DeviceEvent>(
                laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
                device_id
            );
            dispatcher.postEvent(event);
        }
        
        printf("DEBUG: Posted %d events, waiting for processing...\n", total_events);
        
        // 等待所有事件处理完成
        int timeout_count = 0;
        while (event_count.load() < total_events && timeout_count < 1000) {
            PLATFORM_DELAY_MS(10);
            timeout_count++;
        }
        
        auto end_time = PLATFORM_GET_TICK_COUNT();
        auto duration = end_time - start_time;
        
        printf("DEBUG: Processed %d/%d events in %lld ms\n", 
               event_count.load(), total_events, duration);
        
        // 清理监听器
        dispatcher.removeEventListener(listener_id);
        printf("DEBUG: Stress listener removed\n");
        
        TEST_ASSERT(event_count.load() == total_events);
        TEST_ASSERT(duration < 5000); // 5秒内完成
        
        dispatcher.stop();
        return TestResult::kPass;
    }
    
    TestResult test_concurrent_event_handling() {
        // 测试并发事件处理
        auto& dispatcher = laminpie::system::event::LaminPie_EventDispatcher::getInstance();
        dispatcher.start();
        
        std::atomic<int> event_count{0};
        const int thread_count = 5; // 减少线程数量
        const int events_per_thread = 100; // 减少每个线程的事件数量
        
        printf("DEBUG: Starting concurrent test with %d threads, %d events each\n", 
               thread_count, events_per_thread);
        
        // 注册监听器
        uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [&event_count](const laminpie::system::event::DeviceEvent& event) {
                event_count++;
                return true;
            }
        );
        
        printf("DEBUG: Concurrent listener registered with ID: %lu\n", listener_id);
        
        // 创建多个线程发送事件
        std::vector<std::thread> threads;
        for (int t = 0; t < thread_count; t++) {
            threads.emplace_back([&dispatcher, t, events_per_thread]() {
                for (int i = 0; i < events_per_thread; i++) {
                    auto device_id = std::make_shared<DeviceIdentifier>(
                        DeviceIdentifier::BusType::BUS_I2C, 
                        "concurrent_device_" + std::to_string(t) + "_" + std::to_string(i)
                    );
                    auto event = std::make_shared<laminpie::system::event::DeviceEvent>(
                        laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
                        device_id
                    );
                    dispatcher.postEvent(event);
                }
            });
        }
        
        // 等待所有线程完成
        for (auto& thread : threads) {
            thread.join();
        }
        
        printf("DEBUG: All threads completed, waiting for event processing...\n");
        
        // 等待所有事件处理完成
        int timeout_count = 0;
        int expected_events = thread_count * events_per_thread;
        while (event_count.load() < expected_events && timeout_count < 1000) {
            PLATFORM_DELAY_MS(10);
            timeout_count++;
        }
        
        printf("DEBUG: Processed %d/%d concurrent events\n", 
               event_count.load(), expected_events);
        
        // 清理监听器
        dispatcher.removeEventListener(listener_id);
        printf("DEBUG: Concurrent listener removed\n");
        
        TEST_ASSERT(event_count.load() == expected_events);
        
        dispatcher.stop();
        return TestResult::kPass;
    }
};