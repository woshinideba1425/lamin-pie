#include "laminpie_sht30_driver.h"
#include "laminpie_device_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <sys/_stdint.h>

static const char* TAG = "SHT30";
namespace laminpie::device::drivers::i2c{
// CRC8校验多项式: x^8 + x^5 + x^4 + 1 = 0x31
#define CRC8_POLYNOMIAL 0x31

Sht30Driver::Sht30Driver(std::shared_ptr<Bus> bus) : I2cDriver(bus), ctemp(0), ftemp(0), humidity(0) {
    // 初始化支持的设备ID - 不使用resize，直接初始化一个元素的向量
    uint16_t sht30_addr = SHT30_ADDR;
    DeviceIdentifier sht30Device(DeviceIdentifier::BUS_I2C, std::to_string(sht30_addr));
    sht30Device.i2c.address = sht30_addr;
    sht30Device.version = "1.0.0";
    _supportedDeviceIds.push_back(sht30Device);
    
    // 初始化支持的设备地址
    _supportedDeviceAddress.push_back(SHT30_ADDR);
    
    // 初始化传感器数据缓冲区
    memset(sensorData, 0, sizeof(sensorData));
}

Sht30Driver::~Sht30Driver() {
    _supportedDeviceIds.clear();
    _supportedDeviceAddress.clear();
}

std::string Sht30Driver::getName() const {
    return _supportedDeviceIds[0].id;
}

std::string Sht30Driver::getVersion() const {
    return _supportedDeviceIds[0].version;
}

std::vector<DeviceIdentifier> Sht30Driver::getSupportedDeviceIds() const {
    return _supportedDeviceIds;
}

bool Sht30Driver::probeDevice(const DeviceIdentifier& identifier) const {
    uint16_t deviceAddress = identifier.getI2cAddress();
    // 检查设备是否支持
    if (!identifier.isI2cDevice()) {
        ESP_LOGI(TAG, "Device is not an I2c device");
        return false;
    }
    // 检查设备地址是否有效
    if (identifier.getI2cAddress() == 0 || identifier.getI2cAddress() > 127) {
        ESP_LOGI(TAG, "Device address is invalid");
        return false;
    }
    ESP_LOGI(TAG, "deviceAddress: 0x%02x", deviceAddress);
    if (std::find(_supportedDeviceAddress.begin(), _supportedDeviceAddress.end(), deviceAddress) != _supportedDeviceAddress.end()) {
        I2CBusCommand cmd = getI2cBus()->prepareProbeCommand(deviceAddress);
        BusTransferResult result = getI2cBus()->transfer(cmd);
        if (result.success_) {
            return true;
        }
    }else{
        ESP_LOGI(TAG, "deviceAddress not in supportedDeviceAddress");
    }
    return false;
}

bool Sht30Driver::setupDevice(std::shared_ptr<DeviceIdentifier> device) {
    // 发送测量命令
    if (!writeSHT30Command(0x2C06)) {
        ESP_LOGE(TAG, "Failed to send measurement command");
        return false;
    }
    
    // 等待传感器准备数据
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // 读取初始数据
    if (!updateSensorData()) {
        ESP_LOGE(TAG, "SHT30 initialization failed");
        return false;
    }
    
    ESP_LOGI(TAG, "SHT30 initialized successfully");
    return true;
}

void Sht30Driver::releaseDevice(std::shared_ptr<DeviceIdentifier> device) {
    // 没有特殊的释放操作
    ESP_LOGI(TAG, "SHT30 released");
}

bool Sht30Driver::suspend(std::shared_ptr<DeviceIdentifier> device) {
    // SHT30没有特殊的休眠命令，可以考虑停止周期性测量
    ESP_LOGI(TAG, "SHT30 suspended");
    return true;
}

bool Sht30Driver::resume(std::shared_ptr<DeviceIdentifier> device) {
    // 恢复时重新发送测量命令并更新数据
    if (!updateSensorData()) {
        ESP_LOGE(TAG, "Failed to resume SHT30");
        return false;
    }
    
    ESP_LOGI(TAG, "SHT30 resumed");
    return true;
}

bool Sht30Driver::updateSensorData() {
    // 发送测量命令
    if (!writeSHT30Command(0x2C06)) {
        ESP_LOGE(TAG, "Failed to send measurement command");
        return false;
    }
    
    // 等待传感器准备数据 (至少15ms，根据数据手册)
    vTaskDelay(pdMS_TO_TICKS(20));
    
    // 读取传感器数据
    if (!readSHT30Data()) {
        ESP_LOGE(TAG, "Failed to read sensor data");
        return false;
    }
    
    // 计算温度和湿度值
    ctemp = ((((sensorData[0] * 256.0) + sensorData[1]) * TEMPERATURE_CALIBRATION) / MAX_SENSOR_VALUE) - TEMPERATURE_OFFSET;
    ftemp = (ctemp * CELSIUS_TO_FAHRENHEIT_MULTIPLIER) + CELSIUS_TO_FAHRENHEIT_OFFSET;
    humidity = ((((sensorData[3] * 256.0) + sensorData[4]) * HUMIDITY_CALIBRATION) / MAX_SENSOR_VALUE);
    
    return true;
}

bool Sht30Driver::writeSHT30Command(uint16_t command) {
    uint8_t cmd_bytes[2] = {
        static_cast<uint8_t>(command >> 8),    // 高字节
        static_cast<uint8_t>(command & 0xFF)   // 低字节
    };
    
    return writeDevice(SHT30_ADDR, cmd_bytes, 2);
}

bool Sht30Driver::readSHT30Data() {
    // 读取6字节数据 (温度高字节、温度低字节、温度CRC、湿度高字节、湿度低字节、湿度CRC)
    if (!readDevice(SHT30_ADDR, sensorData, 6)) {
        ESP_LOGE(TAG, "Failed to read from SHT30");
        return false;
    }
    
    // 验证温度数据的CRC
    uint8_t temp_crc = calculateCRC8(sensorData, 2);
    if (temp_crc != sensorData[2]) {
        ESP_LOGE(TAG, "Temperature CRC check failed: calculated 0x%02X, received 0x%02X", 
                 temp_crc, sensorData[2]);
        return false;
    }
    
    // 验证湿度数据的CRC
    uint8_t hum_crc = calculateCRC8(sensorData + 3, 2);
    if (hum_crc != sensorData[5]) {
        ESP_LOGE(TAG, "Humidity CRC check failed: calculated 0x%02X, received 0x%02X", 
                 hum_crc, sensorData[5]);
        return false;
    }
    
    return true;
}

bool Sht30Driver::resetSHT30() {
    // 发送软复位命令 (0x30A2)
    if (!writeSHT30Command(0x30A2)) {
        ESP_LOGE(TAG, "Failed to send reset command");
        return false;
    }
    
    // 等待传感器复位完成
    vTaskDelay(pdMS_TO_TICKS(10));
    
    ESP_LOGI(TAG, "SHT30 reset successful");
    return true;
}

uint8_t Sht30Driver::calculateCRC8(const uint8_t* data, size_t len) const {
    uint8_t crc = 0xFF; // 初始值
    
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i]; // 异或当前字节
        
        // 处理8位
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            } else {
                crc = (crc << 1);
            }
        }
    }
    
    return crc;
}
}