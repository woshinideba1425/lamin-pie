// laminpie_core_manager.hpp
#pragma once

#include "laminpie_system_event_type.hpp"
#include "laminpie_event_dispatcher.hpp"
#include "laminpie_core_framework.hpp"
#include "laminpie_system_internal.h"

namespace laminpie::system::framework::manager {
using namespace event;

class Laminpie_Core_Manager {
public:
    explicit Laminpie_Core_Manager(Laminpie_Core_Framework& core_framework);
    ~Laminpie_Core_Manager();

    // 初始化事件处理器
    bool Begin();
    
    // 清理资源
    void Cleanup();

    // 获取Core Framework引用
    Laminpie_Core_Framework& GetCoreFramework() { return _core_framework; }

private:

    // 禁止拷贝和移动
    Laminpie_Core_Manager(const Laminpie_Core_Manager&) = delete;
    Laminpie_Core_Manager& operator=(const Laminpie_Core_Manager&) = delete;
    Laminpie_Core_Manager(Laminpie_Core_Manager&&) = delete;
    Laminpie_Core_Manager& operator=(Laminpie_Core_Manager&&) = delete;

    // 注册所有事件监听器
    void RegisterEventListeners();
    
    // 设备事件处理
    void RegisterDeviceEventListeners();
    virtual bool HandleDeviceEvent(const DeviceEvent& event){return true;};
    
    // 启动事件处理
    void RegisterBootEventListeners();
    virtual bool HandleBootEvent(const Boot_EventData_t& event){return true;};
    
    // App事件处理
    void RegisterAppEventListeners();
    virtual bool HandleAppEvent(const App_EventData_t& event){return true;};
    
    // 导航事件处理
    void RegisterNavigationEventListeners();
    virtual bool HandleNavigationEvent(const Laminpie_Navigation_EventData_t& event){return true;};
    
    // UI事件处理
    void RegisterUIEventListeners();
    virtual bool HandleUIEvent(const Ui_Update_Event_t& event){return true;};

    // 成员变量
    Laminpie_Core_Framework& _core_framework;
    event::LaminPie_EventDispatcher& _event_dispatcher;
    
    // 事件监听器ID存储
    std::vector<uint32_t> _device_listener_ids;
    std::vector<uint32_t> _boot_listener_ids;
    std::vector<uint32_t> _app_listener_ids;
    std::vector<uint32_t> _navigation_listener_ids;
    std::vector<uint32_t> _ui_listener_ids;
    
    bool _initialized = false;
};

} // namespace laminpie::system::framework