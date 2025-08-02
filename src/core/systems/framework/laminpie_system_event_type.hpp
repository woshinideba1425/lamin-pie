#pragma once

#include <type_traits>
#include <string>
#include <vector>
#include <memory>
#include "interface/device_types.h"

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
template<EnumType T>
struct DeviceEvent : public Event<T> {
    std::shared_ptr<DeviceIdentifier> device;
    std::vector<std::shared_ptr<DeviceIdentifier>> devices;
    
    DeviceEvent(T t, std::shared_ptr<DeviceIdentifier> dev) 
        : Event<T>(t, dev->id), device(dev) {}
        
    DeviceEvent(T t, const std::vector<std::shared_ptr<DeviceIdentifier>>& devList)
        : Event<T>(t, "device_list"), devices(devList) {
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

typedef enum {
    kApp_Status_Uninstalled = 0,
    kApp_Status_Running,
    kApp_Status_Paused,
    kApp_Status_Closed,
    kApp_Status_RunningBg
} Laminpie_App_Status_t;

template<EnumType T>
struct App_Status_t : public Event<T> {
    std::shared_ptr<Laminpie_App_Status_t> app;
    std::vector<std::shared_ptr<Laminpie_App_Status_t>> apps;
    
};


}