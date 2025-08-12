#pragma once
#include "laminpie_event_dispatcher.hpp"
#include "laminpie_system_internal.h"
#include "style/laminpie_gui_style.hpp"
#include "laminpie_core_display.hpp"

namespace laminpie::system::framework {

typedef struct {
    const char *name;
    gui::StyleSize screen_size;
    union {
        Laminpie_CoreHomeData_t home;
        app::Laminpie_Core_HomeNode home_node;
        Laminpie_CoreHomeData_t display;
    };
    app::Laminpie_App_Manager manager;
    app::Laminpie_App_Navigation navigation;
} Laminpie_Core_Data_t;


class Laminpie_Core_Framework {
public:
    friend class app::Laminpie_App_Register;
    friend class app::Laminpie_App_Manager;

    Laminpie_Core_Framework(Laminpie_Core_Data_t &data);
    ~Laminpie_Core_Framework(void);

    static Laminpie_Core_Framework &GetInstance(void);
    const Laminpie_Core_Data_t &GetCoreData(void) const { return _core_data; }
    lv_display_t *GetDisplayDevice(void) const {return _display_device;}
    event::LaminPie_EventDispatcher &GetEventDispatcher(void) {return _core_event;}

protected:
    lv_display_t       *_display_device;
    Laminpie_Core_Data_t _core_data;
    Laminpie_CoreHome                &_core_display;
    app::Laminpie_App_Manager        &_core_manager;
    event::LaminPie_EventDispatcher  &_core_event;

private:
    Laminpie_Core_Framework(const Laminpie_Core_Framework &) = delete;
    Laminpie_Core_Framework &operator=(const Laminpie_Core_Framework &) = delete;
};
}