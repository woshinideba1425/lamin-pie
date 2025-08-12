#pragma once
#include <vector>
#include "laminpie_app_base.hpp"
#include "laminpie_app_register.h"
#include "laminpie_system_event_type.hpp"
#include "laminpie_system_internal.h"
#include "laminpie_app_navigation.hpp"

namespace laminpie::system::app {
struct Laminpie_AppEntry {
    app::Laminpie_App_Base* app;
    Laminpie_App_Status_t app_state;
    Laminpie_App_Navigation_Type_t navigation_type;
};    

typedef struct {
    struct {
        int max_running_num;
    } app;
    struct {
        uint8_t enable_app_save_snapshot: 1;
    } flags;
} Laminpie_App_ManagerData_t;

class Laminpie_App_Manager : public Laminpie_App_Register {
public:
    Laminpie_App_Manager(framework::Laminpie_Core_Framework *framework, Laminpie_App_ManagerData_t &data);
    
    ~Laminpie_App_Manager();

    bool StartApp(app::Laminpie_App_Base* app);
    bool MoveAppToBackground(Laminpie_App_Base* app);
    bool PauseApp(Laminpie_App_Base* app);
    bool DestroyApp(Laminpie_App_Base* app);
    void DestroyAllApps();
    void Update();
    bool IsAppRunning(Laminpie_App_Base* app) const;
    Laminpie_App_Base* GetForegroundApp() const;
    bool IsForegroundAppRunning() const;

protected:
    LaminPie_EventDispatcher &_event_dispatcher;

    virtual bool ProcessAppRunExtra(Laminpie_App_Base *app)    { return true; }
    virtual bool ProcessAppResumeExtra(Laminpie_App_Base *app) { return true; }
    virtual bool ProcessAppPauseExtra(Laminpie_App_Base *app)  { return true; }
    virtual bool ProcessAppCloseExtra(Laminpie_App_Base *app)  { return true; }
    virtual bool ProcessNavigationEvent(Laminpie_App_Navigation_Type_t type) { return true; };

    bool ProcessAppRun(Laminpie_App_Base *app);
    bool ProcessAppResume(Laminpie_App_Base *app);
    bool ProcessAppPause(Laminpie_App_Base *app);
    bool ProcessAppClose(Laminpie_App_Base *app);
    bool SaveAppSnapshot(Laminpie_App_Base *app);
    bool ReleaseAppSnapshot(Laminpie_App_Base *app);
    void ResetActiveApp(void);

    const Laminpie_App_ManagerData_t &_app_manager_data;

private:
    std::vector<Laminpie_AppEntry> _running_apps;   // 运行中的应用列表
    Laminpie_App_Base* _foreground_app;      // 当前前台应用
    Laminpie_App_Navigation _navigation;
};
}