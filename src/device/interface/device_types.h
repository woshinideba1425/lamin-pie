#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

enum class DeviceStatus {
    DISCOVERED,   // 已发现但未匹配驱动
    INITIALIZED,  // 已初始化
    ACTIVE,       // 活跃中
    SUSPENDED,    // 已挂起
    ERROR,        // 错误状态
    REMOVED       // 已移除
};

/**
 * @struct DeviceIdentifier
 * @brief 总线上的原始设备标识符
 */
struct DeviceIdentifier {
    enum BusType {
        BUS_I2C,
        BUS_SPI,
        BUS_UART,
        BUS_USB
    } busType;
    
    union {
        struct {
            uint16_t address;
        } i2c;
        
        struct {
            uint8_t chipSelect;
        } spi;
        
        struct {
            uint16_t vendorId;
            uint16_t productId;
        } usb;
    };
    
    std::string id;
    std::string version;
    DeviceStatus status;
    std::string errorMessage;
    
    DeviceIdentifier() : busType(BUS_I2C), id(""), version("") {
        memset(&i2c, 0, sizeof(i2c));
    }
    
    DeviceIdentifier(BusType bt, const std::string& id) 
        : busType(bt), id(id), version("") {
        memset(&i2c, 0, sizeof(i2c));
    }
    
    bool operator==(const DeviceIdentifier& other) const {
        return busType == other.busType && id == other.id;
    }

    bool isI2cDevice() const {
        return busType == BUS_I2C;
    }

    uint16_t getI2cAddress() const {
        return i2c.address;
    }

    bool isSpiDevice() const {
        return busType == BUS_SPI;
    }

    uint8_t getSpiChipSelect() const {
        return spi.chipSelect;
    }
    
    bool isUsbDevice() const {
        return busType == BUS_USB;
    }

    uint16_t getUsbVendorId() const {
        return usb.vendorId;
    }

    uint16_t getUsbProductId() const {
        return usb.productId;
    }
    
};

/**
 * @struct BusCommand
 * @brief 总线传输命令
 */
struct BusCommand {
    DeviceIdentifier deviceId_;
    std::vector<uint8_t> writeData_;
    size_t readSize_;
    // 其他传输参数
    BusCommand(DeviceIdentifier deviceId, std::vector<uint8_t> &writeData, size_t readSize)
        : deviceId_(deviceId), writeData_(writeData), readSize_(readSize) {}
};

/**
 * @struct BusTransferResult
 * @brief 总线传输结果
 */
struct BusTransferResult {
    bool success_;
    std::vector<uint8_t> readData_;
    int errorCode_;
    std::string errorMessage_;
};