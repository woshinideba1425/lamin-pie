#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "device_types.h"

/**
 * @class Driver
 * @brief 设备驱动抽象接口
 * 
 * 负责设备初始化和操作控制，通过设备获取总线进行通信
 */
class Driver {
public:
    virtual ~Driver() = default;
    
    /**
     * @brief 获取驱动名称
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief 获取驱动版本
     */
    virtual std::string getVersion() const = 0;
    
    /**
     * @brief 获取支持的设备ID列表
     */
    virtual std::vector<DeviceIdentifier> getSupportedDeviceIds() const = 0;
    
    /**
     * @brief 检查是否支持指定设备
     */
    virtual bool probeDevice(const DeviceIdentifier& identifier) const = 0;
    
    /**
     * @brief 绑定并初始化设备
     * 
     * @param device 要绑定的设备
     * @return 初始化成功返回true
     * 
     * 当匹配成功后调用，执行设备初始化
     */
    virtual bool setupDevice(std::shared_ptr<DeviceIdentifier> device) = 0;
    
    /**
     * @brief 解绑设备
     */
    virtual void releaseDevice(std::shared_ptr<DeviceIdentifier> device) = 0;
    
    /**
     * @brief 管理电源状态
     */
    virtual bool suspend(std::shared_ptr<DeviceIdentifier> device) { return true; }
    virtual bool resume(std::shared_ptr<DeviceIdentifier> device) { return true; }
    
protected:
    /**
     * @brief 存储已绑定设备
     */
    std::unordered_map<std::string, std::shared_ptr<DeviceIdentifier>> _boundDevices;
};