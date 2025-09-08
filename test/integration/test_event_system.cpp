#include "test_common.h"
#include "laminpie_event_dispatcher.hpp"
#include "fixtures/mock_event_dispatcher.hpp"

class EventSystemTest {
private:
    system::event::LaminPie_EventDispatcher& dispatcher;
    std::vector<system::event::DeviceEvent> received_events;
    
public:
    EventSystemTest() : dispatcher(system::event::LaminPie_EventDispatcher::getInstance()) {}
    
    TestResult test_event_registration() {
        // 测试事件监听器注册
        auto listener_id = dispatcher.addEventListener<system::event::DeviceEvent>(
            system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [this](const system::event::DeviceEvent& event) {
                received_events.push_back(event);
                return true;
            }
        );
        
        TEST_ASSERT(listener_id > 0);
        return TestResult::kPass;
    }
    
    TestResult test_event_dispatch() {
        // 测试事件分发
        received_events.clear();
        
        // 注册监听器
        auto listener_id = dispatcher.addEventListener<system::event::DeviceEvent>(
            system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [this](const system::event::DeviceEvent& event) {
                received_events.push_back(event);
                return true;
            }
        );
        
        // 发送事件
        auto event = system::event::DeviceEvent(
            system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            "test_device"
        );
        
        dispatcher.dispatchEvent(event);
        
        TEST_ASSERT(received_events.size() == 1);
        TEST_ASSERT(received_events[0].name == "test_device");
        
        return TestResult::kPass;
    }
    
    TestResult test_async_event_processing() {
        // 测试异步事件处理
        received_events.clear();
        
        // 启动事件分发器
        dispatcher.start();
        
        // 注册监听器
        auto listener_id = dispatcher.addEventListener<system::event::DeviceEvent>(
            system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [this](const system::event::DeviceEvent& event) {
                received_events.push_back(event);
                return true;
            }
        );
        
        // 异步发送事件
        auto event = system::event::DeviceEvent(
            system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            "async_device"
        );
        
        dispatcher.postEvent(event);
        
        // 等待事件处理
        PLATFORM_DELAY_MS(100);
        
        TEST_ASSERT(received_events.size() == 1);
        TEST_ASSERT(received_events[0].name == "async_device");
        
        dispatcher.stop();
        return TestResult::kPass;
    }
};