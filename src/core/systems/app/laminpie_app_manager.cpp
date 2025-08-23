#include "laminpie_app_manager.hpp"
#include "laminpie_core_framework.hpp"
#include "laminpie_system_event_type.hpp"
#include "laminpie_system_internal.h"

namespace laminpie::system::app {
Laminpie_App_Manager::Laminpie_App_Manager(framework::Laminpie_Core_Framework *framework, Laminpie_App_ManagerData_t &data)
: Laminpie_App_Register(framework), 
  _event_dispatcher(framework->GetEventDispatcher()), 
  _app_manager_data(data), 
  _navigation(framework->GetAppManager().GetNavigation())
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
    if(!app){
        SYSTEM_APP_LOG_ERROR("StartApp: app is null");
        return false;
    }

    if(IsAppRunning(app)){
        SYSTEM_APP_LOG_ERROR("StartApp: app is already running: %s", app->GetName().c_str());
        return false;
    }

    if(_running_apps.size() >= _app_manager_data.app.max_running_num){
        SYSTEM_APP_LOG_ERROR("StartApp: max running num reached: %s", app->GetName().c_str());
        return false;
    }

    SYSTEM_APP_LOG_INFO("Starting app: %s", app->GetName().c_str());

    Laminpie_AppEntry entry;
    entry.app = app;
    entry.app_state = Laminpie_App_Event_Type::kApp_Status_Created;
    entry.navigation_type = Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_IDLE;

    _running_apps.push_back(entry);

    SetForegroundApp(app);

    _navigation->NavigateToApp(app->GetId());
    
    _event_dispatcher.postEvent(std::make_shared<App_EventData_t>(
        app->GetId(),
        Laminpie_App_Event_Type::kApp_Status_Created,
        app
    ));

    return true;
}

void Laminpie_App_Manager::SetForegroundApp(Laminpie_App_Base* app)
{
    _foreground_app = app;

    _update_first_element = true;
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

void Laminpie_App_Manager::ProcessAppRunningBG(Laminpie_AppEntry& entry)
{
    for(auto &entry : _running_apps){
        if(!entry.app->IsRunningBG()){
            // 前台应用立即执行
            if(entry.app->OnLoop()){
                
                entry.app_state = Laminpie_App_Event_Type::kApp_Status_Running;
            }else{
                entry.app_state = Laminpie_App_Event_Type::kApp_Status_Paused;
            }
        }else{
            if(_running_bg_cycle % 4 == 0){
                if(!entry.app->OnRunningBG()){
                    entry.app_state = Laminpie_App_Event_Type::kApp_Status_Paused;
                    SYSTEM_APP_LOG_DEBUG("Background app executed: %s", entry.app->GetName().c_str());
                }else{
                    entry.app_state = Laminpie_App_Event_Type::kApp_Status_Paused;
                    SYSTEM_APP_LOG_DEBUG("Background app executed: %s", entry.app->GetName().c_str());
                }
            }
        }
    }

    // 递增后台周期计数器
    _running_bg_cycle++;
}

void Laminpie_App_Manager::Update()
{
    SYSTEM_APP_LOG_DEBUG("Updating app manager with %zu running apps", _running_apps.size());
    
    for(auto iter = _running_apps.begin(); iter != _running_apps.end();){
        auto& entry = *iter;
        Laminpie_App_Event_Type current_app_status = entry.app->GetStatus();
        
        // 检查状态是否发生变化
        if (entry.app_state != current_app_status) {
            SYSTEM_APP_LOG_INFO("App state changed: %s [%d -> %d]", 
                               entry.app->GetName().c_str(), 
                               static_cast<int>(entry.app_state), 
                               static_cast<int>(current_app_status));
            
            // 处理状态转换并发送事件
            if (ProcessStateTransition(entry, current_app_status)) {
                entry.app_state = current_app_status;
                
                // 发送应用状态变化事件
                auto app_event = std::make_shared<App_EventData_t>(
                    entry.app->GetId(), 
                    current_app_status, 
                    entry.app
                );
                _event_dispatcher.postEvent(app_event);
            }
        }
        
        // 处理应用的循环逻辑 - 对应流程图中的决策点
        if (entry.app_state == Laminpie_App_Event_Type::kApp_Status_Running) {
            // 检查是否应该继续运行 (exit requested?)
            if (!entry.app->OnLoop()) {
                SYSTEM_APP_LOG_WARN("App loop failed, transitioning to pause: %s", entry.app->GetName().c_str());
                // 根据流程图，应该检查 should destroy?
                if (ShouldDestroyApp(entry.app)) {
                    entry.app_state = Laminpie_App_Event_Type::kApp_Status_Closed;
                } else {
                    entry.app_state = Laminpie_App_Event_Type::kApp_Status_Paused;
                }
            }
        } else if (entry.app_state == Laminpie_App_Event_Type::kApp_Status_RunningBg) {
            ProcessAppRunningBG(entry);
        } else if (entry.app_state == Laminpie_App_Event_Type::kApp_Status_Paused) {
            // 检查暂停的应用是否应该销毁 (should destroy?)
            if (ShouldDestroyApp(entry.app)) {
                entry.app_state = Laminpie_App_Event_Type::kApp_Status_Closed;
            }
        }
        
        // 处理需要销毁的应用
        if (entry.app_state == Laminpie_App_Event_Type::kApp_Status_Closed) {
            SYSTEM_APP_LOG_INFO("Removing destroyed app: %s", entry.app->GetName().c_str());
            iter = _running_apps.erase(iter);
            continue;
        }
        
        // 下个应用
        ++iter;
    }

    // 更新前台应用到第一个位置
    if(_update_first_element){
        _update_first_element = false;

        for(auto iter = _running_apps.begin(); iter != _running_apps.end(); ++iter){
            if(iter->app == _foreground_app){
                std::swap( *(_running_apps.begin()), *(iter));
                break;
            }
        }
    }
    
    // 清理前台应用引用
    if (_foreground_app && 
        (_foreground_app->GetStatus() == Laminpie_App_Event_Type::kApp_Status_Closed ||
         _foreground_app->GetStatus() == Laminpie_App_Event_Type::kApp_Status_Uninstalled)) {
        ResetActiveApp();
    }
}

bool Laminpie_App_Manager::ProcessStateTransition(Laminpie_AppEntry& entry, Laminpie_App_Event_Type new_state)
{
    SYSTEM_APP_LOG_INFO("Processing state transition: %s [%d -> %d]", 
                        entry.app->GetName().c_str(), 
                        static_cast<int>(entry.app_state), 
                        static_cast<int>(new_state));
    
    // 处理状态转换
    switch (new_state) {
        case Laminpie_App_Event_Type::kApp_Status_Created:
            // onCreate: 应用创建阶段
            CheckFalseReturn(entry.app->StartRecordResource(), false, "Start record resource failed");
            CheckFalseReturn(entry.app->OnCreate(), false, "App create failed");
            CheckFalseReturn(entry.app->SaveDisplayTheme(), false, "Save display theme failed");
            CheckFalseReturn(entry.app->EndRecordResource(), false, "End record resource failed");
            break;
            
        case Laminpie_App_Event_Type::kApp_Status_Resumed:
            // onResume: 应用恢复阶段
            CheckFalseReturn(entry.app->StartRecordResource(), false, "Start record resource failed");
            CheckFalseReturn(entry.app->OnResume(), false, "App resume failed");
            CheckFalseReturn(entry.app->SaveDisplayTheme(), false, "Save display theme failed");
            CheckFalseReturn(entry.app->EndRecordResource(), false, "End record resource failed");
            
            // 根据是否为前台应用决定下一步状态
            if (IsForegroundApp(entry.app)) {
                entry.app_state = Laminpie_App_Event_Type::kApp_Status_Running;
                _foreground_app = entry.app;
            } else {
                entry.app_state = Laminpie_App_Event_Type::kApp_Status_RunningBg;
            }
            return true; // 直接返回，因为我们已经设置了新状态
            
        case Laminpie_App_Event_Type::kApp_Status_Running:
            // onRunning: 前台运行状态
            if (_foreground_app != entry.app) {
                _foreground_app = entry.app;
                SYSTEM_APP_LOG_INFO("App became foreground: %s", entry.app->GetName().c_str());
            }
            break;
            
        case Laminpie_App_Event_Type::kApp_Status_RunningBg:
            // onRunningBg: 后台运行状态
            if (_foreground_app == entry.app) {
                ResetActiveApp();
            }
            break;
            
        case Laminpie_App_Event_Type::kApp_Status_Paused:
            // onPause: 应用暂停
            CheckFalseReturn(entry.app->OnPause(), false, "App pause failed");
            CheckFalseReturn(entry.app->ProcessPause(), false, "Pause app failed");
            break;
            
        case Laminpie_App_Event_Type::kApp_Status_Closed:
            // onClose: 应用关闭
            CheckFalseReturn(entry.app->OnClose(), false, "App close failed");
            entry.app->notifyCoreClosed();
            CheckFalseReturn(entry.app->ProcessClose(true), false, "Close app failed");
            break;
            
        case Laminpie_App_Event_Type::kApp_Status_Uninstalled:
            CheckFalseReturn(entry.app->ProcessUninstall(), false, "Uninstall app failed");
            break;
            
        default:
            SYSTEM_APP_LOG_ERROR("Unknown state: %d for app: %s", 
                                static_cast<int>(new_state), 
                                entry.app->GetName().c_str());
            return false;
    }
    
    return true;
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

bool Laminpie_App_Manager::IsForegroundApp(Laminpie_App_Base* app) const
{
    return _foreground_app == app;
}

bool Laminpie_App_Manager::ShouldDestroyApp(Laminpie_App_Base* app) const
{
    // 检查应用是否请求退出或系统要求销毁
    // 这里可以根据具体需求实现更复杂的逻辑
    return app->GetStatus() == Laminpie_App_Event_Type::kApp_Status_Closed ||
           app->GetStatus() == Laminpie_App_Event_Type::kApp_Status_Uninstalled;
}

bool Laminpie_App_Manager::IsForegroundAppRunning() const
{
    SYSTEM_APP_LOG_INFO("Checking if foreground app is running");
    return _foreground_app != nullptr;
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