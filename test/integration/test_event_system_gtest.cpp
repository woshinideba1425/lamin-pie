/**
 * @file test_event_system_gtest.cpp
 * @brief 事件系统测试 - Google Test版本
 * @author LaminPie Team
 * @date 2024
 */

#include "test_common_gtest.h"
#include "test_platform.h"
#include "laminpie_event_dispatcher.hpp"
#include "laminpie_system_event_type.hpp"
#include "interface/device_types.h"

/**
 * @brief 事件系统测试类
 */
class EventSystemTest : public LaminPieIntegrationTest {
protected:
    laminpie::system::event::LaminPie_EventDispatcher& dispatcher;
    std::vector<laminpie::system::event::DeviceEvent> received_events;
    std::mutex events_mutex;
    
public:
    EventSystemTest() : dispatcher(laminpie::system::event::LaminPie_EventDispatcher::getInstance()) {
        dispatcher.start();
    }
    
    ~EventSystemTest() {
        dispatcher.stop();
    }
    
    void SetUp() override {
        LaminPieIntegrationTest::SetUp();
        received_events.clear();
    }
    
    void TearDown() override {
        received_events.clear();
        LaminPieIntegrationTest::TearDown();
    }
    
    // 事件回调函数
    void OnDeviceEvent(const laminpie::system::event::DeviceEvent& event) {
        std::lock_guard<std::mutex> lock(events_mutex);
        received_events.push_back(event);
    }
    
    // 获取接收到的事件数量
    size_t GetReceivedEventCount() {
        std::lock_guard<std::mutex> lock(events_mutex);
        return received_events.size();
    }
    
    // 清空接收到的事件
    void ClearReceivedEvents() {
        std::lock_guard<std::mutex> lock(events_mutex);
        received_events.clear();
    }
};

/**
 * @brief 测试事件监听器注册
 */
TEST_F(EventSystemTest, TestEventRegistration) {
    // 注册事件监听器
    uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
        laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
        [this](const laminpie::system::event::DeviceEvent& event) {
            OnDeviceEvent(event);
        }
    );
    
    // 验证监听器已注册
    EXPECT_GT(dispatcher.getListenerCount(), 0) << "Event listener should be registered";
    
    // 清理注册的监听器
    dispatcher.removeEventListener(listener_id);
    
    SUCCEED();
}

/**
 * @brief 测试事件分发
 */
TEST_F(EventSystemTest, TestEventDispatch) {
    // 注册监听器
    uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
        laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
        [this](const laminpie::system::event::DeviceEvent& event) {
            OnDeviceEvent(event);
        }
    );
    
    // 创建测试设备事件
    auto test_device = std::make_shared<DeviceIdentifier>(
        DeviceIdentifier::BusType::BUS_I2C, "test_device_001");
    
    laminpie::system::event::DeviceEvent test_event(
        laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd, 
        test_device);
    
    // 分发事件
    dispatcher.dispatchEvent(test_event);
    
    // 等待事件处理完成
    bool event_received = WaitForCondition([this]() {
        return GetReceivedEventCount() > 0;
    }, 1000);
    
    EXPECT_TRUE(event_received) << "Event should be received within timeout";
    EXPECT_EQ(GetReceivedEventCount(), 1) << "Should receive exactly one event";
    
    // 验证事件内容
    if (GetReceivedEventCount() > 0) {
        std::lock_guard<std::mutex> lock(events_mutex);
        const auto& received_event = received_events[0];
        EXPECT_EQ(received_event.device->id, "test_device_001") << "Device ID should match";
        EXPECT_EQ(received_event.type, laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd) 
            << "Event type should match";
    }
    
    // 清理监听器
    dispatcher.removeEventListener(listener_id);
}

/**
 * @brief 测试异步事件处理
 */
TEST_F(EventSystemTest, TestAsyncEventProcessing) {
    const int event_count = 10;
    
    // 注册监听器
    uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
        laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
        [this](const laminpie::system::event::DeviceEvent& event) {
            OnDeviceEvent(event);
        }
    );
    
    // 发送多个事件
    for (int i = 0; i < event_count; i++) {
        auto test_device = std::make_shared<DeviceIdentifier>(
            DeviceIdentifier::BusType::BUS_I2C, "async_device_" + std::to_string(i));
        
        laminpie::system::event::DeviceEvent test_event(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd, 
            test_device);
        
        dispatcher.dispatchEvent(test_event);
    }
    
    // 等待所有事件处理完成
    bool all_events_received = WaitForCondition([this]() {
        return GetReceivedEventCount() >= event_count;
    }, 2000);
    
    EXPECT_TRUE(all_events_received) << "All events should be received within timeout";
    EXPECT_EQ(GetReceivedEventCount(), event_count) << "Should receive all " << event_count << " events";
    
    // 清理监听器
    dispatcher.removeEventListener(listener_id);
}

/**
 * @brief 测试事件监听器移除
 */
TEST_F(EventSystemTest, TestEventListenerRemoval) {
    // 注册监听器
    uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
        laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
        [this](const laminpie::system::event::DeviceEvent& event) {
            OnDeviceEvent(event);
        }
    );
    
    size_t initial_listener_count = dispatcher.getListenerCount();
    
    // 移除监听器
    dispatcher.removeEventListener(listener_id);
    
    // 验证监听器已移除
    EXPECT_LT(dispatcher.getListenerCount(), initial_listener_count) 
        << "Listener count should decrease after removal";
    
    // 发送事件，应该不会被接收
    auto test_device = std::make_shared<DeviceIdentifier>(
        DeviceIdentifier::BusType::BUS_I2C, "removed_listener_test");
    
    laminpie::system::event::DeviceEvent test_event(
        laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd, 
        test_device);
    
    dispatcher.dispatchEvent(test_event);
    
    // 等待一段时间，确保事件不会被处理
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(GetReceivedEventCount(), 0) << "No events should be received after listener removal";
}

/**
 * @brief 测试事件系统性能
 */
TEST_F(EventSystemTest, TestEventSystemPerformance) {
    const int event_count = 1000;
    
    // 注册监听器
    uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
        laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
        [this](const laminpie::system::event::DeviceEvent& event) {
            OnDeviceEvent(event);
        }
    );
    
    // 测量事件分发性能
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < event_count; i++) {
        auto test_device = std::make_shared<DeviceIdentifier>(
            DeviceIdentifier::BusType::BUS_I2C, "perf_device_" + std::to_string(i));
        
        laminpie::system::event::DeviceEvent test_event(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd, 
            test_device);
        
        dispatcher.dispatchEvent(test_event);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "Event dispatch performance: " << duration.count() << "μs for " 
              << event_count << " events" << std::endl;
    
    // 验证性能要求（1000个事件应该在100ms内分发完成）
    EXPECT_LT(duration.count(), 100000) << "Event dispatch performance test exceeded time limit";
    
    // 等待所有事件处理完成
    bool all_events_received = WaitForCondition([this]() {
        return GetReceivedEventCount() >= event_count;
    }, 5000);
    
    EXPECT_TRUE(all_events_received) << "All events should be processed within timeout";
    
    // 清理监听器
    dispatcher.removeEventListener(listener_id);
}
