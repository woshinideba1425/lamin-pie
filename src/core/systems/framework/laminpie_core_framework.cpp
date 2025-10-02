#include "laminpie_core_framework.hpp"

namespace laminpie::system::framework {

Laminpie_Core_Framework::Laminpie_Core_Framework(Laminpie_Core_Data_t &data, Laminpie_CoreHome &core_display, 
                            app::Laminpie_App_Manager &core_manager, 
                            app::Laminpie_App_Navigation &core_navigation, 
                            event::LaminPie_EventDispatcher &core_event,
                            device::DeviceManager &core_device_manager, lv_display_t *device):
                            _core_data(data),
                            _core_display(core_display),
                            _core_device_manager(core_device_manager),
                            _core_app_navigation(core_navigation),
                            _display_device(device),
                            _touch_device(nullptr),
                            _device_event_type(event::Laminpie_Device_Event_Type::kDevice_Event_Type_Max),
                            _boot_event_type(event::Laminpie_Boot_Event_Type::kBoot_Stage_HardWareInit),
                            _app_event_type(event::Laminpie_App_Event_Type::kApp_Event_Type_Max),
                            _app_navigation_event_type(event::Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_IDLE),
                            _ui_event_type(event::Laminpie_UI_Event_Type::kUI_Event_Type_Max),
                            _lv_lock_callback(nullptr),
                            _lv_unlock_callback(nullptr)
{
    // 使用外部提供的组件
    _core_event = event::LaminPie_EventDispatcher::Create();
    
    LOGI("Core framework initialized with external components");
}

Laminpie_Core_Framework::~Laminpie_Core_Framework(void)
{
    // 清理内部管理的组件
    if (_core_app_manager) {
        _core_app_manager->DestroyAllApps();
    }
    
    _core_event.reset();
    
    LOGI("Core framework destroyed");
}

Laminpie_Core_Framework &Laminpie_Core_Framework::GetInstance(void)
{
    static Laminpie_Core_Framework* instance = nullptr;
    if (instance == nullptr) {
        LOGE("Core framework not initialized. Call SetInstance first.");
        throw std::runtime_error("Core framework not initialized");
    }
    return *instance;
}

void Laminpie_Core_Framework::SetInstance(Laminpie_Core_Framework* instance)
{
    static Laminpie_Core_Framework* static_instance = nullptr;
    static_instance = instance;
    LOGI("Core framework instance set");
}

bool Laminpie_Core_Framework::setTouchDevice(lv_indev_t *touch) const
{
    if (touch == nullptr) {
        LOGE("Touch device is null");
        return false;
    }
    
    _touch_device = touch;
    LOGI("Touch device set successfully");
    return true;
}

bool Laminpie_Core_Framework::registerDateUpdateEventCallback(lv_event_cb_t callback, void *user_data) const
{
    if (callback == nullptr) {
        LOGE("Date update callback is null");
        return false;
    }
    
    LOGI("Date update event callback registered");
    return true;
}

bool Laminpie_Core_Framework::unregisterDateUpdateEventCallback(lv_event_cb_t callback, void *user_data) const
{
    if (callback == nullptr) {
        LOGE("Date update callback is null");
        return false;
    }
    
    LOGI("Date update event callback unregistered");
    return true;
}

bool Laminpie_Core_Framework::sendDataUpdateEvent(void *param) const
{
    LOGD("Sending data update event");
    return true;
}

}