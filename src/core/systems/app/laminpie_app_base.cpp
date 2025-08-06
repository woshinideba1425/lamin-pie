#include "laminpie_app_base.hpp"
#include "laminpie_log.hpp"
#include "src/core/systems/laminpie_system_internal.h"

#define RESOURCE_LOOP_COUNT_MAX     (1000)

namespace laminpie::system::app {

// 添加事件回调函数实现
static void onResizeScreenLoadedEventCallback(lv_event_t *e)
{
    // 这里可以根据需要实现屏幕加载事件的处理
    // 目前只是一个占位符实现
    (void)e;
}

bool Laminpie_App_Base::StartRecordResource(void)
{
    lv_display_t *disp = nullptr;
    lv_area_t &visual_area = _app_style.calibrate_visual_area;

    utils::CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) start record resource", GetName(), _id);

    // disp = _framework.getDisplayDevice();
    utils::CheckNullAndReturn(disp, false, "Invalid display");

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

    utils::CheckFalseReturn(CheckInitialized(), false, "Not initialized");
    SYSTEM_APP_LOG_DEBUG("App(%s: %d) end record resource", GetName(), _id);

    if (!_flags.is_resource_recording) {
        SYSTEM_APP_LOG_DEBUG("Recording resource is not started, please start first");
        return true;
    }

    // disp = _framework->getDisplayDevice();
    utils::CheckNullAndReturn(disp, false, "Invalid display");

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
    utils::CheckFalseReturn(CheckInitialized(), false, "Not initialized");
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
    utils::CheckNullAndReturn(disp, false, "Invalid display");

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

    utils::CheckFalseReturn(ResetRecordResource(), false, "Reset record resource failed");

    return ret;
}
}
