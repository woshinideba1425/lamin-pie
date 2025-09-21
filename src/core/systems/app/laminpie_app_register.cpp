#include "laminpie_app_register.h"
#include "laminpie_app_base.hpp"
#include "laminpie_core_display.hpp"
#include "laminpie_core_framework.hpp"
#include "laminpie_log.hpp"
#include "laminpie_system_internal.h"

using namespace laminpie::utils;

namespace laminpie::system::app {
    Laminpie_App_Register::Laminpie_App_Register(framework::Laminpie_Core_Framework &framework) 
    : _framework(framework), _navigation(&_framework.GetAppNavigation()) {
        SYSTEM_APP_LOG_DEBUG("App register initialized");
    }

    int Laminpie_App_Register::Install(Laminpie_App_Base* app, void* userData) {
        bool app_installed = false;
        bool home_process_app_installed = false;
        bool ret = true;
        lv_area_t app_visual_area = {};
        Laminpie_CoreHome &home = _framework._core_display;
        CheckNullAndReturn(app, -1, "Invalid app");

        for (auto it = _id_installed_app_map.begin(); it != _id_installed_app_map.end(); it++ ){
            CheckFalseReturn(it->second != app, -1, "Already installed");
        }
        CheckFalseReturn(app->OnSetup(), false, "App setup failed");
        app_installed = app->ProcessInstall(&_framework, _app_free_id);
        ret = _id_installed_app_map.insert(std::pair <int, Laminpie_App_Base *>(app->_id, app)).second;
        ret = home.GetAppVisualArea(app, app_visual_area);
        ret = app->SetVisualArea(app_visual_area);
        ret = app->CalibrateVisualArea();

        ret = home.ProcessAppInstall(app);
        if (!ret){
            SYSTEM_APP_LOG_ERROR("Home process app install failed: %s", app->GetName().c_str());
            return -1;
        }

        if(ret){
            _app_free_id++;
        }

        if (!ret){
            if (home_process_app_installed && !home.ProcessAppUninstall(app)){
                SYSTEM_APP_LOG_ERROR("Home process app uninstall failed");
            }
            if (app_installed && !app->ProcessUninstall()){
                SYSTEM_APP_LOG_ERROR("App uninstall failed");
            }
            _id_installed_app_map.erase(app->_id);
            return -1;
        }
        _navigation->RegisterApp(app->_id, app->GetName());
        return app->_id;
    }

    int Laminpie_App_Register::Install(Laminpie_App_Base &app) {
        return Install(&app);
    }

    // 卸载应用
    bool Laminpie_App_Register::Uninstall(Laminpie_App_Base* app) 
    {
        bool ret = true;
        int app_id = -1;
        Laminpie_CoreHome &home = _framework._core_display;

        CheckNullAndReturn(app,false,"Invalid app");
        app_id = app->_id;

        SYSTEM_APP_LOG_DEBUG("Uninstall App(%d)", app_id);

        auto it = _id_installed_app_map.begin();
        for (; it != _id_installed_app_map.end(); it++){
            if(it->second == app){
                break;
            }
        }
        CheckFalseReturn((it->second == app), false, "App(%d) is not installed", app_id);

        CheckFalseReturn(home.ProcessAppUninstall(app), false, "Home process app uninstall failed");

        ret = app->ProcessUninstall();
        if(!ret){
            SYSTEM_APP_LOG_ERROR("App uninstall failed");
        }

        CheckFalseReturn(_id_installed_app_map.erase(app_id) > 0, false, "Remove app failed");

        return ret;
    }

    bool Laminpie_App_Register::Uninstall(Laminpie_App_Base &app)
    {
        return Uninstall(&app);
    }

    bool Laminpie_App_Register::Uninstall(int id)
    {
        Laminpie_App_Base *app = nullptr;
        SYSTEM_APP_LOG_DEBUG("Uninstall App(%d)", id);

        app = GetInstalledApp(id);
        CheckNullAndReturn(app, false, "Get installed app failed");

        CheckFalseReturn(Uninstall(app), false, "Uninstall app failed");

        return true;
    }

    Laminpie_App_Base *Laminpie_App_Register::GetInstalledApp(int id)
    {
        auto it = _id_installed_app_map.find(id);
        if(it != _id_installed_app_map.end()){
            return it->second;
        }
        return nullptr;
    }
    // 注册系统应用
    bool Laminpie_App_Register::InstallSystemApp(Laminpie_App_Base* app)
    {
        app->SetSystemApp(true);
        return Install(app);
    }

    // 判断应用是否为系统应用
    bool Laminpie_App_Register::IsSystemApp(Laminpie_App_Base* app) const {
        CheckNullAndReturn(app, false, "Invalid app");
        return app->IsSystemApp();
    }

    int Laminpie_App_Register::GetAppFreeId(){
        return _app_free_id++;
    }

}