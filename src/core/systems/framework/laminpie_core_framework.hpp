#pragma once
#include "laminpie_system_internal.h"
#include "style/laminpie_gui_style.hpp"
#include "laminpie_core_display.hpp"

namespace laminpie::system::framework {

typedef struct {
    const char *name;
    gui::StyleSize screen_size;
    union {
        Laminpie_CoreHomeData_t home;
        Laminpie_CoreHomeData_t display;
    };
    app::Laminpie_App_Manager manager;
} Laminpie_Core_Data_t;


class Laminpie_Core_Framework {
public:
    Laminpie_Core_Framework(void);
    ~Laminpie_Core_Framework(void);

    static Laminpie_Core_Framework &GetInstance(void);
    const Laminpie_Core_Data_t &GetCoreData(void) const { return _core_data; }
    lv_display_t *GetDisplayDevice(void) const {return _display_device;}

protected:
    lv_display_t       *_display_device;
    Laminpie_Core_Data_t _core_data;

private:
    Laminpie_Core_Framework(const Laminpie_Core_Framework &) = delete;
    Laminpie_Core_Framework &operator=(const Laminpie_Core_Framework &) = delete;
};
}