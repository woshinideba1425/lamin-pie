#pragma once

#include <list>
#include <map>
#include <stdint.h>
#include <string>
#include "lvgl/laminpie_lv.hpp"
#include "../laminpie_system_internal.h"
#include "laminpie_event_dispatcher.hpp"

namespace laminpie::system::app {
using namespace laminpie::gui;
using namespace laminpie::system::event;

typedef struct {
    std::string name;
    laminpie::gui::StyleImage launcher_icon;
    laminpie::gui::StyleSize screen_size;
    struct {
        uint8_t enable_default_screen: 1;
        uint8_t enable_recycle_resource: 1;
        uint8_t enable_resize_visual_area: 1;
        uint8_t enable_running_bg: 1;
    } flags;
    int app_proity;
    void *user_data;
} Laminpie_App_Base_Data_t;

constexpr int Laminpie_App_ID_Min = 1;

class Laminpie_App_Register;
class Laminpie_App_Manager;
}

namespace laminpie::system::framework {
class Laminpie_Core_Framework;
}

namespace laminpie::system::app {

class Laminpie_App_Base {
public:
    friend class Laminpie_App_Register;
    friend class Laminpie_App_Manager;
    
    Laminpie_App_Base(const Laminpie_App_Base_Data_t &data);

    virtual ~Laminpie_App_Base() = default;

    bool CheckInitialized(void) const;

    int GetId(void) const
    {
        return _id;
    }

    const std::string &GetName(void) const
    {
        return _core_active_data.name;
    }

    const laminpie::gui::StyleImage &GetLauncherIcon(void) const
    {
        return _core_active_data.launcher_icon;
    }

    const lv_area_t &GetVisualArea(void) const
    {
        return _app_style.calibrate_visual_area;
    }

    const Laminpie_App_Base_Data_t &GetCoreInitData(void) const
    {
        return _core_init_data;
    }

    const Laminpie_App_Base_Data_t &GetCoreActiveData(void) const
    {
        return _core_active_data;
    }

    framework::Laminpie_Core_Framework *GetFramework(void) const
    {
        return _framework;
    }

    bool IsSystemApp(void) const
    {
        return _flags.is_system_app;
    }

    void SetSystemApp(bool is_system_app)
    {
        _flags.is_system_app = is_system_app;
    }
    

protected:
    LaminPie_EventDispatcher &_event_dispatcher;
    bool notifyCoreClosed(void) const;
    void SetLauncherIconImage(const laminpie::gui::StyleImage &icon_image);
    Laminpie_App_Event_Type GetStatus(void) const { return _status; }

    /// @brief 用于注册app环节的代码执行
    virtual bool OnSetup() { return true; }
    /// @brief 用于创建app环节的代码执行
    virtual bool OnCreate() = 0;  
    /// @brief 用于循环app环节的代码执行
    virtual bool OnLoop() = 0;
    /// @brief 用于恢复app环节的代码执行
    virtual bool OnResume() = 0;
    /// @brief 用于暂停app环节的代码执行
    virtual bool OnPause() = 0;
    /// @brief 用于关闭app环节的代码执行
    virtual bool OnClose() = 0;
    /// @brief 用于运行后台app环节的代码执行
    virtual bool OnRunningBG() = 0;  
    /**
     * @brief Start recording resources(screens, timers, and animations) manually.
     *
     * @note If the `enable_resize_visual_area` flag in `ESP_Brookesia_CoreAppData_t` is set, the core will resize the visual
     *       area of all recorded screens which are recorded in this function. This is useful when the screen displays
     *       floating UIs, such as a status bar. Otherwise, the app's screens will be displayed in full screen, but
     *       some areas might be not visible. The final visual area of the app is the intersection of the app's visual
     *       area and the `screen_size`. The app can call the `getVisualArea()` function to retrieve the final visual
     *       area
     * @note This function should be called before creating any resources, including screens (`lv_obj_create(NULL)`),
     *       animations (`lv_anim_start()`), and timers (`lv_timer_create()`)
     * @note This function should not be called in the `OnCreate()` and `OnPause()` functions.
     *
     * @return true if successful, otherwise false
     *
     */
    bool StartRecordResource(void);

    /**
     * @brief Stop recording resources(screens, timers, and animations) manually.
     *
     * @note This function should be called after creating any resources, including screens (`lv_obj_create(NULL)`),
     *       animations (`lv_anim_start()`), and timers (`lv_timer_create()`)
     * @note This function should not be called in the `OnCreate()` and `OnPause()` functions.
     *
     * @return true if successful, otherwise false
     *
     */
    bool EndRecordResource(void);

    /**
     * @brief Cleanup all recorded resources(screens, timers, and animations) manually. These resources are recorded in
     *        app's `OnCreate()` and `OnPause()` functions, or between the `startRecordResource()` and `stopRecordResource()`
     *        functions.
     *
     * @note If the `enable_recycle_resource` flag in `ESP_Brookesia_CoreAppData_t` is set, when app closes, the core will
     *       call this function automatically. So the app doesn't need to call this function manually.
     * @note This function will clear all resources records after finishing the cleanup.
     *
     * @return true if successful, otherwise false
     *
     */
    bool CleanRecordResource(void);

    framework::Laminpie_Core_Framework *_framework;

    /**
     * @brief Called when the app starts to close. The app can perform extra resource cleanup here.
     *
     * @note If there are resources that not recorded by the core (not created in the `OnCreate()` and `OnPause()` functions,
     *       or between the `startRecordResource()` and `stopRecordResource()` functions), the app should call this
     *       function to cleanup these gui resources manually. This function is not conflicted with the
     *       `CleanRecordResource()` function.
     *
     * @return true if successful, otherwise false
     *
     */
    virtual bool CleanResource(void)
    {
        return true;
    }

    bool IsRunningBG(void) const
    {
        return _flags.is_running_bg;
    }

    void SetRunningBG(bool is_running_bg)
    {
        _flags.is_running_bg = is_running_bg;
    }

private:
    virtual bool beginExtra(void) { return true; }
    virtual bool delExtra(void)   { return true; }
    virtual bool ProcessInstall(framework::Laminpie_Core_Framework *framework, int id);
    virtual bool ProcessUninstall(void);
    virtual bool ProcessCreate(void);
    virtual bool ProcessResume(void);
    virtual bool ProcessPause(void);
    virtual bool ProcessClose(bool is_app_active);    
    
    bool SetVisualArea(const lv_area_t &area);
    bool CalibrateVisualArea(void);
    bool InitDefaultScreen(void);
    bool CleanDefaultScreen(void);
    bool SaveRecentScreen(bool check_valid);
    bool LoadRecentScreen(void);
    bool ResetRecordResource(void);
    bool EnableAutoClean(void);
    bool SaveDisplayTheme(void);
    bool LoadDisplayTheme(void);
    bool SaveAppTheme(void);
    bool LoadAppTheme(void);

    static void onCleanResourceEventCallback(lv_event_t *e);
    static void onResizeScreenLoadedEventCallback(lv_event_t *e);

    //Core
    Laminpie_App_Base_Data_t _core_init_data;
    Laminpie_App_Base_Data_t _core_active_data;
    Laminpie_App_Event_Type _status;
    // Attributes
    int _id;
    struct {
        uint8_t is_closing: 1;
        uint8_t is_runningbg: 1;
        uint8_t is_screen_small: 1;
        uint8_t is_resource_recording: 1;
        uint8_t is_system_app: 1;
        uint8_t is_running_bg: 1;
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