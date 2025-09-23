#pragma once

#include "../interface/driver.h"
#include "laminpie_i2c_bus.h"

namespace laminpie::device::drivers{

// dependencies of laminpie module
using I2c_bus = device::buses::I2c_bus;
using I2CBusCommand = device::buses::I2CBusCommand;

class I2cDriver : public Driver {
public:
    // 构造函数中进行一次性类型检查和转换
    I2cDriver(std::shared_ptr<Bus> bus);
    ~I2cDriver();
    void setBus(std::shared_ptr<Bus> bus);
    
    // 设备地址管理
    std::vector<uint16_t> getDeviceAddress() const;

    // 通用数据验证
    static uint8_t calculateCrc8(const uint8_t* data, size_t length);
    bool verifyCrc8(const uint8_t* data, size_t dataLength, uint8_t crc);

    // 通用接口    
    bool probeDevice(const DeviceIdentifier& identifier) const override;

protected:
    // 获取I2c总线的方法也不需要再做类型转换
    std::shared_ptr<I2c_bus> getI2cBus() const;

    /**
     * @brief 读取设备寄存器
     * @param deviceAddr 设备地址
     * @param regAddr 寄存器地址
     * @param value 读取值的存储位置
     * @return 操作成功返回true
     */
    bool readRegister(uint16_t deviceAddr, uint8_t regAddr, uint8_t& value);
    
    /**
     * @brief 读取设备多字节寄存器
     * @param deviceAddr 设备地址
     * @param regAddr 寄存器地址
     * @param data 数据缓冲区
     * @param length 要读取的字节数
     * @return 操作成功返回true
     */
    bool readRegisters(uint16_t deviceAddr, uint8_t regAddr, uint8_t* data, size_t length);
    
    /**
     * @brief 写入设备寄存器
     * @param deviceAddr 设备地址
     * @param regAddr 寄存器地址
     * @param value 要写入的值
     * @return 操作成功返回true
     */
    bool writeRegister(uint16_t deviceAddr, uint8_t regAddr, uint8_t value);
    
    /**
     * @brief 写入设备多字节寄存器
     * @param deviceAddr 设备地址
     * @param regAddr 寄存器地址
     * @param data 数据缓冲区
     * @param length 要写入的字节数
     * @return 操作成功返回true
     */
    bool writeRegisters(uint16_t deviceAddr, uint8_t regAddr, const uint8_t* data, size_t length);

    /**
     * @brief 读取设备
     * @param deviceAddr 设备地址
     * @param data 数据缓冲区
     * @param length 要读取的字节数
     * @return 操作成功返回true
     */
    bool readDevice(uint16_t deviceAddr, uint8_t* data, size_t length);
    
    /**
     * @brief 写入设备
     * @param deviceAddr 设备地址
     * @param data 数据缓冲区
     * @param length 要写入的字节数
     * @return 操作成功返回true
     */
    bool writeDevice(uint16_t deviceAddr, const uint8_t* data, size_t length);

    /**
     * @brief 写入并读取设备
     * @param deviceAddr 设备地址
     * @param writeData 写入数据
     * @param writeLen 写入数据长度
     * @param readData 读取数据缓冲区
     * @param readLen 读取数据长度
     * @return 操作成功返回true
     */
    bool writeReadDevice(uint16_t deviceAddr, const uint8_t* writeData, size_t writeLen, uint8_t* readData, size_t readLen);
   
    /**
     * @brief 修改寄存器中的特定位
     * @param deviceAddr 设备地址
     * @param regAddr 寄存器地址
     * @param mask 位掩码
     * @param value 要设置的值
     * @return 操作成功返回true
     */
    bool modifyRegisterBits(uint16_t deviceAddr, uint8_t regAddr, uint8_t mask, uint8_t value);

private:
    std::shared_ptr<Bus> _bus;            // 保留原始总线指针
    std::shared_ptr<I2c_bus> _i2c_bus;    // 缓存转换后的I2c_bus指针
    std::vector<uint16_t> _supportedDeviceAddress;
};
}