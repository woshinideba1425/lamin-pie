// error.h - 基础错误类型定义
#pragma once
#include <string>
#include <memory>
#include <functional>
#include <system_error>
#if ESP_PLATFORM
#include "esp_err.h"
#endif

namespace laminate {

// 基础错误接口
class Error {
public:
    virtual ~Error() = default;
    virtual std::string message() const = 0;
    virtual std::string type_name() const = 0;
    virtual int code() const = 0;
    
    // 可选：错误上下文信息
    virtual std::string context() const { return ""; }
    
    // 错误类型检查
    template <typename T>
    bool is() const {
        return dynamic_cast<const T*>(this) != nullptr;
    }
    
    // 错误类型转换
    template <typename T>
    const T* as() const {
        return dynamic_cast<const T*>(this);
    }
};

// 标准错误实现
class StandardError : public Error {
public:
    StandardError(std::string message, int code = 0) 
        : message_(std::move(message)), code_(code) {}
    
    std::string message() const override { return message_; }
    std::string type_name() const override { return "StandardError"; }
    int code() const override { return code_; }

private:
    std::string message_;
    int code_;
};

#if ESP_PLATFORM
// 平台特定错误 - ESP-IDF
class EspError : public Error {
public:
    EspError(esp_err_t err) : err_(err) {}
    
    std::string message() const override { 
        return std::string(esp_err_to_name(err_)) + ": " + 
               std::string(esp_err_to_name_r(err_, err_buf_, sizeof(err_buf_))); 
    }
    
    std::string type_name() const override { return "EspError"; }
    int code() const override { return static_cast<int>(err_); }
    esp_err_t esp_code() const { return err_; }

private:
    esp_err_t err_;
    mutable char err_buf_[64];
};

#endif

// 总线错误
class BusError : public StandardError {
public:
    enum Code {
        DEVICE_NOT_FOUND = 1,
        COMMUNICATION_FAILED = 2,
        TIMEOUT = 3,
        // ...其他总线错误码
    };
    
    BusError(std::string message, Code code) 
        : StandardError(std::move(message), static_cast<int>(code)), bus_code_(code) {}
    
    std::string type_name() const override { return "BusError"; }
    Code bus_code() const { return bus_code_; }

private:
    Code bus_code_;
};

#if CONFIG_LAMINPIE_ENABLE_I2C_BUS && ESP_PLATFORM
// 为特定总线类型扩展错误
class I2cBusError : public BusError {
public:
    enum I2cErrorCode {
        ADDRESS_NACK = 100,
        DATA_NACK = 101,
        ARBITRATION_LOST = 102,
        TIMEOUT = 103,
        GENERIC = 104,
    };
    
    I2cBusError(std::string message, I2cErrorCode code)
        : BusError(std::move(message), BusError::COMMUNICATION_FAILED), i2c_code_(code) {}
    
    std::string type_name() const override { return "I2cBusError"; }
    I2cErrorCode i2c_code() const { return i2c_code_; }
    
    // 添加ESP32错误码转换
    static I2cBusError from_esp_error(esp_err_t esp_err) {
        switch (esp_err) {
            case ESP_ERR_TIMEOUT:
                return I2cBusError("I2C timeout", I2cErrorCode::TIMEOUT);
            case ESP_FAIL:
                return I2cBusError("I2C operation failed", I2cErrorCode::GENERIC);
            // ...其他映射
            default:
                return I2cBusError("Unknown I2C error: " + std::to_string(esp_err), I2cErrorCode::GENERIC);
        }
    }

private:
    I2cErrorCode i2c_code_;
};
#endif
}  // namespace laminate