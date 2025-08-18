#include "laminpie_core_framework.hpp"

namespace laminpie::system::framework {

Laminpie_Core_Framework::Laminpie_Core_Framework(Laminpie_Core_Data_t &data, Laminpie_CoreHome &core_display, 
                            app::Laminpie_App_Manager &core_manager, event::LaminPie_EventDispatcher &core_event,
                            device::DeviceManager &core_device_manager, lv_display_t *device):
                            _core_data(data),
                            _core_display(core_display),
                            _core_app_manager(core_manager),
                            _core_event(core_event),
                            _core_device_manager(core_device_manager),
                            _display_device(device),
                            _touch_device(nullptr),
                            _device_event_type(event::Laminpie_Device_Event_Type::kDevice_Event_Type_Max),
                            _boot_event_type(event::Laminpie_Boot_Event_Type::kBoot_Stage_HardWareInit),
                            _app_event_type(event::Laminpie_App_Event_Type::kApp_Event_Type_Max),
                            _app_navigation_event_type(event::Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_IDLE),
                            _ui_event_type(event::Laminpie_UI_Event_Type::kUI_Event_Type_Max),
                            _lv_lock_callback(nullptr),
                            _lv_unlock_callback(nullptr){}

Laminpie_Core_Framework::~Laminpie_Core_Framework(void)
{

}


}