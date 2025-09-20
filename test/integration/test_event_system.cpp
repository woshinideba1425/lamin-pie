#include "test_common.h"
#include "test_platform.h"
#include "laminpie_event_dispatcher.hpp"
#include "interface/device_types.h"


class EventSystemTest {
private:
    laminpie::system::event::LaminPie_EventDispatcher& dispatcher;
    std::vector<laminpie::system::event::DeviceEvent> received_events;
    
public:
    EventSystemTest() : dispatcher(laminpie::system::event::LaminPie_EventDispatcher::getInstance()) {
        dispatcher.start();
    }
    ~EventSystemTest() {
        dispatcher.stop();
    }
    
    TestResult test_event_registration() {
        // 测试事件监听器注册
        uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [this](const laminpie::system::event::DeviceEvent& event) {
                received_events.push_back(event);
            }
        );
        
        TEST_ASSERT(dispatcher.getListenerCount() > 0);
        
        // 清理注册的监听器，避免影响后续测试
        dispatcher.removeEventListener(listener_id);
        
        return TestResult::kPass;
    }
    
    TestResult test_event_dispatch() {
        // 测试事件分发
        received_events.clear();
        
        printf("DEBUG: Starting test_event_dispatch\n");
        printf("DEBUG: Listener count before registration: %zu\n", dispatcher.getListenerCount());
        
        // 注册监听器
        uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [this](const laminpie::system::event::DeviceEvent& event) {
                printf("DEBUG: Event callback triggered! Device ID: %s\n", event.device->id.c_str());
                received_events.push_back(event);
            }
        );
        
        printf("DEBUG: Listener registered with ID: %lu\n", listener_id);
        printf("DEBUG: Listener count after registration: %zu\n", dispatcher.getListenerCount());
        
        // 发送事件
        auto device_id = std::make_shared<DeviceIdentifier>(
            DeviceIdentifier::BusType::BUS_I2C, "test_device"
        );
        auto event = laminpie::system::event::DeviceEvent(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            device_id
        );
        
        printf("DEBUG: About to dispatch event\n");
        dispatcher.dispatchEvent(event);
        printf("DEBUG: Event dispatched, received_events.size() = %zu\n", received_events.size());
        
        // 清理注册的监听器
        dispatcher.removeEventListener(listener_id);
        printf("DEBUG: Listener removed, count after removal: %zu\n", dispatcher.getListenerCount());
        
        TEST_ASSERT(received_events.size() == 1);
        TEST_ASSERT(received_events[0].device->id == "test_device");
        
        return TestResult::kPass;
    }
    
    TestResult test_async_event_processing() {
        // 测试异步事件处理
        received_events.clear();
        
        printf("DEBUG: Starting test_async_event_processing\n");
        printf("DEBUG: Listener count before registration: %zu\n", dispatcher.getListenerCount());
        
        // 注册监听器
        uint32_t listener_id = dispatcher.addEventListener<laminpie::system::event::DeviceEvent>(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            [this](const laminpie::system::event::DeviceEvent& event) {
                printf("DEBUG: Async event callback triggered! Device ID: %s\n", event.device->id.c_str());
                received_events.push_back(event);
            }
        );
        
        printf("DEBUG: Async listener registered with ID: %lu\n", listener_id);
        printf("DEBUG: Listener count after registration: %zu\n", dispatcher.getListenerCount());
        
        // 异步发送事件
        auto device_id = std::make_shared<DeviceIdentifier>(
            DeviceIdentifier::BusType::BUS_I2C, "async_device"
        );
        auto event = std::make_shared<laminpie::system::event::DeviceEvent>(
            laminpie::system::event::Laminpie_Device_Event_Type::kDeviceAdd,
            device_id
        );
        
        printf("DEBUG: About to post async event\n");
        dispatcher.postEvent(event);
        
        // 等待事件处理
        PLATFORM_DELAY_MS(100);
        
        printf("DEBUG: Async event processed, received_events.size() = %zu\n", received_events.size());
        
        // 清理注册的监听器
        dispatcher.removeEventListener(listener_id);
        printf("DEBUG: Async listener removed, count after removal: %zu\n", dispatcher.getListenerCount());
        
        TEST_ASSERT(received_events.size() == 1);
        TEST_ASSERT(received_events[0].device->id == "async_device");
        
        return TestResult::kPass;
    }
};