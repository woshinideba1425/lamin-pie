#include "laminpie_app_manager.hpp"
#include "laminpie_core_framework.hpp"
namespace laminpie::system::app {
Laminpie_App_Manager::Laminpie_App_Manager(framework::Laminpie_Core_Framework *framework, Laminpie_App_ManagerData_t &data)
: Laminpie_App_Register(framework), _event_dispatcher(framework->GetEventDispatcher()), _app_manager_data(data), _navigation(framework->GetCoreData().navigation)
{
    SYSTEM_APP_LOG_INFO("App manager initialized");
}

Laminpie_App_Manager::~Laminpie_App_Manager()
{
    SYSTEM_APP_LOG_INFO("App manager destroyed");
}

bool Laminpie_App_Manager::StartApp(app::Laminpie_App_Base* app)
{
    SYSTEM_APP_LOG_INFO("Starting app: %s", app->GetName().c_str());
    return true;
}

bool Laminpie_App_Manager::MoveAppToBackground(Laminpie_App_Base* app)
{
    SYSTEM_APP_LOG_INFO("Moving app to background: %s", app->GetName().c_str());
    return true;
}

bool Laminpie_App_Manager::PauseApp(Laminpie_App_Base* app)
{
    SYSTEM_APP_LOG_INFO("Pausing app: %s", app->GetName().c_str());
    return true;
}

bool Laminpie_App_Manager::DestroyApp(Laminpie_App_Base* app)
{
    SYSTEM_APP_LOG_INFO("Destroying app: %s", app->GetName().c_str());
    return true;
}

void Laminpie_App_Manager::DestroyAllApps()
{
    SYSTEM_APP_LOG_INFO("Destroying all apps");
}
void Laminpie_App_Manager::Update()
{

    SYSTEM_APP_LOG_INFO("Updating app manager");
}

bool Laminpie_App_Manager::IsAppRunning(Laminpie_App_Base* app) const
{
    SYSTEM_APP_LOG_INFO("Checking if app is running: %s", app->GetName().c_str());
    return true;
}

Laminpie_App_Base* Laminpie_App_Manager::GetForegroundApp() const
{
    SYSTEM_APP_LOG_INFO("Getting foreground app");
    return nullptr;
}

bool Laminpie_App_Manager::IsForegroundAppRunning() const
{
    SYSTEM_APP_LOG_INFO("Checking if foreground app is running");
    return true;
}

bool Laminpie_App_Manager::ProcessAppRun(Laminpie_App_Base *app)
{
    SYSTEM_APP_LOG_INFO("Processing app run: %s", app->GetName().c_str());
    CheckFalseReturn(app->ResetRecordResource(), false, "Reset record resource failed");
    CheckFalseReturn(app->StartRecordResource(), false, "Start record resource failed");
    if(!app->OnCreate()){
        SYSTEM_APP_LOG_ERROR("App create failed: %s", app->GetName().c_str());
        CheckFalseReturn(app->ProcessClose(true), false, "Close app failed");
        CheckFalseReturn(ProcessAppClose(app), false, "Close app failed");
        return false;
    }
    CheckFalseReturn(app->SaveDisplayTheme(), false, "Save display theme failed");
    CheckFalseReturn(app->EndRecordResource(), false, "End record resource failed");

    return true;
}

bool Laminpie_App_Manager::ProcessAppResume(Laminpie_App_Base *app)
{
    SYSTEM_APP_LOG_INFO("Processing app resume: %s", app->GetName().c_str());
    CheckFalseReturn(app->StartRecordResource(), false, "Start record resource failed");
    if(!app->OnResume()){
        SYSTEM_APP_LOG_ERROR("App resume failed: %s", app->GetName().c_str());
        CheckFalseReturn(app->ProcessClose(true), false, "Close app failed");
        CheckFalseReturn(ProcessAppClose(app), false, "Close app failed");
        return false;
    }
    CheckFalseReturn(app->SaveDisplayTheme(), false, "Save display theme failed");
    CheckFalseReturn(app->EndRecordResource(), false, "End record resource failed");

    return true;
}

bool Laminpie_App_Manager::ProcessAppPause(Laminpie_App_Base *app)
{
    SYSTEM_APP_LOG_INFO("Processing app pause: %s", app->GetName().c_str());

    return true;
}

bool Laminpie_App_Manager::ProcessAppClose(Laminpie_App_Base *app){
    SYSTEM_APP_LOG_INFO("Processing app close: %s", app->GetName().c_str());

    return true;
}
}