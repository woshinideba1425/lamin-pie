#include "laminpie_i2c_bus.h"
#include "buffer_management.h"
#include "interface/device_types.h"
#include <iostream>
#include <sys/_stdint.h>

I2c_bus::I2c_bus(int busNumber, int sda_pin, int scl_pin, uint32_t frequency)
                : busNumber_(busNumber), sda_pin_(sda_pin), scl_pin_(scl_pin), frequency_(frequency){
}

BusTransferResult I2c_bus::transfer(const BusCommand& command){
    I2CBusCommand cmd;

        cmd.deviceAddress = command.deviceId_.i2c.address;
        if (command.writeData_.empty()) {
            cmd.transferType = I2CBusCommand::READ_ONLY;
        } else if (command.readSize_ == 0) {
            cmd.transferType = I2CBusCommand::WRITE_ONLY;
            addToWriteBuffer(command.writeData_, cmd);
        } else {
            cmd.transferType = I2CBusCommand::WRITE_READ;
            addToWriteBuffer(command.writeData_, cmd);
        }
        cmd.readSize = command.readSize_;
    // 执行传输
    {
        std::unique_lock<std::mutex> lock(access_mutex_);
        auto [success, result] = async_deque_.submitAndWait(cmd, [this](const I2CBusCommand& c){
            return executeCommand(c);
        });
        return result.second;
    }
}

BusTransferResult I2c_bus::transfer(const I2CBusCommand& command){
    std::unique_lock<std::mutex> lock(access_mutex_);
    auto [success, result] = async_deque_.submitAndWait(command, [this](const I2CBusCommand& cmd){
        return executeCommand(cmd);
    });
    return result.second;
}


BufferView I2c_bus::addToWriteBuffer(const std::vector<uint8_t>& data, I2CBusCommand& cmd) {
    if (data.empty()) return BufferView();
    
    std::lock_guard<std::mutex> lock(tx_mutex_);
    

    BufferGuard guard(tx_buffer_, data.size());
    
    if (!guard.valid()) {
        std::cout << "Failed to reserve TX buffer space (" << data.size() << " bytes)" << std::endl;
        return BufferView();
    }
    
    memcpy(guard.ptr(), data.data(), data.size());
    guard.commit();
    
    BufferView view(guard.ptr(), data.size());
    
    return view;
}

BufferView I2c_bus::addToWriteBuffer(uint8_t reg, const uint8_t* data, size_t dataSize) {
    std::lock_guard<std::mutex> lock(tx_mutex_);
    BufferGuard guard(tx_buffer_, dataSize + 1);
    if (!guard.valid()) {
        std::cout << "Failed to reserve TX buffer space (" << dataSize + 1 << " bytes)" << std::endl;
        return BufferView();
    }
    
    guard.ptr()[0] = reg;
    if (data && dataSize > 0) {
        memcpy(guard.ptr() + 1, data, dataSize);
    }
    guard.commit();
    
    return BufferView(guard.ptr(), dataSize + 1);
}

BufferView I2c_bus::addToReadBuffer(size_t size) {
    std::lock_guard<std::mutex> lock(rx_mutex_);
    if (size == 0) return BufferView();
    BufferGuard guard(rx_buffer_, size);
    if (!guard.valid()) {
        std::cout << "Failed to reserve RX buffer space (" << size << " bytes)" << std::endl;
        return BufferView();
    }
    guard.commit();
    return BufferView(guard.ptr(), size);
}

BufferView I2c_bus::prepareRegisterData(uint16_t addr, uint8_t reg, const uint8_t* data, size_t dataSize) {
    BufferView view = addToWriteBuffer(reg, data, dataSize);
    
    if (view.empty()) {
        return view; // 传递错误状态
    }
    
    return view;
}

template<size_t N>
I2CBusCommand I2c_bus::prepareRegisterWrite(uint16_t addr, uint8_t reg, const uint8_t (&values)[N]) {
    I2CBusCommand cmd;
    cmd.deviceAddress = addr;
    cmd.transferType = I2CBusCommand::WRITE_ONLY;
    
    BufferView view = prepareRegisterData(addr, reg, values, N);
    if (!view.empty()) {
        cmd.writeData = std::move(view);
    }
    
    return cmd;
}

I2CBusCommand I2c_bus::prepareRegisterWrite(uint16_t addr, uint8_t reg, uint8_t value) {
    I2CBusCommand cmd;
    cmd.deviceAddress = addr;
    cmd.transferType = I2CBusCommand::WRITE_ONLY;
    
    BufferView view = prepareRegisterData(addr, reg, &value, 1);
    if (!view.empty()) {
        cmd.writeData = std::move(view);
    }
    
    return cmd;
}

BufferView I2c_bus::prepareDeviceData(const std::vector<uint8_t>& data, I2CBusCommand& cmd) {
    BufferView view = addToWriteBuffer(data, cmd);
    if (view.empty()) {
        return view; // 传递错误状态
    }
    return view;
}

I2CBusCommand I2c_bus::prepareProbeCommand(uint16_t addr) {
    I2CBusCommand cmd;
    cmd.deviceAddress = addr;
    cmd.transferType = I2CBusCommand::PROBE_ONLY;
    return cmd;
}

BufferView I2c_bus::addToWriteBuffer(uint16_t deviceAddr, const uint8_t* data, size_t length) {
    if (length == 0) return BufferView();
    
    std::lock_guard<std::mutex> lock(tx_mutex_);
    
    BufferGuard guard(tx_buffer_, length);
    
    if (!guard.valid()) {
        std::cout << "BufferGuard failed - not enough space in buffer" << std::endl;
        return BufferView();
    }
    
    std::memcpy(guard.ptr(), data, length);
    guard.commit();
    
    
    return BufferView(guard.ptr(), length);
}

