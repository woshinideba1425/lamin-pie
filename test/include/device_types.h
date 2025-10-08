/**
 * @file device_types.h
 * @brief Device system type definitions
 * @author LaminPie Team
 * @date 2024
 */

#ifndef DEVICE_TYPES_H
#define DEVICE_TYPES_H

#include <cstdint>
#include <string>
#include <memory>

namespace laminpie::system::device {

/**
 * @brief Device types enumeration
 */
enum class DeviceType : uint8_t {
    kSensor = 0,
    kActuator = 1,
    kController = 2
};

/**
 * @brief Device status enumeration
 */
enum class DeviceStatus : uint8_t {
    kInactive = 0,
    kActive = 1,
    kError = 2
};

/**
 * @brief Device information structure
 */
struct DeviceInfo {
    std::string id;
    std::string name;
    DeviceType type;
    DeviceStatus status = DeviceStatus::kInactive;
    
    DeviceInfo() = default;
    DeviceInfo(const std::string& device_id, const std::string& device_name, DeviceType device_type)
        : id(device_id), name(device_name), type(device_type) {}
};

} // namespace laminpie::system::device

#endif // DEVICE_TYPES_H
