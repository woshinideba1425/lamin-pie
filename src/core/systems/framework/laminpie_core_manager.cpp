#include "laminpie_core_manager.h"
#include "laminpie_system_event_type.hpp"
#include "laminpie_system_internal.h"

namespace laminpie::system::framework {

Laminpie_Core_Manager::Laminpie_Core_Manager(Laminpie_Core_Framework& core_framework)
    : _core_framework(core_framework)
    , _event_dispatcher(core_framework.GetEventDispatcher())
{
}

Laminpie_Core_Manager::~Laminpie_Core_Manager()
{
}

bool Laminpie_Core_Manager::Begin()
{
    if(_initialized) {
        SYSTEM_EVENT_LOG_WARN("Core Manager already initialized");
        return false;
    }

    _initialized = true;

    RegisterEventListeners();
    SYSTEM_MANAGER_LOG_INFO("Core Manager initialized successfully");
    return true;
}

void Laminpie_Core_Manager::Cleanup()
{
}

void Laminpie_Core_Manager::RegisterEventListeners() {
    RegisterDeviceEventListeners();
    RegisterBootEventListeners();
    RegisterAppEventListeners();
    RegisterNavigationEventListeners();
    RegisterUIEventListeners();
}

void Laminpie_Core_Manager::RegisterNavigationEventListeners() {
    auto nav_to_app_id = _core_framework.RegisterEventListenerForAll<Laminpie_Navigation_EventData_t>([this](const Laminpie_Navigation_EventData_t& event) {
        HandleNavigationEvent(event);
    });
    _navigation_listener_ids.push_back(nav_to_app_id);
}

void Laminpie_Core_Manager::RegisterDeviceEventListeners() {
    auto device_event_id = _core_framework.RegisterEventListenerForAll<DeviceEvent>([this](const DeviceEvent& event) {
        HandleDeviceEvent(event);
    });
    _device_listener_ids.push_back(device_event_id);
}

void Laminpie_Core_Manager::RegisterBootEventListeners() {
    auto boot_event_id = _core_framework.RegisterEventListenerForAll<Boot_EventData_t>([this](const Boot_EventData_t& event) {
        HandleBootEvent(event);
    });
    _boot_listener_ids.push_back(boot_event_id);
}

void Laminpie_Core_Manager::RegisterAppEventListeners() {
    auto app_event_id = _core_framework.RegisterEventListenerForAll<App_EventData_t>([this](const App_EventData_t& event) {
        HandleAppEvent(event);
    });
    _app_listener_ids.push_back(app_event_id);
}

void Laminpie_Core_Manager::RegisterUIEventListeners() {
    auto ui_event_id = _core_framework.RegisterEventListenerForAll<Ui_Update_Event_t>([this](const Ui_Update_Event_t& event) {
        HandleUIEvent(event);
    });
    _ui_listener_ids.push_back(ui_event_id);
}

}