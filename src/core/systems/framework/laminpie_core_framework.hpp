#pragma once
#include "laminpie_app_base.hpp"
#include "laminpie_event_dispatcher.hpp"
#include "laminpie_system_event_type.hpp"
#include "laminpie_system_internal.h"
#include "style/laminpie_gui_style.hpp"
#include "laminpie_core_display.hpp"
#include "laminpie_device_manager.h"

namespace laminpie::system::framework {

typedef struct {
    const char *name;
    gui::StyleSize screen_size;
    union {
        Laminpie_CoreHomeData_t home;
        app::Laminpie_Core_HomeNode *home_node;
        Laminpie_CoreHomeData_t display;
    };
    app::Laminpie_App_ManagerData_t manager;
} Laminpie_Core_Data_t;


class Laminpie_Core_Framework {
public:
    friend class app::Laminpie_App_Register;
    friend class app::Laminpie_App_Manager;

    Laminpie_Core_Framework(Laminpie_Core_Data_t &data, Laminpie_CoreHome &core_display, 
                            app::Laminpie_App_Manager &core_manager, event::LaminPie_EventDispatcher &core_event,
                            device::DeviceManager &core_device_manager, lv_display_t *device);
    ~Laminpie_Core_Framework(void);

    static Laminpie_Core_Framework &GetInstance(void);
    const Laminpie_Core_Data_t &GetCoreData(void) const { return _core_data; }
    lv_display_t *GetDisplayDevice(void) const {return _display_device;}
    event::LaminPie_EventDispatcher &GetEventDispatcher(void) {return _core_event;}
    device::DeviceManager &GetDeviceManager(void) {return _core_device_manager;}

protected:
    Laminpie_Core_Data_t _core_data;
    Laminpie_CoreHome                &_core_display;
    app::Laminpie_App_Manager        &_core_manager;
    event::LaminPie_EventDispatcher  &_core_event;
    device::DeviceManager            &_core_device_manager;

    lv_display_t       *_display_device;
    mutable lv_indev_t *_touch_device;
private:
    Laminpie_Core_Framework(const Laminpie_Core_Framework &) = delete;
    Laminpie_Core_Framework &operator=(const Laminpie_Core_Framework &) = delete;

    //event
    event::Laminpie_Device_Event_Type _device_event_type;
    event::Laminpie_Boot_Event_Type _boot_event_type;
    event::Laminpie_App_Event_Type _app_event_type;
    event::Laminpie_App_Navigation_Event_Type _app_navigation_event_type;
    event::Laminpie_UI_Event_Type _ui_event_type;
    
    // LVGL
    int _lv_lock_timeout;
    gui::LockCallback _lv_lock_callback;
    gui::UnlockCallback _lv_unlock_callback;
};
}