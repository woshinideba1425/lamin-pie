#include "device_manager.h"
#include "bus.h"
#include "esp_log.h"
#include <algorithm>
#include <memory>

DeviceManager::DeviceManager() : _running(true), _initialized(false) {
    //初始桶数(bucket)指定
    _buses.reserve(10);       // 预期存储约10个总线
    _devices.reserve(15);     // 预期存储约15个设备
    _drivers.reserve(15);     // 预期存储约15个驱动

    _deviceManagerTask = nullptr;
}

DeviceManager::~DeviceManager() {
    shutdown();
}

bool DeviceManager::start() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (_initialized) return true;
    
    // 启动事件分发器
    _eventDispatcher.start();
    
    // 注册事件监听器
    _eventListenerIds.push_back(_eventDispatcher.addEventListener(
        EventType::DEVICE_ADDED, 
        [this](const Event& e) { this->handleDeviceAdded(e); }
    ));
    
    _eventListenerIds.push_back(_eventDispatcher.addEventListener(
        EventType::DEVICE_REMOVED, 
        [this](const Event& e) { this->handleDeviceRemoved(e); }
    ));
    
    _eventListenerIds.push_back(_eventDispatcher.addEventListener(
        EventType::BUS_SCAN_COMPLETE, 
        [this](const Event& e) { this->handleBusScanComplete(e); }
    ));
    
    _eventListenerIds.push_back(_eventDispatcher.addEventListener(
        EventType::DRIVER_REGISTERED, 
        [this](const Event& e) { this->handleDriverRegistered(e); }
    ));
    
    _initialized = true;

    // 为每个驱动发送注册事件
    for (const auto& driver : _drivers) {
        auto event = std::make_shared<Event>(EventType::DRIVER_REGISTERED, driver->getName());
        _eventDispatcher.dispatchEvent(event);
    }

    // 让每个总线扫描设备
    for (auto& bus : _buses) {
        bus->scanDevices();
    }
    
    // 发送总线扫描完成事件
    auto event = std::make_shared<Event>(EventType::BUS_SCAN_COMPLETE, "bus_scan_complete");
    _eventDispatcher.dispatchEvent(event);

    // 创建设备管理器任务
    xTaskCreate(
        [](void* arg) -> void {
            DeviceManager* deviceManager = static_cast<DeviceManager*>(arg);
            deviceManager->scanToAddDevices();
        },
        "DeviceManager",
        DEVICE_MANAGER_TASK_STACK_SIZE,
        this,
        DEVICE_MANAGER_TASK_PRIORITY,
        &_deviceManagerTask
    );

    return true;
}

void DeviceManager::shutdown() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (!_initialized) return;
    
    // 注销所有事件监听器
    for (uint32_t id : _eventListenerIds) {
        _eventDispatcher.removeEventListener(id);
    }
    _eventListenerIds.clear();
    
    // 关闭事件分发器
    _eventDispatcher.stop();
    
    // 释放资源
    _devices.clear();
    _drivers.clear();
    _buses.clear();
    _deviceDriverMap.clear();
    _driverDeviceMap.clear();
    
    _initialized = false;
}

void DeviceManager::scanToAddDevices() {
    while(_running.load()){
        std::vector<std::shared_ptr<DeviceIdentifier>> device_list;
        for (auto& bus : _buses) {
            auto devices = bus->scanDevices();
            for (auto& device : devices) {
                device_list.push_back(std::make_shared<DeviceIdentifier>(device));
            }
        }

        auto event = std::make_shared<DeviceEvent>(EventType::DEVICE_ADDED, device_list);
        _eventDispatcher.dispatchEvent(event);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

bool DeviceManager::registerBus(std::shared_ptr<Bus> bus) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = std::find_if(_buses.begin(), _buses.end(),
                            [&bus](const std::shared_ptr<Bus>& b){
                                return b->getName() == bus->getName();
                            });    
    if (it != _buses.end()) {
        return false; // 总线已存在
    }
    
    // 初始化总线
    if (!bus->initialize()) {
        return false;
    }
    
    // 注册总线
    _buses.push_back(bus);
    
    return true;
}

bool DeviceManager::registerDriver(std::shared_ptr<Driver> driver) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    // 检查驱动是否已注册
    auto it = std::find_if(_drivers.begin(), _drivers.end(),
                            [&driver](const std::shared_ptr<Driver>& d){
                                return d->getName() == driver->getName();
                            });
    if (it != _drivers.end()) {
        return false; // 驱动已存在
    }
    
    // 添加到驱动列表
    _drivers.push_back(driver);

    if (_initialized) {
        auto event = std::make_shared<Event>(EventType::DRIVER_REGISTERED, driver->getName());
        _eventDispatcher.dispatchEvent(event);
    }
    
    return true;
}

void DeviceManager::handleDeviceAdded(const Event& event) {
    const DeviceEvent& devEvent = static_cast<const DeviceEvent&>(event);
    
    std::lock_guard<std::mutex> lock(_mutex);
    
    // 处理设备列表
    if (!devEvent.devices.empty()) {
        // 如果是设备列表事件
        for (const auto& device : devEvent.devices) {
            // 添加到设备列表
            _devices.push_back(device);
            
            // 尝试匹配驱动
            matchDriversWithDevice(device);
        }
    } else if (devEvent.device) {
        // 向后兼容处理单个设备
        // 添加到设备列表
        _devices.push_back(devEvent.device);
        
        // 尝试匹配驱动
        matchDriversWithDevice(devEvent.device);
    }
}

void DeviceManager::matchDriversWithDevice(std::shared_ptr<DeviceIdentifier> device) {
    // 遍历所有驱动尝试匹配
    for (const auto& driver : _drivers) {
        std::string driverName = driver->getName();
        for (const auto& deviceId : driver->getSupportedDeviceIds()) {
            if(deviceId.busType == device->busType && deviceId.id == device->id) {
                bool matched = false;

                if(device->isI2cDevice()){
                    if(deviceId.getI2cAddress() == 0 ||
                    deviceId.getI2cAddress() == device->getI2cAddress()){
                        matched = true;
                    }
                }else if(device->isSpiDevice()){
                    if(deviceId.getSpiChipSelect() == device->getSpiChipSelect()){
                        matched = true;
                    }
                }else if(device->isUsbDevice()){
                    if(deviceId.getUsbVendorId() == device->getUsbVendorId() &&
                    deviceId.getUsbProductId() == device->getUsbProductId()){
                        matched = true;
                    }
                }

                if(matched && driver->probeDevice(*device)){
                    for(int i = 0; i < DEVICE_SETUP_RETRY_COUNT; i++){
                        if(driver->setupDevice(device)){
                            _deviceDriverMap[device->id] = driver;
                            
                            // 添加驱动到设备的双向映射
                            _driverDeviceMap.insert({driverName, device->id});
                            
                            // 触发设备就绪事件
                            auto readyEvent = std::make_shared<DeviceEvent>(
                                EventType::DEVICE_READY,
                                device
                            );
                            _eventDispatcher.dispatchEvent(readyEvent);
                            
                            break;
                        }else{
                            vTaskDelay(1000 / portTICK_PERIOD_MS);
                        }
                    }
                }
                break;
            }
        }
    }
}

std::shared_ptr<DeviceIdentifier> DeviceManager::findDevice(const std::string& deviceId) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = std::find_if(_devices.begin(), _devices.end(),
                            [&deviceId](const std::shared_ptr<DeviceIdentifier>& d){
                                return d->id == deviceId;
                            });
    if (it != _devices.end()) {
        return *it;
    }
    return nullptr;
}

std::shared_ptr<Driver> DeviceManager::getDeviceDriver(const std::string& deviceId) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _deviceDriverMap.find(deviceId);
    if (it != _deviceDriverMap.end()) {
        return it->second;
    }
    return nullptr;
}

void DeviceManager::handleDeviceRemoved(const Event& event) {
    const DeviceEvent& devEvent = static_cast<const DeviceEvent&>(event);
    std::shared_ptr<DeviceIdentifier> device = devEvent.device;
    
    std::lock_guard<std::mutex> lock(_mutex);
    
    // 查找对应的驱动并解除绑定
    auto driverIt = _deviceDriverMap.find(device->id);
    if (driverIt != _deviceDriverMap.end()) {
        std::string driverName = driverIt->second->getName();
        
        // 调用驱动的释放方法
        driverIt->second->releaseDevice(device);
        
        // 从驱动设备映射中移除
        auto range = _driverDeviceMap.equal_range(driverName);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == device->id) {
                _driverDeviceMap.erase(it);
                break;
            }
        }
        
        // 从设备驱动映射中移除
        _deviceDriverMap.erase(driverIt);
    }
    
    // 从设备列表中移除
    auto it = std::find_if(_devices.begin(), _devices.end(),
                            [&device](const std::shared_ptr<DeviceIdentifier>& d){
                                return d->id == device->id;
                            });
    if (it != _devices.end()) {
        _devices.erase(it);
    }
}

void DeviceManager::handleBusScanComplete(const Event& event) {
    // 总线扫描完成后，可以执行一些特定操作
    // 例如：检查是否有未匹配驱动的设备，打印设备状态等
    
    std::lock_guard<std::mutex> lock(_mutex);
    
    // 打印设备匹配状态
    for (const auto& device : _devices) {
        auto driverIt = _deviceDriverMap.find(device->id);
        
        if (driverIt != _deviceDriverMap.end()) {
            // 设备已匹配驱动
            ESP_LOGI("DeviceManager", "Device %s matched with driver %s", 
                     device->id.c_str(), driverIt->second->getName().c_str());
        } else {
            // 设备未匹配驱动
            ESP_LOGI("DeviceManager", "Device %s has no matching driver", device->id.c_str());
        }
    }
}

void DeviceManager::handleDriverRegistered(const Event& event) {
    // 新驱动注册后，尝试与现有设备匹配
    std::string driverName = event.sourceId;
    
    std::lock_guard<std::mutex> lock(_mutex);
    
    // 查找新注册的驱动
    auto driverIt = std::find_if(_drivers.begin(), _drivers.end(),
                            [&driverName](const std::shared_ptr<Driver>& d){
                                return d->getName() == driverName;
                            });
    if (driverIt == _drivers.end()) {
        return; // 驱动不存在，可能是事件数据错误
    }
    
    std::shared_ptr<Driver> newDriver = *driverIt;
    
    // 尝试将新驱动与所有未匹配的设备匹配
    for (const auto& device : _devices) {
        // 跳过已经匹配驱动的设备
        if (_deviceDriverMap.find(device->id) != _deviceDriverMap.end()) {
            continue;
        }
        
        // 检查新驱动是否支持该设备
        for (const auto& supportedId : newDriver->getSupportedDeviceIds()) {
            if (supportedId.busType == device->busType && supportedId.id == device->id) {
                bool matched = false;
                
                if (device->isI2cDevice()) {
                    if (supportedId.getI2cAddress() == 0 ||
                        supportedId.getI2cAddress() == device->getI2cAddress()) {
                        matched = true;
                    }
                } else if (device->isSpiDevice()) {
                    if (supportedId.getSpiChipSelect() == device->getSpiChipSelect()) {
                        matched = true;
                    }
                } else if (device->isUsbDevice()) {
                    if (supportedId.getUsbVendorId() == device->getUsbVendorId() &&
                        supportedId.getUsbProductId() == device->getUsbProductId()) {
                        matched = true;
                    }
                }
                
                if (matched && newDriver->probeDevice(*device)) {
                    for (int i = 0; i < DEVICE_SETUP_RETRY_COUNT; i++) {
                        if (newDriver->setupDevice(device)) {
                            _deviceDriverMap[device->id] = newDriver;
                            
                            // 添加驱动到设备的双向映射
                            _driverDeviceMap.insert({driverName, device->id});
                            
                            // 触发设备就绪事件
                            auto readyEvent = std::make_shared<DeviceEvent>(
                                EventType::DEVICE_READY,
                                device
                            );
                            _eventDispatcher.dispatchEvent(readyEvent);
                            
                            break;
                        } else {
                            vTaskDelay(1000 / portTICK_PERIOD_MS);
                        }
                    }
                }
                break;
            }
        }
    }
}

std::shared_ptr<Bus> DeviceManager::getBus(const std::string& busName) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = std::find_if(_buses.begin(), _buses.end(),
                            [&busName](const std::shared_ptr<Bus>& b){
                                return b->getName() == busName;
                            });
    if (it != _buses.end()) {
        return *it;
    }
    return nullptr;
}

std::shared_ptr<Driver> DeviceManager::getDriver(const std::string& driverName) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = std::find_if(_drivers.begin(), _drivers.end(),
                            [&driverName](const std::shared_ptr<Driver>& d){
                                return d->getName() == driverName;
                            });
    if (it != _drivers.end()) {
        return *it;
    }
    return nullptr;
}

std::vector<std::shared_ptr<DeviceIdentifier>> DeviceManager::findDevicesByType(uint16_t vendorId, uint16_t productId) {
    std::lock_guard<std::mutex> lock(_mutex);
    
    std::vector<std::shared_ptr<DeviceIdentifier>> result;
    for (const auto& device : _devices) {
        if (device->isUsbDevice() && 
            device->getUsbVendorId() == vendorId && 
            device->getUsbProductId() == productId) {
            result.push_back(device);
        }
    }
    
    return result;
}

void DeviceManager::notifyDeviceReady(std::shared_ptr<DeviceIdentifier> device) {
    auto event = std::make_shared<DeviceEvent>(EventType::DEVICE_READY, device);
    _eventDispatcher.dispatchEvent(event);
}

void DeviceManager::notifyDeviceRemoved(std::shared_ptr<DeviceIdentifier> device) {
    auto event = std::make_shared<DeviceEvent>(EventType::DEVICE_REMOVED, device);
    _eventDispatcher.dispatchEvent(event);
}