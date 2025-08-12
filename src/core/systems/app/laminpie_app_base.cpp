#include "laminpie_core_framework.hpp"
#include "laminpie_app_base.hpp"
#include "laminpie_event_dispatcher.hpp"
#include "laminpie_log.hpp"
#include "src/core/systems/laminpie_system_internal.h"
#include "style/laminpie_gui_style.hpp"
#define RESOURCE_LOOP_COUNT_MAX     (1000)

namespace laminpie::system::app {

Laminpie_App_Base::Laminpie_App_Base(const Laminpie_App_Base_Data_t &data):
    _event_dispatcher(LaminPie_EventDispatcher::getInstance()),
    _core_init_data(data),
    _status(Laminpie_App_Status_t::kApp_Status_Uninstalled),
    _id(-1),
    _flags{},
    _display_style{},
    _app_style{},
    _resource_timer_count(0),
    _resource_anim_count(0),
    _resource_head_screen_index(0),
    _resource_screen_count(0),
    _last_screen(nullptr),
    _active_screen(nullptr),
    _resource_head_timer(nullptr),
    _resource_head_anim(nullptr)
{
}
// bool Laminpie_App_Base::CheckInitialized(void) const{
//     return (_id >= Laminpie_App_ID_Min) && (_framework != nullptr) &&
//            (_framework->getAppManager().getInstalledApp(_id) == this);
// }

bool Laminpie_App_Base::notifyCoreClosed(void) const{
    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) notify core closed", GetName(), _id);

    if (_flags.is_closing) {
        return true;
    }

    _event_dispatcher.postEvent(std::make_shared<App_EventData_t>(_id, Laminpie_App_Status_t::kApp_Status_Closed, nullptr));    
    return true;
}

void Laminpie_App_Base::SetLauncherIconImage(const StyleImage &icon_image){
    _core_active_data.launcher_icon = icon_image;
}

bool Laminpie_App_Base::ProcessInstall(framework::Laminpie_Core_Framework *framework, int id){
    CheckFalseReturn(CheckInitialized(), false, "Already initialized");
    CheckNullAndReturn(framework, false, "Framework is invalid");
    CheckNullAndReturn(_core_init_data.name, false, "App name is invalid");

    SYSTEM_APP_LOG_DEBUG("App(%s: %d) install", _core_init_data.name, id);

    _core_active_data = _core_init_data;
    _framework = framework;
    _id = id;

    try{
        // framework->getCoreHome().calibrateCoreObjectSize(framework->getCoreData().screen_size, _core_active_data.screen_size);
    }catch(const std::exception &e){
        throw std::runtime_error("Calibrate screen size failed: " + std::string(e.what()));
    }

    if(beginExtra()){
        _status = Laminpie_App_Status_t::kApp_Status_Created;
    }else{
        ProcessUninstall();
        return false;
    }

    return true;
}

bool Laminpie_App_Base::ProcessUninstall(void){
    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) uninstall", GetName(), _id);

    // _framework = nullptr;
    _core_active_data = {};
    _status = Laminpie_App_Status_t::kApp_Status_Uninstalled;
    _id = -1;
    _flags = {};
    _display_style = {};
    _app_style = {};
    _resource_timer_count = 0;
    _resource_anim_count = 0;
    _resource_head_screen_index = 0;
    _resource_screen_count = 0;
    if(_core_active_data.flags.enable_default_screen && checkLvObjIsValid(_active_screen)){
        lv_obj_del(_active_screen);
    }
    _active_screen = nullptr;
    _resource_head_timer = nullptr;
    _resource_head_anim = nullptr;
    _resource_screens.clear();
    _resource_timers.clear();
    _resource_anims.clear();

    CheckFalseReturn(delExtra(), false, "Begin extra failed");
    _status = Laminpie_App_Status_t::kApp_Status_Uninstalled;

    return true;
}

bool Laminpie_App_Base::ProcessCreate(void){
    bool ret = true;

    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) create", GetName(), _id);

    CheckFalseReturn(SaveRecentScreen(false), false, "Save recent screen before run failed");

    if (!SaveRecentScreen(true)) {
        SYSTEM_APP_LOG_ERROR("Save recent screen after run failed");
        ret = false;
    }
    _status = Laminpie_App_Status_t::kApp_Status_Running;

    if(!ret){
        CheckFalseReturn(ProcessClose(true), false, "Close app failed");
    }
    return ret;
}

bool Laminpie_App_Base::ProcessResume(void)
{
    bool ret = true;

    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) resume", GetName(), _id);

    CheckFalseReturn(LoadRecentScreen(), false, "Load recent screen failed");
    CheckFalseReturn(LoadAppTheme(), false, "Load app theme failed");

    _status = Laminpie_App_Status_t::kApp_Status_Running;

    if(!ret){
        CheckFalseReturn(ProcessClose(true), false, "Close app failed");
    }
    return ret;
}

bool Laminpie_App_Base::ProcessPause(void)
{
    bool ret = true;
    
    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) pause", GetName(), _id);

    CheckFalseReturn(SaveAppTheme(), false, "Save app theme failed");
    CheckFalseReturn(SaveRecentScreen(false), false, "Save recent screen failed");
    CheckFalseReturn(LoadDisplayTheme(), false, "Load display theme failed");

    _status = Laminpie_App_Status_t::kApp_Status_Paused;

    if(!ret){
        CheckFalseReturn(ProcessClose(true), false, "Close app failed");
    }
    return ret;
}

bool Laminpie_App_Base::ProcessClose(bool is_app_active)
{
    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) close", GetName(), _id);

    if(_flags.is_closing){
        return true;
    }

    _event_dispatcher.postEvent(std::make_shared<App_EventData_t>(_id, Laminpie_App_Status_t::kApp_Status_Closed, nullptr));
    return true;
}

bool Laminpie_App_Base::SetVisualArea(const lv_area_t &area)
{
    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) set origin visual area[(%d,%d)-(%d,%d)]", GetName(),
                   _id, area.x1, area.y1, area.x2, area.y2);

    _app_style.origin_visual_area = area;

    return true;
}

bool Laminpie_App_Base::CalibrateVisualArea(void)
{
    int visual_area_x = 0;
    int visual_area_y = 0;
    int visual_area_w = 0;
    int visual_area_h = 0;
    lv_area_t visual_area = _app_style.origin_visual_area;
    const StyleSize &screen_size = _framework->GetCoreData().screen_size;
    const StyleSize &app_size = _framework->GetCoreData().screen_size;

    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) calibrate visual area[origin: (%d,%d)-(%d,%d)]", GetName(),
                   _id, visual_area.x1, visual_area.y1, visual_area.x2, visual_area.y2);

    visual_area_w = visual_area.x2 - visual_area.x1 + 1;
    visual_area_h = visual_area.y2 - visual_area.y1 + 1;
    visual_area_x = visual_area.x1;
    visual_area_y = visual_area.y1;
    if (visual_area_w > app_size.width) {
        visual_area_x = visual_area.x1 + (visual_area_w - app_size.width) / 2;
    }
    if (visual_area_h > app_size.height) {
        visual_area_y = visual_area.y1 + (visual_area_h - app_size.height) / 2;
    }
    visual_area_w = std::min(visual_area_w, app_size.width);
    visual_area_h = std::min(visual_area_h, app_size.height);
    visual_area.x1 = visual_area_x;
    visual_area.y1 = visual_area_y;
    visual_area.x2 = visual_area_x + visual_area_w - 1;
    visual_area.y2 = visual_area_y + visual_area_h - 1;

    _app_style.calibrate_visual_area = visual_area;
    _flags.is_screen_small = ((lv_area_get_height(&visual_area) < screen_size.height) ||
                              (lv_area_get_width(&visual_area) < screen_size.width));

    SYSTEM_APP_LOG_DEBUG("Calibrate visual area(%d,%d-%d,%d)", visual_area.x1, visual_area.y1, visual_area.x2, visual_area.y2);

    return true;
}

bool Laminpie_App_Base::StartRecordResource(void)
{
    lv_display_t *disp = nullptr;
    lv_area_t &visual_area = _app_style.calibrate_visual_area;

    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) start record resource", GetName(), _id);

    // disp = _framework.getDisplayDevice();
    CheckNullAndReturn(disp, false, "Invalid display");

    if (_flags.is_resource_recording) {
        SYSTEM_APP_LOG_DEBUG("Recording resource is already started, don't start again");
        return true;
    }

    if (_core_active_data.flags.enable_resize_visual_area) {
        SYSTEM_APP_LOG_DEBUG("Resieze screen to visual area[(%d,%d)-(%d,%d)]", visual_area.x1, visual_area.y1, visual_area.x2,
                       visual_area.y2);
        _display_style.w = disp->hor_res;
        _display_style.h = disp->ver_res;
        disp->hor_res = visual_area.x2 - visual_area.x1 + 1;
        disp->ver_res = visual_area.y2 - visual_area.y1 + 1;
    }
    _resource_head_screen_index = disp->screen_cnt - 1;
    _resource_head_timer = lv_timer_get_next(nullptr);
    // 修复：使用正确的动画链表访问方式
    _resource_head_anim = (lv_anim_t *)lv_ll_get_head(&(LV_GLOBAL_DEFAULT()->anim_state.anim_ll));
    _flags.is_resource_recording = true;

    return true;
}

bool Laminpie_App_Base::EndRecordResource(void)
{
    bool ret = true;
    uint32_t resource_loop_count = 0;
    lv_display_t *disp = nullptr;
    lv_obj_t *screen = nullptr;
    lv_timer_t *timer_node = nullptr;
    lv_anim_t *anim_node = nullptr;
    const lv_area_t &visual_area = _app_style.calibrate_visual_area;

    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) end record resource", GetName(), _id);

    if (!_flags.is_resource_recording) {
        SYSTEM_APP_LOG_DEBUG("Recording resource is not started, please start first");
        return true;
    }

    // disp = _framework->getDisplayDevice();
    CheckNullAndReturn(disp, false, "Invalid display");

    // Screen
    resource_loop_count = 0;
    for (int i = _resource_head_screen_index + 1; (i < (int)disp->screen_cnt) &&
            (resource_loop_count++ <  RESOURCE_LOOP_COUNT_MAX); i++) {
        screen = (lv_obj_t *)disp->screens[i];
        // Record or update the record information of the screen
        _resource_screens_class_parent_map[screen] = {screen->class_p, (lv_obj_t *)screen->parent};
        if (find(_resource_screens.begin(), _resource_screens.end(), screen) == _resource_screens.end()) {
            // Only record the newest timer
            _resource_screens.push_back(screen);
            _resource_screen_count++;
            // Move screens to visual area when loaded only if needed
            if (_core_active_data.flags.enable_resize_visual_area) {
                lv_obj_set_pos(screen, visual_area.x1, visual_area.y1);
                lv_obj_add_event_cb(screen, onResizeScreenLoadedEventCallback, LV_EVENT_SCREEN_LOAD_START, this);
                // Avoid resetting the position of the previous screen when using animations with `lv_scr_load_anim()`
                lv_obj_add_event_cb(screen, onResizeScreenLoadedEventCallback, LV_EVENT_SCREEN_UNLOAD_START, this);
            }
        } else {
            SYSTEM_APP_LOG_DEBUG("Screen(@0x%p) is already recorded", screen);
        }
    }
    if ((_resource_head_screen_index >= (int)disp->screen_cnt) || (resource_loop_count >= RESOURCE_LOOP_COUNT_MAX)) {
        _resource_screens.clear();
        _resource_screens_class_parent_map.clear();
        _resource_screen_count = 0;
        ret = false;
        SYSTEM_APP_LOG_ERROR("record screen fail");
    } else {
        SYSTEM_APP_LOG_DEBUG("record screen(%d): ", _resource_screen_count);
    }

    // Timer
    resource_loop_count = 0;
    timer_node = lv_timer_get_next(nullptr);
    while ((timer_node != nullptr) && (timer_node != _resource_head_timer) &&
            (resource_loop_count++ < RESOURCE_LOOP_COUNT_MAX)) {
        // Record or update the record information of the timer
        _resource_timers_cb_usr_map[timer_node] = {(lv_timer_cb_t)timer_node->timer_cb, timer_node->user_data};
        if (find(_resource_timers.begin(), _resource_timers.end(), timer_node) == _resource_timers.end()) {
            // Only record the newest timer
            _resource_timers.push_back(timer_node);
            _resource_timer_count++;
        } else {
            SYSTEM_APP_LOG_DEBUG("Timer(@0x%p) is already recorded", timer_node);
        }
        timer_node = lv_timer_get_next(timer_node);
    }
    if (((timer_node == nullptr) && (_resource_head_timer != nullptr)) ||
            (resource_loop_count >= RESOURCE_LOOP_COUNT_MAX)) {
        _resource_timers.clear();
        _resource_timers_cb_usr_map.clear();
        _resource_timer_count = 0;
        ret = false;
        SYSTEM_APP_LOG_ERROR("record timer fail");
    } else {
        SYSTEM_APP_LOG_DEBUG("record timer(%d): ", _resource_timer_count);
    }

    // Animation
    // 修复：使用正确的动画链表访问方式
    anim_node = (lv_anim_t *)lv_ll_get_head(&(LV_GLOBAL_DEFAULT()->anim_state.anim_ll));
    while ((anim_node != nullptr) && (anim_node != _resource_head_anim)) {
        // Record or update the record information of the animation
        _resource_anims_var_exec_map[anim_node] = {anim_node->var, anim_node->exec_cb};
        if (find(_resource_anims.begin(), _resource_anims.end(), anim_node) == _resource_anims.end()) {
            // Only record the newest timer
            _resource_anims.push_back(anim_node);
            _resource_anim_count++;
        } else {
            SYSTEM_APP_LOG_DEBUG("Animation(@0x%p) is already recorded", anim_node);
        }
        // 修复：使用正确的动画链表访问方式
        anim_node = (lv_anim_t *)lv_ll_get_next(&(LV_GLOBAL_DEFAULT()->anim_state.anim_ll), anim_node);
    }
    if ((anim_node == nullptr) && (_resource_head_anim != nullptr)) {
        _resource_anims.clear();
        _resource_anims_var_exec_map.clear();
        _resource_anim_count = 0;
        SYSTEM_APP_LOG_ERROR("record animation fail");
    } else {
        SYSTEM_APP_LOG_DEBUG("record animation(%d): ", _resource_anim_count);
    }

    if (_core_active_data.flags.enable_resize_visual_area) {
        SYSTEM_APP_LOG_DEBUG("Resize screen back to display size(%d x %d)", _display_style.w, _display_style.h);
        disp->hor_res = _display_style.w;
        disp->ver_res = _display_style.h;
    }
    _flags.is_resource_recording = false;

    return ret;
}

bool Laminpie_App_Base::CleanRecordResource(void)
{
    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) clean resource", GetName(), _id);

    bool ret = true;
    bool do_clean = false;
    int resource_loop_count = 0;
    int resource_clean_count = 0;
    lv_display_t *disp = nullptr;
    lv_obj_t *screen_node = nullptr;
    lv_timer_t *timer_node = nullptr;
    lv_anim_t *anim_node = nullptr;

    // 修复：使用正确的显示设备获取方式
    disp = lv_display_get_default();
    CheckNullAndReturn(disp, false, "Invalid display");

    // Screen
    resource_loop_count = 0;
    resource_clean_count = 0;
    for (int i = 0; (i < (int)disp->screen_cnt) && (resource_loop_count++ <  RESOURCE_LOOP_COUNT_MAX);) {
        do_clean = false;
        screen_node = (lv_obj_t *)disp->screens[i];
        auto screen_it = find(_resource_screens.begin(), _resource_screens.end(), screen_node);
        if (screen_it != _resource_screens.end()) {
            auto screen_map_it = _resource_screens_class_parent_map.find(screen_node);
            if (screen_map_it == _resource_screens_class_parent_map.end()) {
                SYSTEM_APP_LOG_ERROR("Screen class parent map not found");
            } else {
                if ((screen_node->class_p == screen_map_it->second.first) &&
                        (screen_node->parent == screen_map_it->second.second)) {
                    lv_obj_del(screen_node);
                    do_clean = true;
                    resource_clean_count++;
                } else {
                    SYSTEM_APP_LOG_DEBUG("Screen(@0x%p) information is not matched, skip", screen_node);
                }
                _resource_screens.erase(screen_it);
                _resource_screens_class_parent_map.erase(screen_map_it);
            }
        }
        i = do_clean ? 0 : i + 1;
    }
    if (resource_loop_count >= RESOURCE_LOOP_COUNT_MAX) {
        ret = false;
        SYSTEM_APP_LOG_ERROR("Clean screen loop count exceed max");
    } else {
        SYSTEM_APP_LOG_DEBUG("Clean screen(%d), miss(%d): ", resource_clean_count, (int)(_resource_screen_count - resource_clean_count));
    }

    // Timer
    resource_loop_count = 0;
    resource_clean_count = 0;
    timer_node = lv_timer_get_next(nullptr);
    while ((timer_node != nullptr) && (_resource_timers.size() > 0) &&
            (resource_loop_count++ < RESOURCE_LOOP_COUNT_MAX)) {
        do_clean = false;
        auto timer_it = find(_resource_timers.begin(), _resource_timers.end(), timer_node);
        if (timer_it != _resource_timers.end()) {
            auto timer_map_it = _resource_timers_cb_usr_map.find(timer_node);
            if (timer_map_it == _resource_timers_cb_usr_map.end()) {
                SYSTEM_APP_LOG_ERROR("Timer cb usr map not found");
            } else  {
                if ((timer_map_it->second.first == timer_node->timer_cb) &&
                        (timer_map_it->second.second == timer_node->user_data)) {
                    lv_timer_del(timer_node);
                    do_clean = true;
                    resource_clean_count++;
                } else {
                    SYSTEM_APP_LOG_DEBUG("Timer(@0x%p) information is not matched, skip", timer_node);
                }
                _resource_timers.erase(timer_it);
                _resource_timers_cb_usr_map.erase(timer_map_it);
            }
        }
        timer_node = do_clean ? lv_timer_get_next(nullptr) : lv_timer_get_next(timer_node);
    }
    if (resource_loop_count >= RESOURCE_LOOP_COUNT_MAX) {
        ret = false;
        SYSTEM_APP_LOG_ERROR("Clean timer loop count exceed max");
    } else {
        SYSTEM_APP_LOG_DEBUG("Clean timer(%d), miss(%d): ", resource_clean_count, _resource_timer_count - resource_clean_count);
    }

    // Animation
    resource_loop_count = 0;
    resource_clean_count = 0;
    // 修复：使用正确的动画链表访问方式
    anim_node = (lv_anim_t *)lv_ll_get_head(&(LV_GLOBAL_DEFAULT()->anim_state.anim_ll));
    while ((anim_node != nullptr) && (_resource_anims.size() > 0) &&
            (resource_loop_count++ < RESOURCE_LOOP_COUNT_MAX)) {
        do_clean = false;
        auto anim_it = find(_resource_anims.begin(), _resource_anims.end(), anim_node);
        if (anim_it != _resource_anims.end()) {
            auto anim_map_it = _resource_anims_var_exec_map.find(anim_node);
            if (anim_map_it == _resource_anims_var_exec_map.end()) {
                SYSTEM_APP_LOG_ERROR("Animation var exec map not found");
            } else  {
                if ((anim_map_it->second.first == anim_node->var) &&
                        (anim_map_it->second.second == anim_node->exec_cb)) {
                    if (lv_anim_del(anim_node->var, anim_node->exec_cb)) {
                        do_clean = true;
                        resource_clean_count++;
                    } else {
                        SYSTEM_APP_LOG_ERROR("Delete animation failed");
                    }
                } else {
                    SYSTEM_APP_LOG_DEBUG("Anim(@0x%p) information is not matched, skip", anim_node);
                }
                _resource_anims.erase(anim_it);
                _resource_anims_var_exec_map.erase(anim_map_it);
            }
        }
        // 修复：使用正确的动画链表访问方式
        anim_node = do_clean ? (lv_anim_t *)lv_ll_get_head(&(LV_GLOBAL_DEFAULT()->anim_state.anim_ll)) :
                    (lv_anim_t *)lv_ll_get_next(&(LV_GLOBAL_DEFAULT()->anim_state.anim_ll), anim_node);
    }
    if (resource_loop_count >= RESOURCE_LOOP_COUNT_MAX) {
        ret = false;
        SYSTEM_APP_LOG_ERROR("Clean timer loop count exceed max");
    } else {
        SYSTEM_APP_LOG_DEBUG("Clean anim(%d), miss(%d): ", resource_clean_count, _resource_anim_count - resource_clean_count);
    }

    CheckFalseReturn(ResetRecordResource(), false, "Reset record resource failed");

    return ret;
}

bool Laminpie_App_Base::InitDefaultScreen(void)
{
    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) init default screen", GetName(), _id);

    _active_screen = lv_obj_create(nullptr);
    CheckNullAndReturn(_active_screen, false, "Create default screen failed");

    lv_scr_load(_active_screen);

    return true;
}

bool Laminpie_App_Base::CleanDefaultScreen(void)
{
    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) clean default active screen", GetName(), _id);

    if (checkLvObjIsValid(_active_screen)) {
        lv_obj_del(_active_screen);
    } else {
        SYSTEM_APP_LOG_WARN("Active screen is already cleaned");
    }
    _active_screen = nullptr;

    return true;
}


bool Laminpie_App_Base::SaveRecentScreen(bool check_valid)
{
    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) save recent screen", GetName(), _id);

    lv_obj_t *active_screen = lv_disp_get_scr_act(_framework->GetDisplayDevice());
    CheckNullAndReturn(active_screen, false, "Invalid active screen");

    if (check_valid) {
        CheckFalseReturn(active_screen != _last_screen, false, "No app screen");
    }
    _active_screen = active_screen;
    _last_screen = active_screen;

    return true;
}

bool Laminpie_App_Base::LoadRecentScreen(void)
{
    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) load recent screen", GetName(), _id);

    // TODO
    // if (_flags.is_screen_small) {
    //     // Create a temp screen to recolor the background
    //     ESP_UTILS_CHECK_FALSE_RETURN(createAndloadTempScreen(), false, "Create temp screen failed");
    // }

    CheckFalseReturn(checkLvObjIsValid(_active_screen), false, "Invalid active screen");
    lv_scr_load(_active_screen);

    // if (_flags.is_screen_small) {
    //     ESP_UTILS_CHECK_FALSE_RETURN(delTempScreen(), false, "Delete temp screen failed");
    // }

    return true;
}

bool Laminpie_App_Base::ResetRecordResource(void)
{
    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) reset record resource", GetName(), _id);

    // Screen
    _resource_screen_count = 0;
    _resource_screens.clear();
    _resource_screens_class_parent_map.clear();

    // Timer
    _resource_timer_count = 0;
    _resource_timers.clear();
    _resource_timers_cb_usr_map.clear();

    // Animation
    _resource_anim_count = 0;
    _resource_anims.clear();
    _resource_anims_var_exec_map.clear();

    _flags.is_resource_recording = false;

    return true;
}

bool Laminpie_App_Base::EnableAutoClean(void)
{
    lv_obj_t *last_screen = _framework->GetDisplayDevice()->scr_to_load;

    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) enable auto clean", GetName(), _id);

    // Check if the last screen is valid, if not, use the active screen
    if (last_screen == nullptr) {
        last_screen = _active_screen;
    }
    SYSTEM_APP_LOG_DEBUG("Clean resource when screen(0x%p) loaded", last_screen);

    CheckFalseReturn(checkLvObjIsValid(last_screen), false, "Invalid last screen");
    lv_obj_add_event_cb(last_screen, onCleanResourceEventCallback, LV_EVENT_SCREEN_UNLOADED, this);

    return true;
}

bool Laminpie_App_Base::SaveDisplayTheme(void)
{
    lv_display_t *display = nullptr;
    lv_theme_t *theme = nullptr;

    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) save display theme", GetName(), _id);

    display = _framework->GetDisplayDevice();
    CheckNullAndReturn(display, false, "Invalid display");

    theme = lv_disp_get_theme(display);
    CheckNullAndReturn(theme, false, "Invalid display theme");

    _display_style.theme = theme;

    return true;
}

bool Laminpie_App_Base::LoadDisplayTheme(void)
{
    lv_display_t *display = nullptr;
    lv_theme_t *&theme = _display_style.theme;

    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) load display theme", GetName(), _id);

    display = _framework->GetDisplayDevice();
    CheckNullAndReturn(display, false, "Invalid display");

    CheckNullAndReturn(theme, false, "Invalid display theme");
    lv_disp_set_theme(display, theme);

    return true;
}

bool Laminpie_App_Base::SaveAppTheme(void)
{
    lv_display_t *display = nullptr;
    lv_theme_t *theme = nullptr;

    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) save app theme", GetName(), _id);

    display = _framework->GetDisplayDevice();
    CheckNullAndReturn(display, false, "Invalid display");

    theme = lv_disp_get_theme(display);
    CheckNullAndReturn(theme, false, "Invalid app theme");

    _app_style.theme = theme;

    return true;
}

bool Laminpie_App_Base::LoadAppTheme(void)
{
    lv_display_t *display = nullptr;
    lv_theme_t *&theme = _display_style.theme;

    CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) load app theme", GetName(), _id);

    display = _framework->GetDisplayDevice();
    CheckNullAndReturn(display, false, "Invalid display");

    CheckNullAndReturn(theme, false, "Invalid app theme");
    lv_disp_set_theme(display, theme);

    return true;
}

void Laminpie_App_Base::onCleanResourceEventCallback(lv_event_t *event)
{
    Laminpie_App_Base *app = nullptr;

    SYSTEM_APP_LOG_DEBUG("App clean resource event callback");
    CheckNullAndReturn(event, false, "Invalid event");

    app = (Laminpie_App_Base *)lv_event_get_user_data(event);
    CheckNullAndReturn(app, false, "Invalid app");

    SYSTEM_APP_LOG_DEBUG("Clean app(%s: %d) resources", app->GetName(), app->_id);
    CheckFalseReturn(app->CheckInitialized(), false, "Not initialized");

    if (!app->CleanResource()) {
        SYSTEM_APP_LOG_ERROR("Clean resource failed");
    }
    if (app->_core_active_data.flags.enable_recycle_resource) {
        if (!app->CleanRecordResource()) {
            SYSTEM_APP_LOG_ERROR("Clean record resource failed");
        }
    } else if (app->_core_active_data.flags.enable_default_screen && !app->CleanDefaultScreen()) {
        SYSTEM_APP_LOG_ERROR("Clean default screen failed");
    }
}

void Laminpie_App_Base::onResizeScreenLoadedEventCallback(lv_event_t *event)
{
    Laminpie_App_Base *app = nullptr;
    lv_obj_t *screen = nullptr;
    lv_area_t area = { 0 ,0, 0, 0};

    SYSTEM_APP_LOG_DEBUG("App resize screen loaded event callback");
    CheckNullAndReturn(event, false, "Invalid event");

    app = (Laminpie_App_Base *)lv_event_get_user_data(event);
    screen = (lv_obj_t *)lv_event_get_target(event);
    CheckNullAndReturn(app, false, "Invalid app");
    CheckNullAndReturn(screen, false, "Invalid screen");

    CheckFalseReturn(app->CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("Resize app(%s: %d) screen", app->GetName(), app->_id);

    area = app->GetVisualArea();
    lv_obj_set_pos(screen, area.x1, area.y1);
}

}
