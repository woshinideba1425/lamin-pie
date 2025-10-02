#include "laminpie_esp_i2c_bus.h"
#include "interface/device_types.h"
#include "esp_log.h"
#include <cstdint>
#include <sys/_stdint.h>
#include <inttypes.h>
#include "driver/i2c.h"
namespace laminpie::device::bsp::esp {
EspI2cBus::EspI2cBus(i2c_port_num_t busNumber, gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t frequency)
    : I2c_bus(busNumber, sda_pin, scl_pin, frequency),
    busNumber_(busNumber),
    sda_pin_(sda_pin),
    scl_pin_(scl_pin),
    frequency_(frequency){
    bus_handle_ = std::unique_ptr<i2c_master_bus_t, I2cBusDeleter>(nullptr);
    dev_handle_map_.clear();
    device_addr_list_.clear();
    device_addr_list_.reserve(10);
}

EspI2cBus::~EspI2cBus() {
    bus_handle_.reset();
}

bool EspI2cBus::initialize() {
    LOGI(TAG, "Initializing I2C on SDA: %d, SCL: %d with frequency: %lu Hz", sda_pin_, scl_pin_, (unsigned long)frequency_);

    esp_err_t ret;
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = GPIO_NUM_11,
            .scl_io_num = GPIO_NUM_10,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };

    i2c_master_bus_handle_t raw_bus_handle = nullptr;
    ret = i2c_new_master_bus(&i2c_bus_cfg, &raw_bus_handle);
    if(ret != ESP_OK){
        LOGE(TAG, "Failed to initialize I2C bus: %d", ret);
        return false;
    }
    bus_handle_.reset(raw_bus_handle);

    return true;
}

std::vector<DeviceIdentifier> EspI2cBus::scanDevices() {
    std::vector<uint16_t> current_devices;
    std::vector<DeviceIdentifier> device_list;
    
    // 1. 先检查已知设备是否仍然存在
    for (auto addr_it = device_addr_list_.begin(); addr_it != device_addr_list_.end();) {
        uint16_t addr = *addr_it;
        esp_err_t ret = i2c_master_probe(bus_handle_.get(), addr, 20); // 缩短已知设备超时
        
        if (ret == ESP_OK) {
            // 设备仍然存在
            current_devices.push_back(addr);
            ++addr_it;
        } else {
            // 设备已移除，从设备句柄映射中删除
            LOGI(TAG, "Device removed from address: 0x%02X", addr);
            dev_handle_map_.erase(addr);
            addr_it = device_addr_list_.erase(addr_it);
        }
    }
    
    // 2. 扫描可能的新设备(跳过保留地址和已知设备)
    static const uint8_t common_addresses[] = {
        0x1C, 0x1D, 0x1E, // 磁力计
        0x3C, 0x3D,       // OLED显示器
        0x38, 0x39,       // 多功能传感器
        0x40, 0x41, 0x44,       // 湿度传感器
        0x48, 0x49, 0x4A, 0x4B, // 温度传感器
        0x50, 0x51, 0x52, 0x53, // EEPROM
        0x68, 0x69,       // RTC, IMU
        0x76, 0x77        // 气压传感器
    };
    
    // 先扫描常见地址
    for (uint8_t addr : common_addresses) {
        // 跳过已知设备
        if (std::find(current_devices.begin(), current_devices.end(), addr) != current_devices.end()) {
            continue;
        }
        
        esp_err_t ret = i2c_master_probe(bus_handle_.get(), addr, 100);
        if (ret == ESP_OK) {
            LOGI(TAG, "Found new device at common address: 0x%02X", addr);
            current_devices.push_back(addr);
            device_addr_list_.push_back(addr);
        }
    }
    
    // 每隔N次扫描，执行一次完整扫描(可以通过计数器控制)
    static int full_scan_counter = 0;
    if (++full_scan_counter >= 10) {  // 每10次周期扫描做一次完整扫描
        full_scan_counter = 0;
        
        // 扫描剩余地址(跳过保留地址和已检测地址)
        for (uint16_t addr = 0x08; addr < 0x78; addr++) {
            // 跳过已知设备和常见设备列表
            if (std::find(current_devices.begin(), current_devices.end(), addr) != current_devices.end()) {
                continue;
            }
            
            esp_err_t ret = i2c_master_probe(bus_handle_.get(), addr, 100);
            if (ret == ESP_OK) {
                LOGI(TAG, "Found new device at address: 0x%02X", addr);
                current_devices.push_back(addr);
                device_addr_list_.push_back(addr);
            }
        }
    }
    
    // 3. 创建设备对象列表
    for (uint16_t addr : current_devices) {
        // 检查设备是否已经有handle
        if (dev_handle_map_.find(addr) == dev_handle_map_.end()) {
            DeviceIdentifier id(DeviceIdentifier::BUS_I2C, std::to_string(addr));
            id.i2c.address = addr;
            i2c_device_config_t dev_cfg = {
                .dev_addr_length = I2C_ADDR_BIT_LEN_7,
                .device_address = addr,
                .scl_speed_hz = frequency_,
            };
            
            i2c_master_dev_handle_t raw_dev_handle = nullptr;
            esp_err_t ret = i2c_master_bus_add_device(bus_handle_.get(), &dev_cfg, &raw_dev_handle);
            if (ret != ESP_OK) {
                LOGE(TAG, "Failed to add device to bus: %d", ret);
                continue;
            }
            
            dev_handle_map_[addr] = std::unique_ptr<i2c_master_dev_t, I2cDevDeleter>(raw_dev_handle);
            device_list.push_back(id);
        } else {
            // 设备已经有handle，只需创建DeviceIdentifier
            DeviceIdentifier id(DeviceIdentifier::BUS_I2C, std::to_string(addr));
            id.i2c.address = addr;
            device_list.push_back(id);
        }
    }
    
    // 创建一个空的设备标识符作为占位符
    auto emptyDevice = std::make_shared<DeviceIdentifier>();
    auto event = std::make_shared<laminpie::system::event::DeviceEvent>(
        DeviceEventType::kBusScanComplete, emptyDevice);
    _eventDispatcher.dispatchEvent(*event);
    
    return device_list;
}

std::string EspI2cBus::getName() const {
    return TAG;
}

esp_err_t EspI2cBus::write_i2c(uint16_t deviceAddr, const uint8_t* data, size_t length) {
    auto it = dev_handle_map_.find(deviceAddr);
    if(it == dev_handle_map_.end()){
        LOGE(TAG, "Device handle not found for address: 0x%02X", deviceAddr);
        return ESP_ERR_NOT_FOUND;
    }
    
    BufferView view = addToWriteBuffer(deviceAddr, data, length);
    if(view.empty()){
        LOGE(TAG, "Failed to add data to write buffer");
        return ESP_ERR_NO_MEM;
    }
    
    esp_err_t ret = i2c_master_transmit(it->second.get(), view.data(), view.size(), 100);
    if (ret != ESP_OK) {
        LOGE(TAG, "i2c_master_transmit failed: %s (0x%x)", esp_err_to_name(ret), ret);
    }
    
    return ret;
}

esp_err_t EspI2cBus::read_i2c(uint16_t deviceAddr, uint8_t* data, size_t length) {
    auto it = dev_handle_map_.find(deviceAddr);
    if(it == dev_handle_map_.end()){
        return ESP_ERR_NOT_FOUND;
    }
    
    // 创建带所有权的 BufferView
    BufferView view = addToReadBuffer(length);
    if(view.empty()){
        return ESP_ERR_NO_MEM;
    }
    
    esp_err_t ret = i2c_master_receive(it->second.get(), view.data(), view.size(), 100);
    if(ret == ESP_OK && data != nullptr) {
        // 复制数据到用户提供的缓冲区
        std::memcpy(data, view.data(), view.size());
    }
    
    return ret;
}

esp_err_t EspI2cBus::writeRead_i2c(uint16_t deviceAddr, const uint8_t* writeData, size_t writeLen, uint8_t* readData, size_t readLen) {
    auto it = dev_handle_map_.find(deviceAddr);
    if(it == dev_handle_map_.end()){
        return ESP_ERR_NOT_FOUND;
    }
    
    BufferView view = addToWriteBuffer(deviceAddr, writeData, writeLen);
    if(view.empty()){
        return ESP_ERR_NO_MEM;
    }
    
    return i2c_master_transmit_receive(it->second.get(), view.data(), view.size(), readData, readLen, 100);
}

bool EspI2cBus::write(uint16_t deviceAddr, const uint8_t* data, size_t length) {
    esp_err_t ret = write_i2c(deviceAddr, data, length);
    if(ret != ESP_OK){
        LOGE(TAG, "Failed to write data to device: %s (%d)", esp_err_to_name(ret), ret);
        return false;
    }
    return true;
}

bool EspI2cBus::read(uint16_t deviceAddr, uint8_t* data, size_t length) {
    esp_err_t ret = read_i2c(deviceAddr, data, length);
    if(ret != ESP_OK){
        LOGE(TAG, "Failed to read data from device: %s (%d)", esp_err_to_name(ret), ret);
        return false;
    }
    return true;
}

bool EspI2cBus::writeRead(uint16_t deviceAddr, const uint8_t* writeData, size_t writeLen, uint8_t* readData, size_t readLen) {
    esp_err_t ret = writeRead_i2c(deviceAddr, writeData, writeLen, readData, readLen);
    if(ret != ESP_OK){
        LOGE(TAG, "Failed to write and read data from device: %s (%d)", esp_err_to_name(ret), ret);
        return false;
    }
    return true;
}

BusTransferResult EspI2cBus::executeCommand(const I2CBusCommand& command) {
    esp_err_t ret = ESP_OK;
    std::vector<uint8_t> readData;
    
    switch(command.transferType) {
        case I2CBusCommand::TransferType::WRITE_ONLY:
            ret = write_i2c(command.deviceAddress, command.writeData.data(), command.writeData.size());
            break;
            
        case I2CBusCommand::TransferType::READ_ONLY: {
            readData.resize(command.readSize);
            ret = read_i2c(command.deviceAddress, readData.data(), command.readSize);
            break;
        }
        
        case I2CBusCommand::TransferType::WRITE_READ: {
            if(!command.options.noStop){
                readData.resize(command.readSize);
                ret = writeRead_i2c(command.deviceAddress, command.writeData.data(), command.writeData.size(), 
                                readData.data(), command.readSize);
            }else{
                readData.resize(command.readSize);
                ret = write_i2c(command.deviceAddress, command.writeData.data(), command.writeData.size());
                if(ret == ESP_OK){
                    ret = read_i2c(command.deviceAddress, readData.data(), command.readSize);
                }
            }
            break;
        }

        case I2CBusCommand::TransferType::PROBE_ONLY: {
            ret = i2c_master_probe(bus_handle_.get(), command.deviceAddress, 100);
            if(ret == ESP_OK){
                LOGI(TAG, "Device found at address: 0x%02X", command.deviceAddress);
                DeviceIdentifier id(DeviceIdentifier::BUS_I2C, std::to_string(command.deviceAddress)); 
                id.i2c.address = command.deviceAddress; 
                i2c_device_config_t dev_cfg = {
                    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
                    .device_address = command.deviceAddress,
                    .scl_speed_hz = frequency_,
                };
                i2c_master_dev_handle_t raw_dev_handle = nullptr;
                // ret = i2c_master_bus_add_device(bus_handle_.get(), &dev_cfg, &raw_dev_handle);
                // if(ret != ESP_OK){
                //     LOGE(TAG, "Failed to add device to bus: %d", ret);
                // }
                // dev_handle_map_[command.deviceAddress] = std::unique_ptr<i2c_master_dev_t, I2cDevDeleter>(raw_dev_handle);
            }else{
                LOGE(TAG, "Device not found at address: 0x%02X", command.deviceAddress);
            }
            break;
        }
        
        default:
            return BusTransferResult(false, {}, ESP_ERR_INVALID_ARG, "Invalid transfer type");
    }
    
    if(ret == ESP_OK) {
        return BusTransferResult(true, std::move(readData), ESP_OK, "");
    } else {
        std::string errMsg = std::string("I2C operation failed: ") + esp_err_to_name(ret);
        return BusTransferResult(false, {}, ret, errMsg);
    }
}
}