/**
 * @file test_event_stress_gtest.cpp
 * @brief 事件压力测试 - Google Test版本
 * @author LaminPie Team
 * @date 2024
 */

#include "test_common_gtest.h"
#include "test_platform.h"
#include "laminpie_event_dispatcher.hpp"
#include "interface/device_types.h"

/**
 * @brief 事件压力测试类
 */
class EventStressTest : public LaminPiePerformanceTest {
private:
    laminpie::system::event::LaminPie_EventDispatcher& dispatcher;
    std::atomic<int> event_count{0};
    std::atomic<int> processed_count{0};
    
public:
    EventStressTest() : dispatcher(laminpie::system::event::LaminPie_EventDispatcher::getInstance()) {
        dispatcher.start();
    }
    
    ~EventStressTest() {
        dispatcher.stop();
    }
    
    void SetUp() override {
        LaminPiePerformanceTest::SetUp();
        event_count = 0;
        processed_count = 0;
    }
    
    void TearDown() override {
        LaminPiePerformanceTest::TearDown();
    }
};

/**
 * @brief 测试高频事件处理
 */
TEST_F(EventStressTest, TestHighFrequencyEvents) {
    const int event_count = 10000;
    const uint32_t max_time_us = 1000000; // 1秒
    
    // 注册事件监听器
    uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
        laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
        [this](const laminpie::system::event::DeviceEvent& event) {
            processed_count.fetch_add(1);
        }
    );
    
    // 测量事件分发性能
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 快速发送大量事件
    for (int i = 0; i < event_count; i++) {
        auto test_device = std::make_shared<laminpie::system::device::DeviceInfo>();
        test_device->id = "stress_device_" + std::to_string(i);
        
        laminpie::system::event::DeviceEvent test_event;
        test_event.device = test_device;
        test_event.event_type = laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd;
        
        dispatcher.dispatchEvent(test_event);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto dispatch_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "High frequency event dispatch: " << dispatch_duration.count() << "μs for " 
              << event_count << " events" << std::endl;
    
    EXPECT_LT(dispatch_duration.count(), max_time_us) 
        << "High frequency event dispatch exceeded time limit";
    
    // 等待所有事件处理完成
    bool all_events_processed = WaitForCondition([this, event_count]() {
        return processed_count.load() >= event_count;
    }, 5000);
    
    EXPECT_TRUE(all_events_processed) << "All events should be processed within timeout";
    EXPECT_EQ(processed_count.load(), event_count) 
        << "All " << event_count << " events should be processed";
    
    // 清理监听器
    dispatcher.removeEventListener(listener_id);
}

/**
 * @brief 测试并发事件处理
 */
TEST_F(EventStressTest, TestConcurrentEventHandling) {
    const int thread_count = 8;
    const int events_per_thread = 1000;
    const uint32_t max_time_us = 2000000; // 2秒
    
    // 注册事件监听器
    uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
        laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
        [this](const laminpie::system::event::DeviceEvent& event) {
            processed_count.fetch_add(1);
        }
    );
    
    std::vector<std::thread> threads;
    
    // 测量并发事件处理性能
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 创建多个线程同时发送事件
    for (int t = 0; t < thread_count; t++) {
        threads.emplace_back([this, t, events_per_thread]() {
            for (int i = 0; i < events_per_thread; i++) {
                auto test_device = std::make_shared<laminpie::system::device::DeviceInfo>();
                test_device->id = "concurrent_device_" + std::to_string(t) + "_" + std::to_string(i);
                
                laminpie::system::event::DeviceEvent test_event;
                test_event.device = test_device;
                test_event.event_type = laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd;
                
                dispatcher.dispatchEvent(test_event);
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto concurrent_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "Concurrent event handling: " << concurrent_duration.count() << "μs for " 
              << thread_count * events_per_thread << " events from " << thread_count << " threads" << std::endl;
    
    EXPECT_LT(concurrent_duration.count(), max_time_us) 
        << "Concurrent event handling exceeded time limit";
    
    // 等待所有事件处理完成
    int total_events = thread_count * events_per_thread;
    bool all_events_processed = WaitForCondition([this, total_events]() {
        return processed_count.load() >= total_events;
    }, 10000);
    
    EXPECT_TRUE(all_events_processed) << "All concurrent events should be processed within timeout";
    EXPECT_EQ(processed_count.load(), total_events) 
        << "All " << total_events << " concurrent events should be processed";
    
    // 清理监听器
    dispatcher.removeEventListener(listener_id);
}

/**
 * @brief 测试事件队列压力
 */
TEST_F(EventStressTest, TestEventQueueStress) {
    const int burst_size = 5000;
    const int burst_count = 5;
    
    // 注册事件监听器
    uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
        laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
        [this](const laminpie::system::event::DeviceEvent& event) {
            processed_count.fetch_add(1);
        }
    );
    
    // 发送突发事件
    for (int burst = 0; burst < burst_count; burst++) {
        LOGI("Sending burst %d of %d events", burst + 1, burst_size);
        
        for (int i = 0; i < burst_size; i++) {
            auto test_device = std::make_shared<laminpie::system::device::DeviceInfo>();
            test_device->id = "burst_device_" + std::to_string(burst) + "_" + std::to_string(i);
            
            laminpie::system::event::DeviceEvent test_event;
            test_event.device = test_device;
            test_event.event_type = laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd;
            
            dispatcher.dispatchEvent(test_event);
        }
        
        // 短暂等待，让事件处理
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // 等待所有事件处理完成
    int total_events = burst_count * burst_size;
    bool all_events_processed = WaitForCondition([this, total_events]() {
        return processed_count.load() >= total_events;
    }, 15000);
    
    EXPECT_TRUE(all_events_processed) << "All burst events should be processed within timeout";
    EXPECT_EQ(processed_count.load(), total_events) 
        << "All " << total_events << " burst events should be processed";
    
    LOGI("Event queue stress test completed: %d events processed", 
                processed_count.load());
    
    // 清理监听器
    dispatcher.removeEventListener(listener_id);
}
