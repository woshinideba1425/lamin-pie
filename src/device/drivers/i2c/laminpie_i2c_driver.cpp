#include "laminpie_i2c_driver.h"
#include <vector>

namespace laminpie::device::drivers{
I2cDriver::I2cDriver(std::shared_ptr<Bus> bus) : _bus(bus) {
    // 只进行一次RTTI操作
    _i2c_bus = std::static_pointer_cast<I2c_bus>(bus);
    if (!_i2c_bus) {
        // 处理错误：总线不是I2c_bus类型
        throw std::invalid_argument("Bus is not an I2c_bus");
    }
    _supportedDeviceAddress.clear();
    _supportedDeviceAddress.reserve(10);
}

I2cDriver::~I2cDriver() {
    _supportedDeviceAddress.clear();
}

void I2cDriver::setBus(std::shared_ptr<Bus> bus) {
    _bus = bus;
    _i2c_bus = std::static_pointer_cast<I2c_bus>(bus);
    if (!_i2c_bus) {
        throw std::invalid_argument("Bus is not an I2c_bus");
    }
}

std::vector<uint16_t> I2cDriver::getDeviceAddress() const {
    return _supportedDeviceAddress;
}

bool I2cDriver::probeDevice(const DeviceIdentifier& identifier) const {
    uint8_t deviceAddress = identifier.getI2cAddress();
    // 检查设备是否支持
    if (!identifier.isI2cDevice()) {
        std::cout << "Device is not an I2c device" << std::endl;
        return false;
    }
    // 检查设备地址是否有效
    if (identifier.getI2cAddress() == 0 || identifier.getI2cAddress() > 127) {
        std::cout << "Device address is invalid" << std::endl;
        return false;
    }

    if (std::find(_supportedDeviceAddress.begin(), _supportedDeviceAddress.end(), deviceAddress) != _supportedDeviceAddress.end()) {
        I2CBusCommand cmd = _i2c_bus->prepareProbeCommand(deviceAddress);
        BusTransferResult result = _i2c_bus->transfer(cmd);
        if (result.success_) {
            std::cout << "Device is found" << std::endl;
            return true;
        }
    }

    std::cout << "Device is not found" << std::endl;
    return false;
}

bool I2cDriver::verifyCrc8(const uint8_t* data, size_t dataLength, uint8_t crc) {
    uint8_t calculatedCrc = calculateCrc8(data, dataLength);
    return calculatedCrc == crc;
}

uint8_t I2cDriver::calculateCrc8(const uint8_t* data, size_t length) {
    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
    }
    return crc;
}

std::shared_ptr<I2c_bus> I2cDriver::getI2cBus() const {
    return _i2c_bus;
}

// 实现读寄存器函数
bool I2cDriver::readRegister(uint16_t deviceAddr, uint8_t regAddr, uint8_t& value) {
    // 创建I2C命令
    I2CBusCommand cmd;
    cmd.deviceAddress = deviceAddr;
    cmd.transferType = I2CBusCommand::WRITE_READ;
    cmd.readSize = 1;
    cmd.options.noStop = false;
    
    // 准备寄存器地址数据
    BufferView view = _i2c_bus->prepareRegisterData(deviceAddr, regAddr, nullptr, 0);
    if (!view.empty()) {
        cmd.writeData = std::move(view);
        
        // 执行传输
        BusTransferResult result = _i2c_bus->transfer(cmd);
        if (result.success_ && result.readData_.size() >= 1) {
            value = result.readData_[0];
            return true;
        }
    }
    
    return false;
}

// 实现读多字节寄存器函数
bool I2cDriver::readRegisters(uint16_t deviceAddr, uint8_t regAddr, uint8_t* data, size_t length) {
    // 创建I2C命令
    I2CBusCommand cmd;
    cmd.deviceAddress = deviceAddr;
    cmd.transferType = I2CBusCommand::WRITE_READ;
    cmd.readSize = length;
    cmd.options.noStop = false;
    
    // 准备寄存器地址数据
    BufferView view = _i2c_bus->prepareRegisterData(deviceAddr, regAddr, nullptr, 0);
    if (!view.empty()) {
        cmd.writeData = std::move(view);
        
        // 执行传输
        BusTransferResult result = _i2c_bus->transfer(cmd);
        if (result.success_ && result.readData_.size() >= length) {
            memcpy(data, result.readData_.data(), length);
            return true;
        }
    }
    
    return false;
}

// 实现写寄存器函数
bool I2cDriver::writeRegister(uint16_t deviceAddr, uint8_t regAddr, uint8_t value) {
    // 使用I2c_bus提供的prepareRegisterWrite方法
    I2CBusCommand cmd = _i2c_bus->prepareRegisterWrite(deviceAddr, regAddr, value);
    
    // 执行传输
    BusTransferResult result = _i2c_bus->transfer(cmd);
    
    return result.success_;
}

// 实现写多字节寄存器函数
bool I2cDriver::writeRegisters(uint16_t deviceAddr, uint8_t regAddr, const uint8_t* data, size_t length) {
    // 创建I2C命令
    I2CBusCommand cmd;
    cmd.deviceAddress = deviceAddr;
    cmd.transferType = I2CBusCommand::WRITE_ONLY;
    
    // 准备寄存器地址和数据
    BufferView view = _i2c_bus->prepareRegisterData(deviceAddr, regAddr, data, length);
    if (!view.empty()) {
        cmd.writeData = std::move(view);
        
        // 执行传输
        BusTransferResult result = _i2c_bus->transfer(cmd);
        return result.success_;
    }
    
    return false;
}

// 实现直接读取设备函数
bool I2cDriver::readDevice(uint16_t deviceAddr, uint8_t* data, size_t length) {
    // 创建I2C命令
    I2CBusCommand cmd;
    cmd.deviceAddress = deviceAddr;
    cmd.transferType = I2CBusCommand::READ_ONLY;
    cmd.readSize = length;
    
    // 执行传输
    BusTransferResult result = _i2c_bus->transfer(cmd);
    
    if (result.success_ && result.readData_.size() >= length) {
        // 复制读取的数据
        memcpy(data, result.readData_.data(), length);
        return true;
    }
    
    return false;
}

// 实现直接写入设备函数
bool I2cDriver::writeDevice(uint16_t deviceAddr, const uint8_t* data, size_t length) {
    // 创建I2C命令
    I2CBusCommand cmd;
    cmd.deviceAddress = deviceAddr;
    cmd.transferType = I2CBusCommand::WRITE_ONLY;
    std::vector<uint8_t> data_vec(data, data + length);

    BufferView view = _i2c_bus->prepareDeviceData(data_vec, cmd);
    if (!view.empty()) {
        cmd.writeData = std::move(view);
        
        // 执行传输
        BusTransferResult result = _i2c_bus->transfer(cmd);
        return result.success_;
    }
    return false;
}

// 实现写入并读取设备函数
bool I2cDriver::writeReadDevice(uint16_t deviceAddr, const uint8_t* writeData, size_t writeLen, uint8_t* readData, size_t readLen) {
    I2CBusCommand cmd;
    cmd.deviceAddress = deviceAddr;
    cmd.transferType = I2CBusCommand::WRITE_READ;
    cmd.readSize = readLen;
    cmd.options.noStop = false; // 默认不使用noStop
    
    std::vector<uint8_t> writeData_vec(writeData, writeData + writeLen);
    BufferView view = _i2c_bus->addToWriteBuffer(writeData_vec, cmd);
    if (!view.empty()) {
        cmd.writeData = std::move(view);
        
        // 执行传输
        BusTransferResult result = _i2c_bus->transfer(cmd);
        if (result.success_ && result.readData_.size() >= readLen) {
            memcpy(readData, result.readData_.data(), readLen);
            return true;
        }
    }
    
    return false;   
}

// 实现修改寄存器位函数
bool I2cDriver::modifyRegisterBits(uint16_t deviceAddr, uint8_t regAddr, uint8_t mask, uint8_t value) {
    uint8_t currentValue = 0;
    
    // 读取当前寄存器值
    if (!readRegister(deviceAddr, regAddr, currentValue)) {
        return false;
    }
    
    // 清除mask指定的位
    currentValue &= ~mask;
    
    // 设置新的位值（确保value的相关位在mask范围内）
    currentValue |= (value & mask);
    
    // 写回修改后的值
    return writeRegister(deviceAddr, regAddr, currentValue);
}
}