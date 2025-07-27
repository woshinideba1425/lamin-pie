#pragma once

#include <I2c_driver.h>

// SHT30相关常量定义
#define SHT30_ADDR 0x44
#define TEMPERATURE_CALIBRATION 175
#define HUMIDITY_CALIBRATION 100
#define MAX_SENSOR_VALUE 65535.0
#define CELSIUS_TO_FAHRENHEIT_MULTIPLIER 1.8
#define CELSIUS_TO_FAHRENHEIT_OFFSET 32
#define TEMPERATURE_OFFSET 45

class Sht30Driver : public I2cDriver {
public:
    Sht30Driver(std::shared_ptr<Bus> bus);
    ~Sht30Driver();
    std::string getName() const override;
    std::string getVersion() const override;
    std::vector<DeviceIdentifier> getSupportedDeviceIds() const override;
    bool probeDevice(const DeviceIdentifier& identifier) const override;
    bool setupDevice(std::shared_ptr<DeviceIdentifier> device) override;
    void releaseDevice(std::shared_ptr<DeviceIdentifier> device) override;
    bool suspend(std::shared_ptr<DeviceIdentifier> device) override;
    bool resume(std::shared_ptr<DeviceIdentifier> device) override;

    // 传感器数据获取方法
    float getCtemp() const { return ctemp; }
    float getFtemp() const { return ftemp; }
    float getHumidity() const { return humidity; }
    
    // 更新传感器数据
    bool updateSensorData();
    
private:
    std::vector<DeviceIdentifier> _supportedDeviceIds;
    std::vector<uint16_t> _supportedDeviceAddress;

    // 传感器数据
    uint8_t sensorData[6];
    float ctemp, ftemp, humidity;

    // SHT30操作方法
    bool writeSHT30Command(uint16_t command);
    bool readSHT30Data();
    bool resetSHT30();
    
    // CRC校验方法
    uint8_t calculateCRC8(const uint8_t* data, size_t len) const;
};