#include "laminpie_app_navigation.hpp"
#include "src/core/systems/laminpie_system_internal.h"
#include <algorithm>
#include <chrono>
#include <unordered_set>

namespace laminpie::system::app {

Laminpie_App_Navigation::Laminpie_App_Navigation()
    : _current_app_id(-1), _previous_app_id(-1), 
      _next_app_id(-1), _parent_app_id(-1), _first_child_app_id(-1),
      _is_navigating(false), _last_navigation_time(0), _event_dispatcher(nullptr) {
    SYSTEM_APP_LOG_INFO("App navigation system initialized - home state ready");
}

bool Laminpie_App_Navigation::NavigateToApp(int appId) {
    if (_is_navigating) {
        SYSTEM_APP_LOG_WARN("Navigation already in progress, cannot navigate to: %s", appId.c_str());
        return false;
    }
    
    if (!ValidateAppId(appId)) {
        SYSTEM_APP_LOG_ERROR("Invalid app ID: %s", appId.c_str());
        return false;
    }
    
    if (!IsAppRegistered(appId)) {
        SYSTEM_APP_LOG_ERROR("App not registered: %s", appId.c_str());
        return false;
    }
    
    if (IsCurrentApp(appId)) {
        SYSTEM_APP_LOG_DEBUG("Already at requested app: %s", appId.c_str());
        return true;
    }
    
    // 验证导航路径
    if (!ValidateNavigationPath(appId)) {
        SYSTEM_APP_LOG_ERROR("Invalid navigation path to app: %s", appId.c_str());
        return false;
    }
    
    // 开始导航
    _is_navigating = true;
    int fromAppId = _current_app_id;
    
    // 发送导航开始事件
    SendNavigationEvent(Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_STARTED, 
                       fromAppId, appId, true);
    
    // 更新导航状态
    _previous_app_id = _current_app_id;
    _current_app_id = appId;
    
    // 更新App关系信息
    const auto& appInfo = _registered_apps.at(appId);
    _parent_app_id = appInfo.parentId;
    _first_child_app_id = appInfo.firstChildId;
    
    // 查找下一个和上一个App
    _next_app_id = appInfo.nextId;
    _previous_app_id = appInfo.previousId;
    
    // 更新导航历史
    UpdateNavigationHistory(appId);
    
    // 完成导航
    _is_navigating = false;
    _last_navigation_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    SYSTEM_APP_LOG_INFO("Successfully navigated from %s to %s", 
                       fromAppId.empty() ? "none" : fromAppId.c_str(), 
                       appId.c_str());
    
    // 发送导航完成事件
    SendNavigationEvent(Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_COMPLETED, 
                       fromAppId, appId, true);
    
    // 通知导航事件
    NotifyNavigationEvent(fromAppId, appId);
    
    return true;
}

bool Laminpie_App_Navigation::NavigateToHome() {
    if (_is_navigating) {
        SYSTEM_APP_LOG_WARN("Navigation already in progress, cannot navigate to home");
        return false;
    }
    
    if (IsAtHome()) {
        SYSTEM_APP_LOG_DEBUG("Already at home");
        return true;
    }
    
    // 开始导航到home
    _is_navigating = true;
    int fromAppId = _current_app_id;
    
    // 发送导航开始事件
    SendNavigationEvent(Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_STARTED, 
                       fromAppId, -1, true);  // target_app = -1 表示home
    
    // 更新导航状态
    _previous_app_id = _current_app_id;
    _current_app_id = -1;  // -1 表示home状态
    _parent_app_id = -1;
    _first_child_app_id = -1;
    _next_app_id = -1;
    
    // 完成导航
    _is_navigating = false;
    _last_navigation_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    SYSTEM_APP_LOG_INFO("Successfully navigated from app %d to home", fromAppId);
    
    // 发送导航完成事件
    SendNavigationEvent(Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_COMPLETED, 
                       fromAppId, -1, true);
    
    // 通知导航事件
    NotifyNavigationEvent(fromAppId, -1);
    
    return true;
}

bool Laminpie_App_Navigation::NavigateToPreviousApp() {
    if (_previous_app_id == -1) {
        SYSTEM_APP_LOG_WARN("No previous app available");
        return false;
    }
    
    return NavigateToApp(_previous_app_id);
}

bool Laminpie_App_Navigation::NavigateToNextApp() {
    if (_next_app_id == -1) {
        SYSTEM_APP_LOG_WARN("No next app available");
        return false;
    }
    
    return NavigateToApp(_next_app_id);
}

bool Laminpie_App_Navigation::NavigateToParentApp() {
    if (_parent_app_id == -1) {
        SYSTEM_APP_LOG_WARN("No parent app available");
        return false;
    }
    
    return NavigateToApp(_parent_app_id);
}

bool Laminpie_App_Navigation::NavigateToChildApp() {
    if (_first_child_app_id == -1) {
        SYSTEM_APP_LOG_WARN("No child app available");
        return false;
    }
    
    return NavigateToApp(_first_child_app_id);
}

std::vector<int> Laminpie_App_Navigation::GetRecentApps() const {
    std::vector<int> recentApps;
    recentApps.reserve(_navigation_history.size());
    
    for (const auto& history : _navigation_history) {
        recentApps.push_back(history.appId);
    }
    
    return recentApps;
}

int Laminpie_App_Navigation::GetPreviousAppId() const {
    return _previous_app_id;
}

int Laminpie_App_Navigation::GetNextAppId() const {
    return _next_app_id;
}

int Laminpie_App_Navigation::GetParentAppId() const {
    return _parent_app_id;
}

int Laminpie_App_Navigation::GetFirstChildAppId() const {
    return _first_child_app_id;
}

bool Laminpie_App_Navigation::RegisterApp(int appId, const std::string& appName) {
    if (IsAppRegistered(appId)) {
        SYSTEM_APP_LOG_WARN("App already registered: %d", appId);
        return false;
    }
    
    AppInfo appInfo(appId, appName);
    _registered_apps[appId] = appInfo;
    
    // 移除自动设置home的逻辑，因为home不再是app
    // 不再需要设置 _home_app_id
    
    // 发送App注册事件
    SendNavigationEvent(Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_APP_REGISTERED, 
                       -1, appId, true);  // source_app = -1 表示从home注册
    
    SYSTEM_APP_LOG_INFO("Successfully registered app: %d (%s)", appId, appName.c_str());
    return true;
}

bool Laminpie_App_Navigation::UnregisterApp(int appId) {
    if (!IsAppRegistered(appId)) {
        SYSTEM_APP_LOG_WARN("App not registered: %d", appId);
        return false;
    }
    
    // 检查是否为当前App
    if (IsCurrentApp(appId)) {
        SYSTEM_APP_LOG_WARN("Cannot unregister current app: %d", appId);
        return false;
    }
    
    // 移除home检查，因为home不再是app
    // 不再需要检查 IsHomeApp(appId)
    
    // 从历史记录中移除
    RemoveFromHistory(appId);
    
    // 从注册表中移除
    _registered_apps.erase(appId);
    
    // 发送App注销事件
    SendNavigationEvent(Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_APP_UNREGISTERED, 
                       appId, -1, true);  // target_app = -1 表示回到home
    
    SYSTEM_APP_LOG_INFO("Successfully unregistered app: %d", appId);
    return true;
}

bool Laminpie_App_Navigation::SetAppRelationship(int parentId, int childId) {
    if (!IsAppRegistered(parentId)) {
        SYSTEM_APP_LOG_ERROR("Parent app not registered: %d", parentId);
        return false;
    }
    
    if (!IsAppRegistered(childId)) {
        SYSTEM_APP_LOG_ERROR("Child app not registered: %d", childId);
        return false;
    }
    
    if (parentId == childId) {
        SYSTEM_APP_LOG_ERROR("Cannot set self as parent");
        return false;
    }
    
    // 设置父子关系
    _registered_apps[parentId].firstChildId = childId;
    _registered_apps[childId].parentId = parentId;
    
    // 发送关系变更事件
    SendNavigationEvent(Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_RELATIONSHIP_CHANGED, 
                       parentId, childId, true);
    
    SYSTEM_APP_LOG_INFO("Set parent-child relationship: %s -> %s", parentId.c_str(), childId.c_str());
    return true;
}

bool Laminpie_App_Navigation::SetAppLinearRelationship(int previousId, 
                                                       int currentId, 
                                                       int nextId) {
    if (!IsAppRegistered(currentId)) {
        SYSTEM_APP_LOG_ERROR("Current app not registered: %d", currentId);
        return false;
    }
    
    if (previousId >= 0 && !IsAppRegistered(previousId)) {
        SYSTEM_APP_LOG_ERROR("Previous app not registered: %d", previousId);
        return false;
    }
    
    if (nextId >= 0 && !IsAppRegistered(nextId)) {
        SYSTEM_APP_LOG_ERROR("Next app not registered: %d", nextId);
        return false;
    }
    
    // 设置线性关系
    if (previousId >= 0) {
        _registered_apps[previousId].nextId = currentId;
        _registered_apps[currentId].previousId = previousId;
    }
    
    if (nextId >= 0) {
        _registered_apps[currentId].nextId = nextId;
        _registered_apps[nextId].previousId = currentId;
    }
    
    // 发送关系变更事件
    SendNavigationEvent(Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_RELATIONSHIP_CHANGED, 
                       currentId, -1, true);
    
    SYSTEM_APP_LOG_INFO("Set linear relationship: %s <-> %s <-> %s", 
                       previousId >= 0 ? std::to_string(previousId).c_str() : "none",
                       std::to_string(currentId).c_str(),
                       nextId >= 0 ? std::to_string(nextId).c_str() : "none");
    return true;
}

void Laminpie_App_Navigation::ClearNavigationHistory() {
    _navigation_history.clear();
    SYSTEM_APP_LOG_INFO("Navigation history cleared");
}

std::vector<NavigationHistory> Laminpie_App_Navigation::GetNavigationHistory() const {
    return std::vector<NavigationHistory>(_navigation_history.begin(), _navigation_history.end());
}

bool Laminpie_App_Navigation::IsAppInHistory(int appId) const {
    return std::any_of(_navigation_history.begin(), _navigation_history.end(),
                      [&appId](const NavigationHistory& history) {
                          return history.appId == appId;
                      });
}

bool Laminpie_App_Navigation::IsAppRegistered(int appId) const {
    return _registered_apps.find(appId) != _registered_apps.end();
}

bool Laminpie_App_Navigation::IsCurrentApp(int appId) const {
    return _current_app_id == appId;
}

// 更新：获取当前状态描述
std::string Laminpie_App_Navigation::GetCurrentStateDescription() const {
    if (IsAtHome()) {
        return "HOME";
    } else {
        const AppInfo* info = GetAppInfo(_current_app_id);
        return info ? info->appName : "UNKNOWN";
    }
}

const AppInfo* Laminpie_App_Navigation::GetAppInfo(int appId) const {
    auto it = _registered_apps.find(appId);
    return (it != _registered_apps.end()) ? &(it->second) : nullptr;
}

// 更新：获取导航状态字符串
std::string Laminpie_App_Navigation::GetNavigationStateString() const {
    std::stringstream ss;
    ss << "Navigation State:\n";
    ss << "  Current: " << (IsAtHome() ? "HOME" : std::to_string(_current_app_id)) << "\n";
    ss << "  Previous: " << (_previous_app_id >= 0 ? std::to_string(_previous_app_id) : "NONE") << "\n";
    ss << "  Next: " << (_next_app_id >= 0 ? std::to_string(_next_app_id) : "NONE") << "\n";
    ss << "  Parent: " << (_parent_app_id >= 0 ? std::to_string(_parent_app_id) : "NONE") << "\n";
    ss << "  First Child: " << (_first_child_app_id >= 0 ? std::to_string(_first_child_app_id) : "NONE") << "\n";
    ss << "  Registered Apps: " << _registered_apps.size() << "\n";
    ss << "  History Size: " << _navigation_history.size() << "\n";
    return ss.str();
}

void Laminpie_App_Navigation::DumpNavigationState() const {
    SYSTEM_APP_LOG_INFO("=== Navigation State Dump ===");
    SYSTEM_APP_LOG_INFO("Current App: %s", _current_app_id.c_str());
    SYSTEM_APP_LOG_INFO("Home App: %s", _home_app_id.c_str()); // 保留_home_app_id，虽然它不再是app
    SYSTEM_APP_LOG_INFO("Previous App: %s", _previous_app_id.c_str());
    SYSTEM_APP_LOG_INFO("Next App: %s", _next_app_id.c_str());
    SYSTEM_APP_LOG_INFO("Parent App: %s", _parent_app_id.c_str());
    SYSTEM_APP_LOG_INFO("First Child App: %s", _first_child_app_id.c_str());
    SYSTEM_APP_LOG_INFO("Registered Apps: %zu", _registered_apps.size());
    SYSTEM_APP_LOG_INFO("History Size: %zu", _navigation_history.size());
    SYSTEM_APP_LOG_INFO("Is Navigating: %s", _is_navigating ? "true" : "false");
    SYSTEM_APP_LOG_INFO("Event Dispatcher: %s", _event_dispatcher ? "set" : "not set");
    SYSTEM_APP_LOG_INFO("================================");
}

bool Laminpie_App_Navigation::UpdateNavigationHistory(int appId) {
    // 从历史记录中移除已存在的相同App
    RemoveFromHistory(appId);
    
    // 添加到历史记录开头
    AddToHistory(appId);
    
    // 限制历史记录大小
    TrimHistorySize();
    
    return true;
}

void Laminpie_App_Navigation::NotifyNavigationEvent(int fromAppId, int toAppId) {
    if (_navigation_callback) {
        try {
            _navigation_callback(fromAppId, toAppId);
        } catch (const std::exception& e) {
            SYSTEM_APP_LOG_ERROR("Navigation callback exception: %s", e.what());
        }
    }
}

bool Laminpie_App_Navigation::ValidateNavigationPath(int targetAppId) const {
    // 检查是否存在循环引用
    int currentId = targetAppId;
    std::unordered_set<int> visited;
    
    while (currentId >= 0 && visited.find(currentId) == visited.end()) {
        visited.insert(currentId);
        auto it = _registered_apps.find(currentId);
        if (it == _registered_apps.end()) {
            break;
        }
        currentId = it->second.parentId;
    }
    
    return visited.find(targetAppId) == visited.end();
}

void Laminpie_App_Navigation::AddToHistory(int appId) {
    auto it = std::find_if(_navigation_history.begin(), _navigation_history.end(),
                           [&appId](const NavigationHistory& history) {
                               return history.appId == appId;
                           });
    
    if (it != _navigation_history.end()) {
        // 更新现有记录的时间戳
        it->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    } else {
        // 添加新记录
        NavigationHistory history(appId, "");
        auto appIt = _registered_apps.find(appId);
        if (appIt != _registered_apps.end()) {
            history.appName = appIt->second.appName;
        }
        history.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        _navigation_history.push_front(history);
    }
}

void Laminpie_App_Navigation::RemoveFromHistory(int appId) {
    _navigation_history.erase(
        std::remove_if(_navigation_history.begin(), _navigation_history.end(),
                      [&appId](const NavigationHistory& history) {
                          return history.appId == appId;
                      }),
        _navigation_history.end()
    );
}

void Laminpie_App_Navigation::TrimHistorySize() {
    while (_navigation_history.size() > kMaxHistorySize) {
        _navigation_history.pop_back();
    }
}

void Laminpie_App_Navigation::SendNavigationEvent(Laminpie_App_Navigation_Event_Type event_type,
                                                 int source_app,
                                                 int target_app,
                                                 bool success,
                                                 const std::string& error_msg) {
    if (!_event_dispatcher) {
        SYSTEM_APP_LOG_DEBUG("Event dispatcher not set, skipping navigation event");
        return;
    }
    
    try {
        // 创建导航事件数据
        auto nav_event = std::make_shared<Laminpie_Navigation_EventData_t>(
            event_type, source_app, target_app, "", "", success, nullptr);
        
        if (!success && !error_msg.empty()) {
            nav_event->error_message = error_msg;
        }
        
        // 发送事件
        _event_dispatcher->postEvent(nav_event);
        
        SYSTEM_APP_LOG_DEBUG("Navigation event sent: %s [%s -> %s] success=%s", 
                           nav_event->GetTypeIndexString().c_str(),
                           source_app.c_str(), target_app.c_str(),
                           success ? "true" : "false");
    } catch (const std::exception& e) {
        SYSTEM_APP_LOG_ERROR("Failed to send navigation event: %s", e.what());
    }
}

// 新增：Home状态管理
bool Laminpie_App_Navigation::CanNavigateToHome() const {
    // 检查是否可以导航到home
    // 1. 不在导航中
    // 2. 当前不在home状态
    // 3. 没有阻塞的导航操作
    return !_is_navigating && !IsAtHome();
}

bool Laminpie_App_Navigation::ForceReturnToHome() {
    SYSTEM_APP_LOG_WARN("Force returning to home state");
    
    // 强制重置所有导航状态
    int fromAppId = _current_app_id;
    
    _current_app_id = -1;
    _previous_app_id = -1;
    _next_app_id = -1;
    _parent_app_id = -1;
    _first_child_app_id = -1;
    _is_navigating = false;
    
    // 发送强制返回home事件
    SendNavigationEvent(Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_TO_HOME, 
                       fromAppId, -1, true);
    
    SYSTEM_APP_LOG_INFO("Force returned to home from app: %d", fromAppId);
    return true;
}

} // namespace laminpie::system::app