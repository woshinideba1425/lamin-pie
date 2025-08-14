#pragma once

#include <functional>
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
    virtual std::type_index getTypeIndex() const = 0;
};

// 具体事件类型的基类
template<typename EnumType>
class Event : public IEvent {
public:
    EnumType type;
    
    explicit Event(EnumType event_type) : type(event_type) {}
    
    std::type_index getTypeIndex() const override {
        return std::type_index(typeid(EnumType));
    }
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
        : Event<Laminpie_Device_Event_Type>(t), device(dev) {}
        
    DeviceEvent(Laminpie_Device_Event_Type t, const std::vector<std::shared_ptr<DeviceIdentifier>>& devList)
        : Event<Laminpie_Device_Event_Type>(t), devices(devList) {
        if (!devList.empty()) {
            device = devList[0];
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
        : Event<Laminpie_Boot_Event_Type>(event_type) {}
    
    Boot_EventData_t(Laminpie_Boot_Event_Type event_type, 
                     Laminpie_Boot_Event_Type prev_phase,
                     uint8_t progress = 0)
        : Event<Laminpie_Boot_Event_Type>(event_type)
        , previous_phase(prev_phase)
        , progress_percent(progress) {}

    Boot_EventData_t(Laminpie_Boot_Event_Type event_type,
                     int err_code,
                     const std::string& err_msg,
                     Laminpie_Boot_Event_Type fail_phase)
        : Event<Laminpie_Boot_Event_Type>(event_type)
        , error_code(err_code)
        , error_message(err_msg)
        , failing_phase(fail_phase) {}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// App event type ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class Laminpie_App_Event_Type {
    kApp_Status_Uninstalled = 0,
    kApp_Status_Created,    
    kApp_Status_Running,
    kApp_Status_Paused,
    kApp_Status_Closed,
    kApp_Status_RunningBg,
    kApp_Status_Destroyed,
    kApp_Event_Type_Max,
};

struct App_EventData_t : public Event<Laminpie_App_Event_Type> {
    int id;
    Laminpie_App_Event_Type type;
    void *data;
    App_EventData_t(int app_id, Laminpie_App_Event_Type event_type, void *event_data)
        : Event<Laminpie_App_Event_Type>(event_type), id(app_id), data(event_data) {}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// Navigation event type /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class Laminpie_App_Navigation_Event_Type{
    // 应用级导航操作
    kNAVIGATE_TYPE_TO_APP,              // NavigateToApp()
    kNAVIGATE_TYPE_BACK_TO_PARENT_APP,  // NavigateBackToParentApp()
    kNAVIGATE_TYPE_TO_NEXT_APP,         // NavigateToNextApp()
    kNAVIGATE_TYPE_TO_PREVIOUS_APP,     // NavigateToPreviousApp()
    kNAVIGATE_TYPE_TO_ROOT_APP,         // 导航到根应用
    
    // 页面级导航操作
    kNAVIGATE_TYPE_TO_PAGE,             // NavigateToPage()
    kNAVIGATE_TYPE_TO_APP_PAGE,         // NavigateToAppPage()
    kNAVIGATE_TYPE_BACK_PAGE,           // NavigateBackPage()
    kNAVIGATE_TYPE_TO_NEXT_PAGE,        // NavigateToNextPage()
    kNAVIGATE_TYPE_TO_PREVIOUS_PAGE,    // NavigateToPreviousPage()
    
    // 系统级导航操作
    kNAVIGATE_TYPE_TO_HOME,             // 返回系统主页
    kNAVIGATE_TYPE_TO_RECENTS,          // 最近使用的应用
    kNAVIGATE_TYPE_SYSTEM_BACK,         // 系统级返回
    
    // 导航状态
    kNAVIGATE_TYPE_IDLE,                // 空闲状态
    kNAVIGATE_TYPE_IN_PROGRESS,         // 导航进行中
    kNAVIGATE_TYPE_COMPLETED,           // 导航完成
    kNAVIGATE_TYPE_FAILED,              // 导航失败
    
    kNAVIGATE_TYPE_MAX,
};

// 导航事件数据结构
struct Laminpie_Navigation_EventData_t : public Event<Laminpie_App_Navigation_Event_Type> {
    std::string source_app_id;      // 源应用ID
    std::string target_app_id;      // 目标应用ID  
    std::string source_page_id;     // 源页面ID
    std::string target_page_id;     // 目标页面ID
    bool navigation_success;        // 导航是否成功
    std::string error_message;      // 错误信息（如果失败）
    void* navigation_data;          // 导航相关的额外数据
    
    Laminpie_Navigation_EventData_t(Laminpie_App_Navigation_Event_Type nav_type,
                                   const std::string& src_app = "",
                                   const std::string& tgt_app = "",
                                   const std::string& src_page = "",
                                   const std::string& tgt_page = "",
                                   bool success = true,
                                   void* data = nullptr)
        : Event<Laminpie_App_Navigation_Event_Type>(nav_type),
          source_app_id(src_app), target_app_id(tgt_app),
          source_page_id(src_page), target_page_id(tgt_page),
          navigation_success(success), navigation_data(data) {}
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
};

}