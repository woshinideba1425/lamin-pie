#pragma once

#include <list>
#include <map>
#include <string>
#include "lvgl.h"
#include "lvgl/laminpie_lv_helper.hpp"

typedef struct {
    const char *name;
    laminpie::gui::StyleImage launcher_icon;
    laminpie::gui::StyleSize screen_size;
    struct {
        uint8_t enable_default_screen: 1;
        uint8_t enable_recycle_resource: 1;
        uint8_t enable_resize_visual_area: 1;
    } flags;
    int app_proity;
    void *user_data;
} Laminpie_App_Base_Data_t;

typedef enum {
    Laminpie_App_Status_Uninstalled = 0,
    Laminpie_App_Status_Running,
    Laminpie_App_Status_Paused,
    Laminpie_App_Status_Closed,
    Laminpie_App_Status_RunningBg
} Laminpie_App_Status_t;

constexpr int Laminpie_App_ID_Min = 1;

class Laminpie_App_Base {
    public:
        friend class Laminpie_App_Manager;

        Laminpie_App_Base(const Laminpie_App_Base_Data_t &data);

        Laminpie_App_Base(const char *name, const void *launcher_icon, bool use_default_screen);

        virtual ~Laminpie_App_Base() = default;

        bool checkInitialized(void) const;



    /// @brief 用于注册app环节的代码执行
    virtual void onSetup() {}
    virtual void onCreate() = 0;  
    virtual void onLoop() = 0;
    virtual void onResume() = 0;
    virtual void onPause() = 0;
    virtual void onDestroy() = 0;
    virtual void onRunningBG() = 0;  
};