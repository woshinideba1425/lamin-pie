#include "test_common.h"
#include "laminpie_device_manager.h"
#include "fixtures/mock_device_manager.hpp"

class DeviceSystemTest {
private:
    device::DeviceManager& device_manager;
    
public:
    DeviceSystemTest() : device_manager(device::DeviceManager::getInstance()) {}
    
    TestResult test_device_registration() {
        // 测试设备注册
        auto mock_driver = std::make_shared<MockDriver>("test_driver");
        auto result = device_manager.registerDriver(mock_driver);
        
        TEST_ASSERT(result);
        
        auto retrieved_driver = device_manager.getDriver("test_driver");
        TEST_ASSERT(retrieved_driver != nullptr);
        TEST_ASSERT(retrieved_driver->getName() == "test_driver");
        
        return TestResult::kPass;
    }
    
    TestResult test_device_discovery() {
        // 测试设备发现
        auto mock_bus = std::make_shared<MockBus>("test_bus");
        device_manager.registerBus(mock_bus);
        
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
        device_manager.registerDriver(mock_driver);
        
        // 测试设备添加
        auto device = std::make_shared<MockDeviceIdentifier>("lifecycle_device");
        device_manager.notifyDeviceReady(device);
        
        // 验证设备状态
        TEST_ASSERT(device_manager.findDevice("lifecycle_device") != nullptr);
        
        // 测试设备移除
        device_manager.notifyDeviceRemoved(device);
        
        // 验证设备已移除
        TEST_ASSERT(device_manager.findDevice("lifecycle_device") == nullptr);
        
        return TestResult::kPass;
    }
};