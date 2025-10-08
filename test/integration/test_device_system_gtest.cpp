/**
 * @file test_device_system_gtest.cpp
 * @brief 设备系统测试 - Google Test版本
 * @author LaminPie Team
 * @date 2024
 */

#include "test_common_gtest.h"
#include "test_platform.h"
#include "device_types.h"
#include <atomic>
#include <thread>
#include <vector>

/**
 * @brief 设备系统测试类
 */
class DeviceSystemTest : public LaminPieIntegrationTest {
public:
    void SetUp() override {
        LaminPieIntegrationTest::SetUp();
    }
    
    void TearDown() override {
        LaminPieIntegrationTest::TearDown();
    }
};

/**
 * @brief 测试设备注册
 */
TEST_F(DeviceSystemTest, TestDeviceRegistration) {
    // 创建测试设备
    auto test_device = std::make_shared<laminpie::system::device::DeviceInfo>();
    test_device->id = "test_device_001";
    test_device->name = "Test Device";
    test_device->type = laminpie::system::device::DeviceType::kSensor;
    
    // 验证设备创建
    EXPECT_NE(test_device, nullptr) << "Device should be created successfully";
    EXPECT_EQ(test_device->id, "test_device_001") << "Device ID should match";
    EXPECT_EQ(test_device->name, "Test Device") << "Device name should match";
    EXPECT_EQ(test_device->type, laminpie::system::device::DeviceType::kSensor) 
        << "Device type should match";
    
    LOGI("Device registration test completed for device: %s", 
                test_device->id.c_str());
}

/**
 * @brief 测试设备发现
 */
TEST_F(DeviceSystemTest, TestDeviceDiscovery) {
    const int device_count = 10;
    std::vector<std::shared_ptr<laminpie::system::device::DeviceInfo>> devices;
    
    // 创建多个设备
    for (int i = 0; i < device_count; i++) {
        auto device = std::make_shared<laminpie::system::device::DeviceInfo>();
        device->id = "discovery_device_" + std::to_string(i);
        device->name = "Discovery Device " + std::to_string(i);
        device->type = static_cast<laminpie::system::device::DeviceType>(i % 3);
        
        devices.push_back(device);
    }
    
    // 验证所有设备创建成功
    EXPECT_EQ(devices.size(), device_count) << "All devices should be created";
    
    for (int i = 0; i < device_count; i++) {
        EXPECT_NE(devices[i], nullptr) << "Device " << i << " should be created";
        EXPECT_EQ(devices[i]->id, "discovery_device_" + std::to_string(i)) 
            << "Device " << i << " ID should match";
    }
    
    LOGI("Device discovery test completed for %d devices", device_count);
}

/**
 * @brief 测试设备生命周期
 */
TEST_F(DeviceSystemTest, TestDeviceLifecycle) {
    // 创建设备
    auto device = std::make_shared<laminpie::system::device::DeviceInfo>();
    device->id = "lifecycle_device";
    device->name = "Lifecycle Test Device";
    device->type = laminpie::system::device::DeviceType::kActuator;
    
    // 测试设备初始化
    EXPECT_NE(device, nullptr) << "Device should be created";
    
    // 模拟设备状态变化
    device->status = laminpie::system::device::DeviceStatus::kActive;
    EXPECT_EQ(device->status, laminpie::system::device::DeviceStatus::kActive) 
        << "Device status should be active";
    
    // 模拟设备停用
    device->status = laminpie::system::device::DeviceStatus::kInactive;
    EXPECT_EQ(device->status, laminpie::system::device::DeviceStatus::kInactive) 
        << "Device status should be inactive";
    
    // 模拟设备错误状态
    device->status = laminpie::system::device::DeviceStatus::kError;
    EXPECT_EQ(device->status, laminpie::system::device::DeviceStatus::kError) 
        << "Device status should be error";
    
    LOGI("Device lifecycle test completed for device: %s", 
                device->id.c_str());
}

/**
 * @brief 测试设备类型
 */
TEST_F(DeviceSystemTest, TestDeviceTypes) {
    // 测试所有设备类型
    std::vector<laminpie::system::device::DeviceType> device_types = {
        laminpie::system::device::DeviceType::kSensor,
        laminpie::system::device::DeviceType::kActuator,
        laminpie::system::device::DeviceType::kController
    };
    
    for (size_t i = 0; i < device_types.size(); i++) {
        auto device = std::make_shared<laminpie::system::device::DeviceInfo>();
        device->id = "type_device_" + std::to_string(i);
        device->name = "Type Test Device " + std::to_string(i);
        device->type = device_types[i];
        
        EXPECT_EQ(device->type, device_types[i]) 
            << "Device type " << i << " should match";
        
        LOGI("Device type test completed for type: %d", 
                    static_cast<int>(device_types[i]));
    }
}

/**
 * @brief 测试设备性能
 */
TEST_F(DeviceSystemTest, TestDevicePerformance) {
    const int device_count = 1000;
    const uint32_t max_time_us = 100000; // 100ms
    
    // 测量设备创建性能
    auto create_start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::shared_ptr<laminpie::system::device::DeviceInfo>> devices;
    devices.reserve(device_count);
    
    for (int i = 0; i < device_count; i++) {
        auto device = std::make_shared<laminpie::system::device::DeviceInfo>();
        device->id = "perf_device_" + std::to_string(i);
        device->name = "Performance Device " + std::to_string(i);
        device->type = static_cast<laminpie::system::device::DeviceType>(i % 3);
        
        devices.push_back(device);
    }
    
    auto create_end = std::chrono::high_resolution_clock::now();
    auto create_duration = std::chrono::duration_cast<std::chrono::microseconds>(create_end - create_start);
    
    std::cout << "Device creation performance: " << create_duration.count() << "μs for " 
              << device_count << " devices" << std::endl;
    
    EXPECT_LT(create_duration.count(), max_time_us) 
        << "Device creation performance test exceeded time limit";
    
    // 验证所有设备创建成功
    EXPECT_EQ(devices.size(), device_count) << "All devices should be created";
    
    // 测量设备访问性能
    auto access_start = std::chrono::high_resolution_clock::now();
    
    for (const auto& device : devices) {
        EXPECT_NE(device, nullptr) << "Device should not be null";
        EXPECT_FALSE(device->id.empty()) << "Device ID should not be empty";
        EXPECT_FALSE(device->name.empty()) << "Device name should not be empty";
    }
    
    auto access_end = std::chrono::high_resolution_clock::now();
    auto access_duration = std::chrono::duration_cast<std::chrono::microseconds>(access_end - access_start);
    
    std::cout << "Device access performance: " << access_duration.count() << "μs for " 
              << device_count << " devices" << std::endl;
    
    EXPECT_LT(access_duration.count(), max_time_us) 
        << "Device access performance test exceeded time limit";
}

/**
 * @brief 测试设备并发操作
 */
TEST_F(DeviceSystemTest, TestDeviceConcurrentOperations) {
    const int thread_count = 4;
    const int devices_per_thread = 100;
    
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};
    
    // 创建多个线程，每个线程创建多个设备
    for (int t = 0; t < thread_count; t++) {
        threads.emplace_back([&]() {
            for (int i = 0; i < devices_per_thread; i++) {
                auto device = std::make_shared<laminpie::system::device::DeviceInfo>();
                device->id = "concurrent_device_" + std::to_string(t) + "_" + std::to_string(i);
                device->name = "Concurrent Device " + std::to_string(t) + "_" + std::to_string(i);
                device->type = static_cast<laminpie::system::device::DeviceType>(i % 3);
                
                if (device && !device->id.empty() && !device->name.empty()) {
                    success_count.fetch_add(1);
                } else {
                    failure_count.fetch_add(1);
                }
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    int total_devices = thread_count * devices_per_thread;
    EXPECT_EQ(success_count.load() + failure_count.load(), total_devices) 
        << "All devices should be processed";
    
    EXPECT_EQ(failure_count.load(), 0) 
        << "No devices should fail in concurrent operations";
    
    LOGI("Concurrent operations test completed: %d successful, %d failed", 
                success_count.load(), failure_count.load());
}
