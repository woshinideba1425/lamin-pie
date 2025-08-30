#pragma once

#include <functional>
#include <string_view>
#include <type_traits>
#include <string>
#include <vector>
#include <memory>
#include "interface/device_types.h"
#include "lvgl.h"
#include "misc/lv_types.h"
#include <typeindex>
#include <chrono>

namespace laminpie::system::event {
// 事件基类
template<typename T>
concept EnumType = std::is_enum_v<T>;

// 基础事件接口
class IEvent {
public:
    virtual ~IEvent() = default;
    virtual std::type_index GetTypeIndex() const = 0;
    virtual std::string GetTypeIndexString() const = 0;
    virtual std::string GetEventName() const = 0;
};

// 具体事件类型的基类
template<typename EnumType>
class Event : public IEvent {
public:
    // 类型别名，让继承类可以访问枚举类型
    using EnumTypeAlias = EnumType;
    
    EnumType type;
    std::string name;
    
    explicit Event(EnumType event_type, std::string_view event_name) 
        : type(event_type), name(event_name) {}
    
    std::type_index GetTypeIndex() const override {
        return std::type_index(typeid(EnumType));
    }

    std::string GetEventName() const override {
        return name;
    }
    
    // 纯虚函数，由继承类实现
    std::string GetTypeIndexString() const override = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// Device event type /////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class Laminpie_Device_Event_Type {
    kDeviceAdd,
    kDeviceRemove,
    kDeviceError,
    kDeviceStatusChanged,
    kDeviceDataReady,
    kDeviceReady,
    kDriverRegistered,
    kBusScanComplete,
    kDevice_Event_Type_Max,
};

// 设备事件
struct DeviceEvent : public Event<Laminpie_Device_Event_Type> {
    std::shared_ptr<DeviceIdentifier> device;
    std::vector<std::shared_ptr<DeviceIdentifier>> devices;
    
    DeviceEvent(Laminpie_Device_Event_Type t, std::shared_ptr<DeviceIdentifier> dev) 
        : Event<Laminpie_Device_Event_Type>(t, "exDevice"), device(dev) {}
        
    DeviceEvent(Laminpie_Device_Event_Type t, const std::vector<std::shared_ptr<DeviceIdentifier>>& devList)
        : Event<Laminpie_Device_Event_Type>(t, "exDevice"), devices(devList) {
        if (!devList.empty()) {
            device = devList[0];
        }
    }
    
    // 实现 GetTypeIndexString 接口
    std::string GetTypeIndexString() const override {
        switch (type) {
            case Laminpie_Device_Event_Type::kDeviceAdd: return "DeviceAdd";
            case Laminpie_Device_Event_Type::kDeviceRemove: return "DeviceRemove";
            case Laminpie_Device_Event_Type::kDeviceError: return "DeviceError";
            case Laminpie_Device_Event_Type::kDeviceStatusChanged: return "DeviceStatusChanged";
            case Laminpie_Device_Event_Type::kDeviceDataReady: return "DeviceDataReady";
            case Laminpie_Device_Event_Type::kDeviceReady: return "DeviceReady";
            case Laminpie_Device_Event_Type::kDriverRegistered: return "DriverRegistered";
            case Laminpie_Device_Event_Type::kBusScanComplete: return "BusScanComplete";
            default: return "Unknown";
        }
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// System Boot event type ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class Laminpie_Boot_Event_Type {
    kBoot_Stage_HardWareInit = 0,
    kBoot_Stage_SytemServiceInit,
    kBoot_Stage_BSPInit,
    kBoot_Stage_DriverInit,
    kBoot_Stage_MiddlewareInit,
    kBoot_Stage_Resourceload,
    kBoot_Stage_AppInit,
    kBoot_Stage_Complete,
    kBoot_Stage_Failed,
    kBoot_Event_Type_Max,
};

struct Boot_EventData_t : public Event<Laminpie_Boot_Event_Type> {
    // phase & progress
    Laminpie_Boot_Event_Type previous_phase {Laminpie_Boot_Event_Type::kBoot_Event_Type_Max};
    Laminpie_Boot_Event_Type next_phase {Laminpie_Boot_Event_Type::kBoot_Event_Type_Max};
    uint8_t                  progress_percent {0};
    uint16_t                 step_index {0};
    uint16_t                 total_steps {7}; // 7个启动阶段

    // diagnostics
    int                      error_code {0};
    std::string              error_message;
    Laminpie_Boot_Event_Type failing_phase {Laminpie_Boot_Event_Type::kBoot_Event_Type_Max};
    uint32_t                 retry_count {0};

    // observability
    std::string              request_id;
    std::chrono::steady_clock::time_point timestamp {std::chrono::steady_clock::now()};

    // execution metadata
    const char*              source_module {"BOOT"};

    // constructors
    explicit Boot_EventData_t(Laminpie_Boot_Event_Type event_type)
        : Event<Laminpie_Boot_Event_Type>(event_type, "Boot") {}
    
    Boot_EventData_t(Laminpie_Boot_Event_Type event_type, 
                     Laminpie_Boot_Event_Type prev_phase,
                     uint8_t progress = 0)
        : Event<Laminpie_Boot_Event_Type>(event_type, "Boot")
        , previous_phase(prev_phase)
        , progress_percent(progress) {}

    Boot_EventData_t(Laminpie_Boot_Event_Type event_type,
                     int err_code,
                     const std::string& err_msg,
                     Laminpie_Boot_Event_Type fail_phase)
        : Event<Laminpie_Boot_Event_Type>(event_type, "Boot")
        , error_code(err_code)
        , error_message(err_msg)
        , failing_phase(fail_phase) {}
    
    // 实现 GetTypeIndexString 接口
    std::string GetTypeIndexString() const override {
        switch (type) {
            case Laminpie_Boot_Event_Type::kBoot_Stage_HardWareInit: return "HardwareInit";
            case Laminpie_Boot_Event_Type::kBoot_Stage_SytemServiceInit: return "SystemServiceInit";
            case Laminpie_Boot_Event_Type::kBoot_Stage_BSPInit: return "BSPInit";
            case Laminpie_Boot_Event_Type::kBoot_Stage_DriverInit: return "DriverInit";
            case Laminpie_Boot_Event_Type::kBoot_Stage_MiddlewareInit: return "MiddlewareInit";
            case Laminpie_Boot_Event_Type::kBoot_Stage_Resourceload: return "ResourceLoad";
            case Laminpie_Boot_Event_Type::kBoot_Stage_AppInit: return "AppInit";
            case Laminpie_Boot_Event_Type::kBoot_Stage_Complete: return "Complete";
            case Laminpie_Boot_Event_Type::kBoot_Stage_Failed: return "Failed";
            default: return "Unknown";
        }
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// App event type ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class Laminpie_App_Event_Type {
    kApp_Status_Uninstalled = 0,
    kApp_Status_Created,    
    kApp_Status_Running,
    kApp_Status_Resumed,
    kApp_Status_Paused,
    kApp_Status_Closed,
    kApp_Status_RunningBg,
    kApp_Event_Type_Max,
};

struct App_EventData_t : public Event<Laminpie_App_Event_Type> {
    int id;
    Laminpie_App_Event_Type type;
    void *data;
    
    App_EventData_t(int app_id, Laminpie_App_Event_Type event_type, void *event_data)
        : Event<Laminpie_App_Event_Type>(event_type,"App"), id(app_id), data(event_data) {}
    
    // 实现 GetTypeIndexString 接口
    std::string GetTypeIndexString() const override {
        switch (type) {
            case Laminpie_App_Event_Type::kApp_Status_Uninstalled: return "Uninstalled";
            case Laminpie_App_Event_Type::kApp_Status_Created: return "Created";
            case Laminpie_App_Event_Type::kApp_Status_Running: return "Running";
            case Laminpie_App_Event_Type::kApp_Status_Paused: return "Paused";
            case Laminpie_App_Event_Type::kApp_Status_Closed: return "Closed";
            case Laminpie_App_Event_Type::kApp_Status_RunningBg: return "RunningBackground";
            default: return "Unknown";
        }
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// Navigation event type /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class Laminpie_App_Navigation_Event_Type {
    // 系统级App导航事件
    kNAVIGATE_TYPE_TO_APP,                    // NavigateToApp
    kNAVIGATE_TYPE_TO_PREVIOUS_APP,           // NavigateToPreviousApp
    kNAVIGATE_TYPE_TO_NEXT_APP,               // NavigateToNextApp
    kNAVIGATE_TYPE_TO_PARENT_APP,             // NavigateToParentApp
    kNAVIGATE_TYPE_TO_CHILD_APP,              // NavigateToChildApp
    
    // 导航状态事件
    kNAVIGATE_TYPE_STARTED,                   // 导航开始
    kNAVIGATE_TYPE_COMPLETED,                 // 导航完成
    kNAVIGATE_TYPE_FAILED,                    // 导航失败
    kNAVIGATE_TYPE_CANCELLED,                 // 导航取消
    
    // 导航管理事件
    kNAVIGATE_TYPE_APP_REGISTERED,            // App注册
    kNAVIGATE_TYPE_APP_UNREGISTERED,          // App注销
    kNAVIGATE_TYPE_RELATIONSHIP_CHANGED,      // 关系变更
    
    // 保留原有类型以兼容现有代码
    kNAVIGATE_TYPE_TO_PAGE,                   // 页面级导航（App内部使用）
    kNAVIGATE_TYPE_TO_APP_PAGE,               // App页面导航
    kNAVIGATE_TYPE_BACK_PAGE,                 // 页面返回
    kNAVIGATE_TYPE_TO_NEXT_PAGE,              // 下一页
    kNAVIGATE_TYPE_TO_PREVIOUS_PAGE,          // 上一页
    kNAVIGATE_TYPE_TO_HOME,                   // 回到主页
    kNAVIGATE_TYPE_TO_RECENTS,                // 最近应用
    kNAVIGATE_TYPE_SYSTEM_BACK,               // 系统返回
    
    // 状态标识
    kNAVIGATE_TYPE_IDLE,                      // 空闲状态
    kNAVIGATE_TYPE_IN_PROGRESS,               // 进行中
    
    kNAVIGATE_TYPE_MAX
};

// 导航事件数据结构
struct Laminpie_Navigation_EventData_t : public Event<Laminpie_App_Navigation_Event_Type> {
    std::string source_app_id;      // 源应用ID
    std::string target_app_id;      // 目标应用ID  
    std::string source_page_id;     // 源页面ID（可选，页面级导航时使用）
    std::string target_page_id;     // 目标页面ID（可选，页面级导航时使用）
    bool navigation_success;        // 导航是否成功
    std::string error_message;      // 错误信息（如果失败）
    void* navigation_data;          // 导航相关的额外数据
    
    // 新增字段
    uint64_t navigation_timestamp;  // 导航时间戳
    std::string navigation_type;    // 导航类型描述
    
    Laminpie_Navigation_EventData_t(Laminpie_App_Navigation_Event_Type nav_type,
                                   const std::string& src_app = "",
                                   const std::string& tgt_app = "",
                                   const std::string& src_page = "",
                                   const std::string& tgt_page = "",
                                   bool success = true,
                                   void* data = nullptr)
        : Event<Laminpie_App_Navigation_Event_Type>(nav_type, "Navigation"),
          source_app_id(src_app), target_app_id(tgt_app),
          source_page_id(src_page), target_page_id(tgt_page),
          navigation_success(success), navigation_data(data),
          navigation_timestamp(0), navigation_type("") {}
    
    // 更新GetTypeIndexString实现
    std::string GetTypeIndexString() const override {
        switch (type) {
            // 系统级App导航
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_TO_APP: return "NavigateToApp";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_TO_PREVIOUS_APP: return "NavigateToPreviousApp";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_TO_NEXT_APP: return "NavigateToNextApp";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_TO_PARENT_APP: return "NavigateToParentApp";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_TO_CHILD_APP: return "NavigateToChildApp";
            
            // 导航状态
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_STARTED: return "NavigationStarted";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_COMPLETED: return "NavigationCompleted";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_FAILED: return "NavigationFailed";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_CANCELLED: return "NavigationCancelled";
            
            // 导航管理
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_APP_REGISTERED: return "AppRegistered";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_APP_UNREGISTERED: return "AppUnregistered";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_RELATIONSHIP_CHANGED: return "RelationshipChanged";
            
            // 页面级导航（保留兼容性）
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_TO_PAGE: return "NavigateToPage";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_TO_APP_PAGE: return "NavigateToAppPage";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_BACK_PAGE: return "NavigateBackPage";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_TO_NEXT_PAGE: return "NavigateToNextPage";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_TO_PREVIOUS_PAGE: return "NavigateToPreviousPage";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_TO_HOME: return "NavigateToHome";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_TO_RECENTS: return "NavigateToRecents";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_SYSTEM_BACK: return "SystemBack";
            
            // 状态标识
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_IDLE: return "Idle";
            case Laminpie_App_Navigation_Event_Type::kNAVIGATE_TYPE_IN_PROGRESS: return "InProgress";
            
            default: return "Unknown";
        }
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// UI event type ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class Laminpie_UI_Event_Type {
    kUI_Event_Update = 0,
    kUI_Event_Type_Max,
};

struct Ui_Update_Event_t : public Event<Laminpie_UI_Event_Type> {
    lv_obj_t *obj;
    lv_theme_t *theme;
    lv_event_code_t event;
    lv_event_cb_t cb;
    void *user_data;
    
    // 实现 GetTypeIndexString 接口
    std::string GetTypeIndexString() const override {
        switch (type) {
            case Laminpie_UI_Event_Type::kUI_Event_Update: return "UIUpdate";
            default: return "Unknown";
        }
    }
};

}