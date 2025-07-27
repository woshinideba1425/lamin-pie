#pragma once

#include <string>
#include <vector>
#include <memory>
#include "device_types.h" // 简单的类型定义，不包含完整Device类

/**
 * @class Bus
 * @brief 纯通信总线抽象接口
 * 
 * 专注于硬件通信功能，不包含匹配或设备管理逻辑
 */
class Bus : public std::enable_shared_from_this<Bus> {
public:
    virtual ~Bus() = default;
    
    /**
     * @brief 初始化总线硬件
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief 扫描设备标识符
     * 
     * @return 总线上检测到的设备标识符列表
     * 
     * 只返回原始设备标识符(如I2C地址)，不创建Device对象
     */
    virtual std::vector<DeviceIdentifier> scanDevices() = 0;
    
    /**
     * @brief 获取总线名称
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief 执行总线传输操作
     * 
     * @param command 传输命令
     * @return 操作结果
     * 
     * 通用传输接口，具体总线类型提供更专用的方法
     */
    virtual BusTransferResult transfer(const BusCommand& command) = 0;
    
protected:
    // 无需存储设备和驱动列表
};