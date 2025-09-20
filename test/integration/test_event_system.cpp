#include "test_common.h"
#include "test_platform.h"
#include "laminpie_event_dispatcher.hpp"
#include "interface/device_types.h"


class EventSystemTest {
private:
    laminpie::system::event::LaminPie_EventDispatcher& dispatcher;
    std::vector<laminpie::system::event::DeviceEvent> received_events;
    
public:
    EventSystemTest() : dispatcher(laminpie::system::event::LaminPie_EventDispatcher::getInstance()) {}
    
    TestResult test_event_registration() {
        // 测试事件监听器注册
        dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [this](const laminpie::system::event::DeviceEvent& event) {
                received_events.push_back(event);
                return true;
            }
        );
        
        TEST_ASSERT(dispatcher.getListenerCount() > 0);
        return TestResult::kPass;
    }
    
    TestResult test_event_dispatch() {
        // 测试事件分发
        received_events.clear();
        
        // 注册监听器
        dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [this](const laminpie::system::event::DeviceEvent& event) {
                received_events.push_back(event);
                return true;
            }
        );
        
        // 发送事件
        auto device_id = std::make_shared<DeviceIdentifier>(
            DeviceIdentifier::BusType::BUS_I2C, "test_device"
        );
        auto event = laminpie::system::event::DeviceEvent(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            device_id
        );
        
        dispatcher.dispatchEvent(event);
        
        TEST_ASSERT(received_events.size() == 1);
        TEST_ASSERT(received_events[0].device->id == "test_device");
        
        return TestResult::kPass;
    }
    
    TestResult test_async_event_processing() {
        // 测试异步事件处理
        received_events.clear();
        
        // 启动事件分发器
        dispatcher.start();
        
        // 注册监听器
        dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [this](const laminpie::system::event::DeviceEvent& event) {
                received_events.push_back(event);
                return true;
            }
        );
        
        // 异步发送事件
        auto device_id = std::make_shared<DeviceIdentifier>(
            DeviceIdentifier::BusType::BUS_I2C, "async_device"
        );
        auto event = std::make_shared<laminpie::system::event::DeviceEvent>(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            device_id
        );
        
        dispatcher.postEvent(event);
        
        // 等待事件处理
        PLATFORM_DELAY_MS(100);
        
        TEST_ASSERT(received_events.size() == 1);
        TEST_ASSERT(received_events[0].device->id == "async_device");
        
        dispatcher.stop();
        return TestResult::kPass;
    }
};