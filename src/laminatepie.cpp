#include "laminatepie.h"
#include "builtin_apps.h"

namespace LAMINATEPIE{
    void Laminate::app_in_use()
    {
        FreeRTOSAppBase* app_ptr = nullptr;
        app_ptr = new BUILTIN_APP::HeartRateApp;
        framework->install(app_ptr,NULL);

        app_ptr = new BUILTIN_APP::BloodOxyzenApp;
        framework->install(app_ptr,NULL);

        app_ptr = new BUILTIN_APP::CalendarApp;
        framework->install(app_ptr,NULL);

        app_ptr = new BUILTIN_APP::WeatherApp;
        framework->install(app_ptr,NULL);

        app_ptr = new BUILTIN_APP::BloodPressureApp;
        framework->install(app_ptr,NULL);

        app_ptr = new BUILTIN_APP::AlarmApp;
        framework->install(app_ptr,NULL);

        app_ptr = new BUILTIN_APP::Settings;
        framework->install(app_ptr,NULL);
        // 将Settings也注册为系统应用
        // framework->registerSystemApp(app_ptr);

        app_ptr = new BUILTIN_APP::Ahrs;
        framework->install(app_ptr,NULL);

        app_ptr = new BUILTIN_APP::InfraredApp;
        framework->install(app_ptr,NULL);

        // 为每个应用安装左滑退出回调事件
        auto appList = framework->getAppList();
        for (const auto& appEntry : appList) {
            lv_obj_t* app_screen = appEntry.app->getScreen();
            if (app_screen) {
                lv_obj_set_user_data(app_screen, appEntry.app); 

                lv_obj_add_event_cb(app_screen, Laminate::_lvgl_app_event, LV_EVENT_ALL, framework);
                ESP_LOGI("APP_Register", "Installed event callback for app: %s", appEntry.app->getAppName().c_str());
            } else {
                ESP_LOGW("APP_Register", "No screen found for app: %s", appEntry.app->getAppName().c_str());
            }
        }
        ESP_LOGI("Laminate", "All apps installed and configured.");
    }

    void Laminate::star_app_framework(){
        ESP_LOGI("Laminate", "Starting framework...");
        framework->activity();
    }

    void Laminate::_lvgl_app_event(lv_event_t* e){
        Framework* framework = (Framework*)lv_event_get_user_data(e);
        FreeRTOSAppBase* app = (FreeRTOSAppBase*)lv_obj_get_user_data(lv_event_get_target(e));
        lv_event_code_t event_code = lv_event_get_code(e);
        
        if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
            lv_indev_wait_release(lv_indev_get_act());
            
            if (app == nullptr) {
                ESP_LOGE("APP_Event", "App is null, cannot close task.");
                return;
            }

            // 如果是Launcher应用，不做任何操作
            if (app->getAppName() == "Launcher") {
                return;
            }
            
            _ui_screen_change(&ui_main_tabview, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 0, &ui_main_tapview_screen_init);

            // 使用APP_Register::isSystemApp检查是否为系统应用
            if (app->isAllowBgRunning() && framework->isSystemApp(app)) {
                framework->moveAppToBackground(app);
            } else {
                // 否则关闭任务
                app->closeTask();
            }
            
            // 总是启动Launcher
            framework->startApp(framework->getApp("Launcher"));
        }
    }
}
