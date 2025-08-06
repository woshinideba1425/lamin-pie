#include "laminpie_app_register.h"

namespace laminpie::system::app {


    // 获取应用的 ID
    int Laminpie_App_Register::GetAppId(Laminpie_App_Base* app) {
        for (const auto& app_entry : _app_list) {
            if (app_entry.app == app) {
                return app_entry.id;
            }
        }
        return -1;
    }

    // 通过应用名称获取应用 ID
    int Laminpie_App_Register::GetAppId(const char* name) {
        for (const auto& app_entry : _app_list) {
            if (app_entry.app->GetName() == name) {
                return app_entry.id;
            }
        }
        return -1;
    }

    // 根据 ID 获取应用实例
    Laminpie_App_Base* Laminpie_App_Register::GetApp(int id) {
        for (const auto& app_entry : _app_list) {
            if (app_entry.id == id) {
                return app_entry.app;
            }
        }
        return nullptr;
    }

    // 根据名称获取应用实例
    Laminpie_App_Base* Laminpie_App_Register::GetApp(const char* name) {
        for (const auto& app_entry : _app_list) {
            if (app_entry.app->GetName() == name) {
                return app_entry.app;
            }
        }
        return nullptr;
    }

    // 卸载应用
    bool Laminpie_App_Register::Uninstall(Laminpie_App_Base* app) {
        if (app == nullptr) {
            return false;
        }

        // 遍历应用列表，找到匹配的应用并移除
        for (auto iter = _app_list.begin(); iter != _app_list.end(); ++iter) {
            if (iter->app == app) {
                app->SetRegistered(false);
                _app_list.erase(iter);
                return true;
            }
        }
        return false;
    }

    // 注册系统应用
    bool Laminpie_App_Register::RegisterSystemApp(Laminpie_App_Base* app)
    {
        if (app == nullptr) {
            SYSTEM_APP_LOG_ERROR("App is null, cannot register as system app.");
            return false;
        }

        SYSTEM_APP_LOG_INFO("Registering system app: %s", app->GetName().c_str());
        
        // 查找应用并标记为系统应用
        for (auto& appEntry : _app_list) {
            if (appEntry.app == app) {
                appEntry.isSystemApp = true;
                app->SetSystemApp(true);
                SYSTEM_APP_LOG_INFO("App %s is registered as system app", app->GetName().c_str());
                return true;
            }
        }
        
        SYSTEM_APP_LOG_ERROR("App %s not found, cannot register as system app", app->GetName().c_str());
        return false;
    }

    // 判断应用是否为系统应用
    bool Laminpie_App_Register::IsSystemApp(Laminpie_App_Base* app) const {
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