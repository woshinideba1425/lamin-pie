/**
 * @file test_memory_performance_gtest.cpp
 * @brief 内存性能测试 - Google Test版本
 * @author LaminPie Team
 * @date 2024
 */

#include "test_common_gtest.h"
#include <vector>
#include <memory>
#include <random>

/**
 * @brief 内存性能测试类
 */
class MemoryPerformanceTest : public LaminPiePerformanceTest {
public:
    void SetUp() override {
        LaminPiePerformanceTest::SetUp();
    }
    
    void TearDown() override {
        LaminPiePerformanceTest::TearDown();
    }
};

/**
 * @brief 测试内存分配速度
 */
TEST_F(MemoryPerformanceTest, TestMemoryAllocationSpeed) {
    const int allocation_count = 10000;
    const uint32_t max_time_us = 100000; // 100ms
    
    // 测试小对象分配性能
    auto small_alloc_duration = MeasureExecutionTime([allocation_count]() {
        std::vector<std::unique_ptr<int>> allocations;
        allocations.reserve(allocation_count);
        
        for (int i = 0; i < allocation_count; i++) {
            allocations.emplace_back(std::make_unique<int>(i));
        }
    });
    
    std::cout << "Small object allocation: " << small_alloc_duration << "μs for " 
              << allocation_count << " allocations" << std::endl;
    
    EXPECT_LT(small_alloc_duration, max_time_us) 
        << "Small object allocation performance test exceeded time limit";
    
    // 测试大对象分配性能
    auto large_alloc_duration = MeasureExecutionTime([allocation_count]() {
        std::vector<std::unique_ptr<std::array<char, 1024>>> allocations;
        allocations.reserve(allocation_count);
        
        for (int i = 0; i < allocation_count; i++) {
            allocations.emplace_back(std::make_unique<std::array<char, 1024>>());
        }
    });
    
    std::cout << "Large object allocation: " << large_alloc_duration << "μs for " 
              << allocation_count << " allocations" << std::endl;
    
    EXPECT_LT(large_alloc_duration, max_time_us * 10) 
        << "Large object allocation performance test exceeded time limit";
}

/**
 * @brief 测试内存碎片化
 */
TEST_F(MemoryPerformanceTest, TestMemoryFragmentation) {
    const int cycles = 1000;
    const int max_objects = 100;
    
    std::vector<std::unique_ptr<std::vector<int>>> objects;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> size_dist(1, 1000);
    std::uniform_int_distribution<> action_dist(0, 2);
    
    // 模拟内存分配和释放，测试碎片化
    for (int cycle = 0; cycle < cycles; cycle++) {
        // 随机分配对象
        if (objects.size() < max_objects && action_dist(gen) < 2) {
            int size = size_dist(gen);
            auto obj = std::make_unique<std::vector<int>>();
            obj->reserve(size);
            
            // 填充一些数据
            for (int i = 0; i < size; i++) {
                obj->push_back(i);
            }
            
            objects.push_back(std::move(obj));
        }
        
        // 随机释放对象
        if (!objects.empty() && action_dist(gen) == 0) {
            objects.erase(objects.begin() + (gen() % objects.size()));
        }
    }
    
    // 验证内存操作正常完成
    EXPECT_LE(objects.size(), max_objects) << "Object count should not exceed maximum";
    
    LOGI( "Memory fragmentation test completed with %zu objects", objects.size());
}

/**
 * @brief 测试内存复制性能
 */
TEST_F(MemoryPerformanceTest, TestMemoryCopyPerformance) {
    const int data_size = 1024 * 1024; // 1MB
    const uint32_t max_time_us = 50000; // 50ms
    
    std::vector<char> source_data(data_size);
    std::vector<char> dest_data(data_size);
    
    // 填充源数据
    for (int i = 0; i < data_size; i++) {
        source_data[i] = static_cast<char>(i % 256);
    }
    
    // 测试内存复制性能
    auto copy_duration = MeasureExecutionTime([&]() {
        std::copy(source_data.begin(), source_data.end(), dest_data.begin());
    });
    
    std::cout << "Memory copy performance: " << copy_duration << "μs for " 
              << data_size << " bytes" << std::endl;
    
    EXPECT_LT(copy_duration, max_time_us) 
        << "Memory copy performance test exceeded time limit";
    
    // 验证数据完整性
    EXPECT_EQ(source_data, dest_data) << "Copied data should match source data";
}

/**
 * @brief 测试内存移动性能
 */
TEST_F(MemoryPerformanceTest, TestMemoryMovePerformance) {
    const int data_size = 1024 * 1024; // 1MB
    const uint32_t max_time_us = 10000; // 10ms
    
    std::vector<char> source_data(data_size);
    
    // 填充源数据
    for (int i = 0; i < data_size; i++) {
        source_data[i] = static_cast<char>(i % 256);
    }
    
    // 测试内存移动性能
    auto move_duration = MeasureExecutionTime([&]() {
        std::vector<char> dest_data = std::move(source_data);
        source_data = std::move(dest_data);
    });
    
    std::cout << "Memory move performance: " << move_duration << "μs for " 
              << data_size << " bytes" << std::endl;
    
    EXPECT_LT(move_duration, max_time_us) 
        << "Memory move performance test exceeded time limit";
}

/**
 * @brief 测试内存对齐性能
 */
TEST_F(MemoryPerformanceTest, TestMemoryAlignmentPerformance) {
    const int allocation_count = 10000;
    const uint32_t max_time_us = 50000; // 50ms
    
    // 测试不同对齐要求的内存分配
    auto aligned_alloc_duration = MeasureExecutionTime([allocation_count]() {
        std::vector<std::aligned_storage<64, 64>::type> aligned_objects;
        aligned_objects.reserve(allocation_count);
        
        for (int i = 0; i < allocation_count; i++) {
            aligned_objects.emplace_back();
        }
    });
    
    std::cout << "Aligned memory allocation: " << aligned_alloc_duration << "μs for " 
              << allocation_count << " allocations" << std::endl;
    
    EXPECT_LT(aligned_alloc_duration, max_time_us) 
        << "Aligned memory allocation performance test exceeded time limit";
}
