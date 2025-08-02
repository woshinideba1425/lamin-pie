#pragma once

#include <list>
#include <map>
#include <stdint.h>
#include <string>
#include "lvgl.h"
#include "lvgl/laminpie_lv_helper.hpp"
#include "../laminpie_system_internal.h"
#include "src/core/systems/framework/laminpie_system_event_type.hpp"
#include "laminpie_app_navigation.hpp"

namespace laminpie::system::app {
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

constexpr int Laminpie_App_ID_Min = 1;

class Laminpie_App_Manager;
class Laminpie_Framework;

class Laminpie_App_Base {
public:
    friend class Laminpie_App_Manager;

    Laminpie_App_Base(const Laminpie_App_Base_Data_t &data);

    Laminpie_App_Base(const char *name, const void *launcher_icon, bool use_default_screen);

    virtual ~Laminpie_App_Base() = default;

    bool checkInitialized(void) const;

    int getId(void) const
    {
        return _id;
    }

    const char *getName(void) const
    {
        return _core_active_data.name;
    }

    const laminpie::gui::StyleImage &getLauncherIcon(void) const
    {
        return _core_active_data.launcher_icon;
    }

    const lv_area_t &getVisualArea(void) const
    {
        return _app_style.calibrate_visual_area;
    }

    const Laminpie_App_Base_Data_t &getCoreInitData(void) const
    {
        return _core_init_data;
    }

    const Laminpie_App_Base_Data_t &getCoreActiveData(void) const
    {
        return _core_active_data;
    }

    Laminpie_Framework &getFramework(void) const
    {
        return _framework;
    }

protected:
    event::LaminPie_EventDispatcher<event::Laminpie_App_Status_t> _event_dispatcher;
    bool notifyCoreClosed(void) const;
    void setLauncherIconImage(const laminpie::gui::StyleImage &icon_image);
    bool startRecordResource(void);
    bool endRecordResource(void);
    bool cleanRecordResource(void);

protected:
    /// @brief 用于注册app环节的代码执行
    virtual void onSetup() {}
    virtual void onCreate() = 0;  
    virtual void onLoop() = 0;
    virtual void onResume() = 0;
    virtual void onPause() = 0;
    virtual void onDestroy() = 0;
    virtual void onRunningBG() = 0;  

    Laminpie_Framework &_framework;

private:
    //Core
    Laminpie_App_Base_Data_t _core_init_data;
    Laminpie_App_Base_Data_t _core_active_data;
    event::Laminpie_App_Status_t _status;
    //Navigation
    AppNode _app_node;
    PageNode _home_page_node;
    // Attributes
    int _id;
    struct {
        uint8_t is_closing: 1;
        uint8_t is_runningbg: 1;
        uint8_t is_screen_small: 1;
        uint8_t is_resource_recording: 1;
    } _flags;
    struct {
        int w;
        int h;
        lv_theme_t *theme;
    } _display_style;
    struct {
        lv_area_t origin_visual_area;
        lv_area_t calibrate_visual_area;
        lv_theme_t *theme;
    } _app_style;
    // Resources
    int _resource_timer_count;
    int _resource_anim_count;
    int _resource_head_screen_index;
    int _resource_screen_count;
    lv_obj_t *_last_screen;
    lv_obj_t *_active_screen;
    // lv_obj_t *_temp_screen;
    lv_timer_t *_resource_head_timer;
    lv_anim_t *_resource_head_anim;
    std::list <lv_obj_t *> _resource_screens;
    std::list <lv_timer_t *> _resource_timers;
    std::list <lv_anim_t *> _resource_anims;
    // These maps are meant to store additional information about the recorded resources to prevent accidental cleanup
    std::map<lv_obj_t *, std::pair<const lv_obj_class_t *, lv_obj_t *>> _resource_screens_class_parent_map;
    std::map<lv_timer_t *, std::pair<lv_timer_cb_t, void *>> _resource_timers_cb_usr_map;
    std::map<lv_anim_t *, std::pair<void *, lv_anim_exec_xcb_t>> _resource_anims_var_exec_map;
};
}