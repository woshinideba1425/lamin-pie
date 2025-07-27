#include "app_register.h"

namespace LAMINATEPIE{
    int APP_Register::install(FreeRTOSAppBase* app, SIMPLEKV::SimpleKV_ESP* database, void* userData) 
    {
        if (app == nullptr) {
            ESP_LOGE("APP_Register", "App is null, cannot install.");
            return -1;
        }

        app->setDatabase(database);
        app->setUserData(userData);

        app->onSetup();
        
        APPList_t newApp;
        newApp.app = app;
        
        newApp.id = ++_id;
        _app_list.push_back(newApp);
        
        //lv_obj_add_event_cb(app->getScreen(),_lvgl_app_event,LV_EVENT_ALL,NULL);
        ESP_LOGI("APP_Register", "App installed: %s, ID: %d", app->getAppName().c_str(), newApp.id);
        return newApp.id; 
    }

    // 获取应用的 ID
    int APP_Register::getAppID(FreeRTOSAppBase* app) {
        for (const auto& app_entry : _app_list) {
            if (app_entry.app == app) {
                return app_entry.id;
            }
        }
        return -1;
    }

    // 通过应用名称获取应用 ID
    int APP_Register::getAppID(const char* name) {
        for (const auto& app_entry : _app_list) {
            if (app_entry.app->getAppName() == name) {
                return app_entry.id;
            }
        }
        return -1;
    }

    // 根据 ID 获取应用实例
    FreeRTOSAppBase* APP_Register::getApp(int id) {
        for (const auto& app_entry : _app_list) {
            if (app_entry.id == id) {
                return app_entry.app;
            }
        }
        return nullptr;
    }

    // 根据名称获取应用实例
    FreeRTOSAppBase* APP_Register::getApp(const char* name) {
        for (const auto& app_entry : _app_list) {
            if (app_entry.app->getAppName() == name) {
                return app_entry.app;
            }
        }
        return nullptr;
    }

    // 卸载应用
    bool APP_Register::uninstall(FreeRTOSAppBase* app) {
        if (app == nullptr) {
            return false;
        }

        // 遍历应用列表，找到匹配的应用并移除
        for (auto iter = _app_list.begin(); iter != _app_list.end(); ++iter) {
            if (iter->app == app) {
                app->terminateTask();
                
                _app_list.erase(iter);
                return true;
            }
        }
        return false;
    }

    // 注册系统应用
    bool APP_Register::registerSystemApp(FreeRTOSAppBase* app)
    {
        if (app == nullptr) {
            ESP_LOGE("APP_Register", "App is null, cannot register as system app.");
            return false;
        }

        ESP_LOGI("APP_Register", "Registering system app: %s", app->getAppName().c_str());
        
        // 查找应用并标记为系统应用
        for (auto& appEntry : _app_list) {
            if (appEntry.app == app) {
                appEntry.isSystemApp = true;
                // 注意：setSystemApp是FreeRTOSAppBase的protected成员，外部不能调用
                // app->setSystemApp(true);
                ESP_LOGI("APP_Register", "App %s is registered as system app", app->getAppName().c_str());
                return true;
            }
        }
        
        ESP_LOGW("APP_Register", "App %s not found, cannot register as system app", app->getAppName().c_str());
        return false;
    }

    // 判断应用是否为系统应用
    bool APP_Register::isSystemApp(FreeRTOSAppBase* app) const {
        if (app == nullptr) {
            return false;
        }
        
        for (const auto& app_entry : _app_list) {
            if (app_entry.app == app) {
                return app_entry.isSystemApp;
            }
        }
        return false;
    }

}