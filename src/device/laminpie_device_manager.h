#pragma once

#include <condition_variable>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <map>
#include "interface/device_types.h"
#include "interface/driver.h"
#include "interface/bus.h"
#include "laminpie_event_dispatcher.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define DEVICE_SETUP_RETRY_COUNT 3
#define DEVICE_MANAGER_TASK_PRIORITY 10
#define DEVICE_MANAGER_TASK_STACK_SIZE 4096

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define CAT(a, b) a ## b
#define CONCAT(a, b) CAT(a, b)

namespace laminpie::device {

using DeviceEventType = system::event::Laminpie_Device_Event_Type;
using EventDispatcher = system::event::LaminPie_EventDispatcher;
using Event = system::event::Event<DeviceEventType>;
class DeviceManager {
public:
    // 单例模式实现
    static DeviceManager& getInstance() {
        static DeviceManager instance;
        return instance;
    }
    
    // 总线管理
    bool registerBus(std::shared_ptr<Bus> bus);
    std::shared_ptr<Bus> getBus(const std::string& busName);
    
    // 驱动管理
    bool registerDriver(std::shared_ptr<Driver> driver);
    std::shared_ptr<Driver> getDriver(const std::string& driverName);
    
    // 设备查找
    std::shared_ptr<DeviceIdentifier> findDevice(const std::string& deviceId);
    std::vector<std::shared_ptr<DeviceIdentifier>> findDevicesByType(uint16_t vendorId, uint16_t productId);
    
    // 获取设备对应的驱动
    std::shared_ptr<Driver> getDeviceDriver(const std::string& deviceId);
    
    // 事件通知
    void notifyDeviceReady(std::shared_ptr<DeviceIdentifier> device);
    void notifyDeviceRemoved(std::shared_ptr<DeviceIdentifier> device);
    void scanToAddDevices();

    // 启动设备管理系统
    bool start();
    void shutdown();

private:
    DeviceManager();
    ~DeviceManager();
    
    // 处理事件的回调函数
    void handleDeviceAdded(const laminpie::system::event::DeviceEvent& event);
    void handleDeviceRemoved(const laminpie::system::event::DeviceEvent& event);
    void handleBusScanComplete(const laminpie::system::event::DeviceEvent& event);
    void handleDriverRegistered(const laminpie::system::event::DeviceEvent& event);
    
    // 设备与驱动匹配
    void matchDriversWithDevice(std::shared_ptr<DeviceIdentifier> device);
    
    // 存储总线、设备和驱动
    std::vector<std::shared_ptr<Bus>> _buses;
    std::vector<std::shared_ptr<DeviceIdentifier>> _devices; // 改为哈希表
    std::vector<std::shared_ptr<Driver>> _drivers;  // 改为哈希表
    std::map<std::string, std::shared_ptr<Driver>> _deviceDriverMap;  // 设备ID -> 驱动
    std::multimap<std::string, std::string> _driverDeviceMap;  // 驱动名称 -> 设备ID（多对多）
    
    // 事件分发器
    EventDispatcher& _eventDispatcher = EventDispatcher::getInstance();
    
    // 事件监听器ID
    std::vector<uint32_t> _eventListenerIds;
    
    // 线程安全锁
    mutable std::mutex _mutex;

    std::thread _deviceManagerThread;

    std::atomic<bool> _running;
    
    TaskHandle_t _deviceManagerTask;
    
    // 初始化标志
    bool _initialized;
};



}

#define DEVICE_MANAGER laminpie::device::DeviceManager::getInstance()

#define bus_register(bus_name, ...) \
    static bool bus_register_##bus_name = []() { \
        ESP_LOGE("module_reg", "Registering bus: %s", #bus_name); \
        auto bus = std::make_shared<bus_name>(__VA_ARGS__); \
        bool result = DeviceManager::getInstance().registerBus(bus); \
        if (result) { \
            ESP_LOGI("module_reg", "bus %s register success", #bus_name); \
        } else { \
            ESP_LOGE("module_reg", "bus %s register failed", #bus_name); \
        } \
        return result; \
    }();

#define module_register(module_name, bus_name) \
    static bool module_register_##module_name = []() { \
        ESP_LOGI("module_reg", "register module: %s, bus: %s", #module_name, #bus_name); \
        auto bus = DeviceManager::getInstance().getBus(#bus_name); \
        if (!bus) { \
            ESP_LOGW("module_reg", "bus %s not found, module %s register failed", #bus_name, #module_name); \
            return false; \
        } \
        auto driver = std::make_shared<module_name>(bus); \
        bool result = DeviceManager::getInstance().registerDriver(driver); \
        if (result) { \
            ESP_LOGI("module_reg", "module %s register success", #module_name); \
        } else { \
            ESP_LOGE("module_reg", "module %s register failed", #module_name); \
        } \
        return result; \
    }();
