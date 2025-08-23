#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <functional>
#include "../laminpie_system_internal.h"
#include "laminpie_event_dispatcher.hpp"

namespace laminpie::system::app {

// App信息结构体
struct AppInfo {
    int appId;
    std::string appName;
    int parentId;      // 父App的ID，用于层级关系
    int nextId;        // 下一个App的ID，用于线性关系
    int previousId;    // 上一个App的ID，用于线性关系
    int firstChildId;  // 第一个子App的ID，用于层级关系
    
    // 导航相关属性
    bool isSystemApp = false;  // 是否为系统App
    bool allowBackground = false; // 是否允许后台运行
    int priority = 0;          // 导航优先级
    
    AppInfo() = default;
    
    AppInfo(int id, const std::string& name)
        : appId(id), appName(name) {}
};

// 导航历史记录结构体
struct NavigationHistory {
    int appId;
    std::string appName;
    uint64_t timestamp;        // 导航时间戳
    
    NavigationHistory(int id, const std::string& name)
        : appId(id), appName(name), timestamp(0) {}
};

using namespace laminpie::system::event;

class Laminpie_App_Navigation {

public:
    // 构造函数和析构函数
    Laminpie_App_Navigation();
    ~Laminpie_App_Navigation() = default;
    
    // 禁用拷贝构造和赋值
    Laminpie_App_Navigation(const Laminpie_App_Navigation&) = delete;
    Laminpie_App_Navigation& operator=(const Laminpie_App_Navigation&) = delete;
    
    // 系统级App导航
    bool NavigateToApp(int appId);
    bool NavigateToHome();                    // 改为NavigateToHome，不带App后缀
    bool NavigateToPreviousApp();
    bool NavigateToNextApp();
    bool NavigateToParentApp();
    bool NavigateToChildApp();
    
    // 系统级导航状态
    int GetCurrentAppId() const { return _current_app_id; }
    bool IsAtHome() const { return _current_app_id == -1; }  // 新增：检查是否在home状态
    std::vector<int> GetRecentApps() const;
    int GetPreviousAppId() const;
    int GetNextAppId() const;
    int GetParentAppId() const;
    int GetFirstChildAppId() const;
    
    // App注册管理
    bool RegisterApp(int appId, const std::string& appName);
    bool UnregisterApp(int appId);
    bool SetAppRelationship(int parentId, int childId);
    bool SetAppLinearRelationship(int previousId, int currentId, int nextId);
    
    // 导航历史管理
    void ClearNavigationHistory();
    std::vector<NavigationHistory> GetNavigationHistory() const;
    bool IsAppInHistory(int appId) const;
    
    // 导航状态查询
    bool IsAppRegistered(int appId) const;
    bool IsCurrentApp(int appId) const;
    size_t GetRegisteredAppCount() const { return _registered_apps.size(); }
    
    // 导航事件回调
    using NavigationCallback = std::function<void(int fromAppId, int toAppId)>;
    void SetNavigationCallback(NavigationCallback callback) { _navigation_callback = callback; }
    
    // 调试和状态信息
    void DumpNavigationState() const;
    std::string GetNavigationStateString() const;

    // 设置事件分发器
    void SetEventDispatcher(LaminPie_EventDispatcher* dispatcher) {
        _event_dispatcher = dispatcher;
    }

    // Home状态管理
    bool CanNavigateToHome() const;
    
    // 获取当前状态描述
    std::string GetCurrentStateDescription() const;
    
    // 强制回到home状态（用于系统级操作）
    bool ForceReturnToHome();

    const AppInfo* GetAppInfo(int appId) const;

private:
    // 私有辅助方法
    bool UpdateNavigationHistory(int appId);
    void NotifyNavigationEvent(int fromAppId, int toAppId);
    bool ValidateNavigationPath(int targetAppId) const;
    
    // 导航历史管理
    void AddToHistory(int appId);
    void RemoveFromHistory(int appId);
    void TrimHistorySize();
    
    // 发送导航事件
    void SendNavigationEvent(Laminpie_App_Navigation_Event_Type event_type,
                           int source_app = -1,
                           int target_app = -1,
                           bool success = true,
                           const std::string& error_msg = "");
    
    // 成员变量 - 移除_home_app_id
    int _current_app_id = -1;                        // 当前App ID，-1表示在home状态
    int _previous_app_id = -1;                       // 上一个App ID，-1表示无效
    int _next_app_id = -1;                           // 下一个App ID，-1表示无效
    int _parent_app_id = -1;                         // 父App ID，-1表示无效
    int _first_child_app_id = -1;                    // 第一个子App ID，-1表示无效
    
    std::deque<NavigationHistory> _navigation_history;  // 导航历史记录
    std::unordered_map<int, AppInfo> _registered_apps;  // 已注册的App
    
    // 配置参数
    static constexpr size_t kMaxHistorySize = 20;   // 最大历史记录数量
    static constexpr uint64_t kHistoryTimeoutMs = 30000; // 历史记录超时时间(ms)
    
    // 回调函数
    NavigationCallback _navigation_callback;
    
    // 导航状态
    bool _is_navigating = false;                    // 是否正在导航中
    uint64_t _last_navigation_time = 0;             // 上次导航时间
    
    LaminPie_EventDispatcher* _event_dispatcher = nullptr;
};

} // namespace laminpie::system::app