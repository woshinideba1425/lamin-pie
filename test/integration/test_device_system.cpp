#include "test_common.h"
#include "laminpie_device_manager.h"
#include "src/device/interface/driver.h"
#include "src/device/interface/bus.h"
#include "src/device/interface/device_types.h"
#include <memory>
#include <string>

// Mock类定义
class MockDriver : public Driver {
public:
    std::string name;
    MockDriver(const std::string& driver_name) : name(driver_name) {}
    
    std::string getName() const override { return name; }
    std::string getVersion() const override { return "1.0.0"; }
    std::vector<DeviceIdentifier> getSupportedDeviceIds() const override { return {}; }
    bool probeDevice(const DeviceIdentifier& identifier) const override { return true; }
    bool setupDevice(std::shared_ptr<DeviceIdentifier> device) override { return true; }
    void releaseDevice(std::shared_ptr<DeviceIdentifier> device) override {}
};

class MockBus : public Bus {
public:
    std::string name;
    MockBus(const std::string& bus_name) : name(bus_name) {}
    
    bool initialize() override { return true; }
    std::vector<DeviceIdentifier> scanDevices() override { return {}; }
    std::string getName() const override { return name; }
    BusTransferResult transfer(const BusCommand& command) override { 
        BusTransferResult result;
        result.success_ = true;
        return result;
    }
};

class MockDeviceIdentifier : public DeviceIdentifier {
public:
    std::string name;
    MockDeviceIdentifier(const std::string& device_name) : name(device_name) {
        id = device_name;
        busType = BUS_I2C;
    }
};

class DeviceSystemTest {
private:
    laminpie::device::DeviceManager& device_manager;
    
public:
    DeviceSystemTest() : device_manager(laminpie::device::DeviceManager::getInstance()) {}
    
    TestResult test_device_registration() {
        // 测试设备注册
        auto mock_driver = std::make_shared<MockDriver>("test_driver");
        auto result = device_manager.registerDriver(std::static_pointer_cast<Driver>(mock_driver));
        
        TEST_ASSERT(result);
        
        auto retrieved_driver = device_manager.getDriver("test_driver");
        TEST_ASSERT(retrieved_driver != nullptr);
        TEST_ASSERT(retrieved_driver->getName() == "test_driver");
        
        return TestResult::kPass;
    }
    
    TestResult test_device_discovery() {
        // 测试设备发现
        auto mock_bus = std::make_shared<MockBus>("test_bus");
        device_manager.registerBus(std::static_pointer_cast<Bus>(mock_bus));
        
        // 模拟设备发现
        device_manager.scanToAddDevices();
        
        // 验证设备是否被发现
        auto device = device_manager.findDevice("mock_device_1");
        TEST_ASSERT(device != nullptr);
        
        return TestResult::kPass;
    }
    
    TestResult test_device_lifecycle() {
        // 测试设备生命周期
        auto mock_driver = std::make_shared<MockDriver>("lifecycle_driver");
        device_manager.registerDriver(std::static_pointer_cast<Driver>(mock_driver));
        
        // 测试设备添加
        auto device = std::make_shared<MockDeviceIdentifier>("lifecycle_device");
        device_manager.notifyDeviceReady(std::static_pointer_cast<DeviceIdentifier>(device));
        
        // 验证设备状态
        TEST_ASSERT(device_manager.findDevice("lifecycle_device") != nullptr);
        
        // 测试设备移除
        device_manager.notifyDeviceRemoved(std::static_pointer_cast<DeviceIdentifier>(device));
        
        // 验证设备已移除
        TEST_ASSERT(device_manager.findDevice("lifecycle_device") == nullptr);
        
        return TestResult::kPass;
    }
};