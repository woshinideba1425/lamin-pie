#ifndef ESP_I2C_BUS_H
#define ESP_I2C_BUS_H

#include "laminpie_i2c_bus.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include <map>
#include "laminpie_event_dispatcher.hpp"

namespace laminpie::device::bsp::esp32_s3 {
using DeviceEventType = system::event::Laminpie_Device_Event_Type;
using EventDispatcher = system::event::LaminPie_EventDispatcher;
using Event = system::event::Event<DeviceEventType>;

struct I2cDevDeleter {
    void operator()(i2c_master_dev_handle_t handle) const {
        if (handle) {
            i2c_master_bus_rm_device(handle);  // 释放资源
        }
    }
};

struct I2cBusDeleter {
    void operator()(i2c_master_bus_handle_t handle) const {
        if (handle) {
            i2c_del_master_bus(handle); // 释放资源
        }
    }
};

class EspI2cBus : public I2c_bus {
public:
    EspI2cBus(i2c_port_num_t busNumber, gpio_num_t sda_pin, gpio_num_t scl_pin, uint32_t frequency);
    ~EspI2cBus();

    bool initialize() override;
    std::vector<DeviceIdentifier> scanDevices() override;
    std::string getName() const override;

    bool write(uint16_t deviceAddr, const uint8_t* data, size_t length) override;
    bool read(uint16_t deviceAddr, uint8_t* data, size_t length) override;
    bool writeRead(uint16_t deviceAddr, const uint8_t* writeData, size_t writeLen, uint8_t* readData, size_t readLen) override;
    esp_err_t write_i2c(uint16_t deviceAddr, const uint8_t* data, size_t length);
    esp_err_t read_i2c(uint16_t deviceAddr, uint8_t* data, size_t length);
    esp_err_t writeRead_i2c(uint16_t deviceAddr, const uint8_t* writeData, size_t writeLen, uint8_t* readData, size_t readLen);
    BusTransferResult executeCommand(const I2CBusCommand& command) override;

private:
    const char* TAG = "EspI2cBus";
    i2c_port_num_t busNumber_;
    gpio_num_t sda_pin_;
    gpio_num_t scl_pin_;
    uint32_t frequency_;
    std::map<uint16_t, std::unique_ptr<i2c_master_dev_t , I2cDevDeleter>> dev_handle_map_;
    std::unique_ptr<i2c_master_bus_t, I2cBusDeleter> bus_handle_;
    std::vector<uint8_t> device_addr_list_;
    EventDispatcher& _eventDispatcher = EventDispatcher::getInstance();
};
}
#endif