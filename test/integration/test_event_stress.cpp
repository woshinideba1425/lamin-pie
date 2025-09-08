#include "test_common.h"
#include "laminpie_event_dispatcher.hpp"

class EventStressTest {
public:
    TestResult test_high_frequency_events() {
        // 测试高频事件处理
        auto& dispatcher = system::event::LaminPie_EventDispatcher::getInstance();
        dispatcher.start();
        
        std::atomic<int> event_count{0};
        const int total_events = 10000;
        
        // 注册监听器
        auto listener_id = dispatcher.addEventListener<system::event::DeviceEvent>(
            system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [&event_count](const system::event::DeviceEvent& event) {
                event_count++;
                return true;
            }
        );
        
        // 发送大量事件
        auto start_time = PLATFORM_GET_TICK_COUNT();
        for (int i = 0; i < total_events; i++) {
            auto event = system::event::DeviceEvent(
                system::event::Laminpie_Device_Event_Type::kDeviceAdd,
                "stress_device_" + std::to_string(i)
            );
            dispatcher.postEvent(event);
        }
        
        // 等待所有事件处理完成
        while (event_count.load() < total_events) {
            PLATFORM_DELAY_MS(10);
        }
        
        auto end_time = PLATFORM_GET_TICK_COUNT();
        auto duration = end_time - start_time;
        
        TEST_ASSERT(event_count.load() == total_events);
        TEST_ASSERT(duration < 5000); // 5秒内完成
        
        dispatcher.stop();
        return TestResult::kPass;
    }
    
    TestResult test_concurrent_event_handling() {
        // 测试并发事件处理
        auto& dispatcher = system::event::LaminPie_EventDispatcher::getInstance();
        dispatcher.start();
        
        std::atomic<int> event_count{0};
        const int thread_count = 10;
        const int events_per_thread = 1000;
        
        // 注册监听器
        auto listener_id = dispatcher.addEventListener<system::event::DeviceEvent>(
            system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [&event_count](const system::event::DeviceEvent& event) {
                event_count++;
                return true;
            }
        );
        
        // 创建多个线程发送事件
        std::vector<std::thread> threads;
        for (int t = 0; t < thread_count; t++) {
            threads.emplace_back([&dispatcher, t, events_per_thread]() {
                for (int i = 0; i < events_per_thread; i++) {
                    auto event = system::event::DeviceEvent(
                        system::event::Laminpie_Device_Event_Type::kDeviceAdd,
                        "concurrent_device_" + std::to_string(t) + "_" + std::to_string(i)
                    );
                    dispatcher.postEvent(event);
                }
            });
        }
        
        // 等待所有线程完成
        for (auto& thread : threads) {
            thread.join();
        }
        
        // 等待所有事件处理完成
        while (event_count.load() < thread_count * events_per_thread) {
            PLATFORM_DELAY_MS(10);
        }
        
        TEST_ASSERT(event_count.load() == thread_count * events_per_thread);
        
        dispatcher.stop();
        return TestResult::kPass;
    }
};