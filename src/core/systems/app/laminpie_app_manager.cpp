#include "laminpie_app_manager.hpp"
#include "laminpie_core_framework.hpp"
#include "laminpie_log.hpp"
#include "laminpie_system_event_type.hpp"
#include "laminpie_system_internal.h"

namespace laminpie::system::app {
Laminpie_App_Manager::Laminpie_App_Manager(framework::Laminpie_Core_Framework *framework, Laminpie_App_ManagerData_t &data)
: Laminpie_App_Register(framework), 
  _event_dispatcher(framework->GetEventDispatcher()), 
  _app_manager_data(data)
{
    SYSTEM_APP_LOG_DEBUG("App manager initialized");

    // 注册关闭事件监听器
    _close_event_listener_id = _event_dispatcher.addEventListener<App_EventData_t>(
        Laminpie_App_Event_Type::kApp_Status_Closed,
        [this](const App_EventData_t& event) {
            this->ProcessAppCloseEvent(event);
        }
    );
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
    // 如果应用允许后台运行，则检查是否已经存在，如果存在则恢复
    if(app->GetCoreActiveData().flags.enable_running_bg) {
        for(auto &entry : _running_apps){
            if(entry.app == app){
               if(entry.app_state == Laminpie_App_Event_Type::kApp_Status_Paused) {
                entry.app_state = Laminpie_App_Event_Type::kApp_Status_Resumed;
                break;
               }
            }
        }
    }else{ // 如果不允许后台运行，则已经销毁需要重新创建
        entry.app_state = Laminpie_App_Event_Type::kApp_Status_Created;
    }

    _running_apps.push_back(entry);

    SetForegroundApp(app);
    
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

void Laminpie_App_Manager::NotFoundAppAlert(bool is_found)
{
    if(!is_found){
        SYSTEM_APP_LOG_ERROR("Running apps: %d", _running_apps.size());
        for(auto &entry : _running_apps){
            SYSTEM_APP_LOG_ERROR("Running app: %s", entry.app->GetName().c_str());
            SYSTEM_APP_LOG_ERROR("Running app state: %d", static_cast<int>(entry.app_state));
        }
    }
}
bool Laminpie_App_Manager::MoveAppToBackground(Laminpie_App_Base* app)
{
    SYSTEM_APP_LOG_INFO("Moving app to background: %s", app->GetName().c_str());
    bool is_found = false;
    for(auto &entry : _running_apps){
        if(entry.app == app){
            if(entry.app->GetCoreActiveData().flags.enable_running_bg){
                ProcessStateTransition(entry, Laminpie_App_Event_Type::kApp_Status_RunningBg);
            }else{
                SYSTEM_APP_LOG_ERROR("MoveAppToBackground: app is not allow to run bg: %s", app->GetName().c_str());
                return false;
            }
            is_found = true;
            break;
        }
    }

    NotFoundAppAlert(is_found);

    return true;
}

bool Laminpie_App_Manager::PauseApp(Laminpie_App_Base* app)
{
    SYSTEM_APP_LOG_INFO("Pausing app: %s", app->GetName().c_str());
    bool is_found = false;
    for(auto &entry : _running_apps){
        if(entry.app == app){
            ProcessStateTransition(entry, Laminpie_App_Event_Type::kApp_Status_Paused);
            is_found = true;
            break;
        }
    }

    NotFoundAppAlert(is_found);

    return true;
}

bool Laminpie_App_Manager::DestroyApp(Laminpie_App_Base* app)
{
    SYSTEM_APP_LOG_INFO("Destroying app: %s", app->GetName().c_str());
    bool is_found = false;
    for(auto &entry : _running_apps){
        if(entry.app == app){
            ProcessStateTransition(entry, Laminpie_App_Event_Type::kApp_Status_Closed);
            is_found = true;
            break;
        }
    }

    NotFoundAppAlert(is_found);

    return true;
}

void Laminpie_App_Manager::DestroyAllApps()
{
    SYSTEM_APP_LOG_INFO("Destroying all apps");
    for(auto &entry : _running_apps){
        ProcessStateTransition(entry, Laminpie_App_Event_Type::kApp_Status_Closed);
    }
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

void Laminpie_App_Manager::ProcessAppCloseEvent(const App_EventData_t& event)
{
    SYSTEM_APP_LOG_INFO("Received close event for app ID: %d", event.app_id);
    
    // 找到对应的应用并设置状态为关闭
    for (auto& entry : _running_apps) {
        if (entry.app->GetId() == event.id) {
            SYSTEM_APP_LOG_INFO("Setting app to closed state: %s", entry.app->GetName().c_str());
            entry.app_state = Laminpie_App_Event_Type::kApp_Status_Closed;
            _running_apps.erase(std::remove_if(_running_apps.begin(), _running_apps.end(), 
                [&entry](const Laminpie_AppEntry& e) { return e.app == entry.app; }), 
                _running_apps.end());
            break;
        }
    }
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
                if(current_app_status != Laminpie_App_Event_Type::kApp_Status_Closed){
                    auto app_event = std::make_shared<App_EventData_t>(
                        entry.app->GetId(), 
                        current_app_status, 
                        entry.app
                    );
                    _event_dispatcher.postEvent(app_event);
                }
            }
        }
        
        ProcessStateProgressionLogic(entry);
        
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

void Laminpie_App_Manager::ProcessStateProgressionLogic(Laminpie_AppEntry& entry)
{
    // 处理应用的循环逻辑 - 对应流程图中的决策点
    if (entry.app_state == Laminpie_App_Event_Type::kApp_Status_Created) { // 创建状态
        entry.app_state = Laminpie_App_Event_Type::kApp_Status_Resumed;
    } else if(entry.app_state == Laminpie_App_Event_Type::kApp_Status_Resumed){ // 恢复状态
        entry.app_state = Laminpie_App_Event_Type::kApp_Status_Running;
    }else if (entry.app_state == Laminpie_App_Event_Type::kApp_Status_Running) { // 运行状态
        // 如果应用不是前台应用，则设置为前台应用
        if (_foreground_app != entry.app) {
            _foreground_app = entry.app;

            _update_first_element = true;
            SYSTEM_APP_LOG_INFO("App became foreground: %s", entry.app->GetName().c_str());
        }
    } else if (entry.app_state == Laminpie_App_Event_Type::kApp_Status_RunningBg) { // 后台运行状态
        if(_foreground_app == entry.app){
            entry.app_state = Laminpie_App_Event_Type::kApp_Status_Resumed;
        }
    } else if (entry.app_state == Laminpie_App_Event_Type::kApp_Status_Paused) { // 暂停状态
        // 检查暂停的应用是否应该销毁 (should destroy?)
        if (ShouldDestroyApp(entry.app)) {
            entry.app_state = Laminpie_App_Event_Type::kApp_Status_Closed;
        }
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
            ProcessAppCreate(entry.app);
            break;
            
        case Laminpie_App_Event_Type::kApp_Status_Resumed:
            // onResume: 应用恢复阶段
            if(ProcessAppResume(entry.app)){
                entry.app_state = Laminpie_App_Event_Type::kApp_Status_Running;
            }
            break;
            
        case Laminpie_App_Event_Type::kApp_Status_Running:
            // onRunning: 前台运行状态

            if (!entry.app->OnLoop()) {
                SYSTEM_APP_LOG_WARN("App loop failed, transitioning to pause: %s", entry.app->GetName().c_str());
                // 根据流程图，应该检查 should destroy?
                if (ShouldDestroyApp(entry.app)) {
                    entry.app_state = Laminpie_App_Event_Type::kApp_Status_Closed;
                } else {
                    entry.app_state = Laminpie_App_Event_Type::kApp_Status_Paused;
                }
            }
            break;
            
        case Laminpie_App_Event_Type::kApp_Status_RunningBg:
            // onRunningBg: 后台运行状态
            ProcessAppRunningBG(entry);
            break;
            
        case Laminpie_App_Event_Type::kApp_Status_Paused:
            // onPause: 应用暂停
            CheckFalseReturn(entry.app->OnPause(), false, "App pause failed");
            CheckFalseReturn(entry.app->ProcessPause(), false, "Pause app failed");
            break;
            
        case Laminpie_App_Event_Type::kApp_Status_Closed:
            // onClose: 应用关闭
            CheckFalseReturn(entry.app->OnClose(), false, "App close failed");
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
    for(auto &entry : _running_apps){
        if(entry.app == app){
            return true;
        }
    }
    return false;
}

Laminpie_App_Base* Laminpie_App_Manager::GetForegroundApp() const
{
    SYSTEM_APP_LOG_INFO("Getting foreground app");
    return _foreground_app;
}

bool Laminpie_App_Manager::IsForegroundApp(Laminpie_App_Base* app) const
{
    return _foreground_app == app;
}

bool Laminpie_App_Manager::ShouldDestroyApp(Laminpie_App_Base* app) const
{
    return !app->GetCoreActiveData().flags.enable_running_bg;
}

bool Laminpie_App_Manager::IsForegroundAppRunning() const
{
    SYSTEM_APP_LOG_INFO("Checking if foreground app is running");
    return _foreground_app != nullptr;
}

bool Laminpie_App_Manager::ProcessAppCreate(Laminpie_App_Base *app)
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
    CheckFalseReturn(app->ProcessPause(), false, "App process pause failed");
    if(_app_manager_data.flags.enable_app_save_snapshot) {
        if(!SaveAppSnapshot(app)) {
            SYSTEM_MANAGER_LOG_ERROR("Save app snapshot failed");
        }
    }

    return true;
}

bool Laminpie_App_Manager::ProcessAppClose(Laminpie_App_Base *app){
    SYSTEM_APP_LOG_INFO("Processing app close: %s", app->GetName().c_str());

    return true;
}
}