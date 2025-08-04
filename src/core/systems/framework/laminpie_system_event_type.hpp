#pragma once

#include <type_traits>
#include <string>
#include <vector>
#include <memory>
#include "interface/device_types.h"
#include "lvgl.h"
#include "misc/lv_types.h"


namespace laminpie::system::event {
// 事件基类
template<typename T>
concept EnumType = std::is_enum_v<T>;

template<EnumType T>
struct Event {
    T type;
    std::string sourceId;
    
    Event(T t, const std::string& source) : type(t), sourceId(source) {}
    virtual ~Event() = default;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// Device event type /////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class DeviceEventType {
    kDeviceAdd,
    kDeviceRemove,
    kDeviceError,
    kDeviceStatusChanged,
    kDeviceDataReady,
    kDeviceReady,
    kDriverRegistered,
    kBusScanComplete
};

// 设备事件
struct DeviceEvent : public Event<DeviceEventType> {
    std::shared_ptr<DeviceIdentifier> device;
    std::vector<std::shared_ptr<DeviceIdentifier>> devices;
    
    DeviceEvent(DeviceEventType t, std::shared_ptr<DeviceIdentifier> dev) 
        : Event<DeviceEventType>(t, dev->id), device(dev) {}
        
    DeviceEvent(DeviceEventType t, const std::vector<std::shared_ptr<DeviceIdentifier>>& devList)
        : Event<DeviceEventType>(t, "device_list"), devices(devList) {
        if (!devList.empty()) {
            device = devList[0];
        }
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// System Boot event type ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class BootEventType {
    kBoot_Stage_HardWareInit = 0,
    kBoot_Stage_SytemServiceInit,
    kBoot_Stage_BSPInit,
    kBoot_Stage_DriverInit,
    kBoot_Stage_MiddlewareInit,
    kBoot_Stage_Resourceload,
    kBoot_Stage_AppInit,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// App event type ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class Laminpie_App_Status_t {
    kApp_Status_Uninstalled = 0,
    kApp_Status_Running,
    kApp_Status_Paused,
    kApp_Status_Closed,
    kApp_Status_RunningBg
};

struct App_Status_t : public Event<Laminpie_App_Status_t> {
    std::shared_ptr<Laminpie_App_Status_t> app;
    std::vector<std::shared_ptr<Laminpie_App_Status_t>> apps;
    
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// UI event type ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class Laminpie_Ui_Event_t {
    kUI_Event_Update = 0,
};

struct Ui_Event_t : public Event<Laminpie_Ui_Event_t> {
    lv_obj_t *obj;
    lv_theme_t *theme;
    lv_event_t *event;
    lv_event_cb_t cb;
};

}