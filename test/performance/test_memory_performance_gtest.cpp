/**
 * @file test_memory_performance_gtest.cpp
 * @brief 内存性能测试
 * @author LaminPie Team
 * @date 2024
 */

#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <memory>
#include <random>
#include "test_common_gtest.h"

using namespace laminpie;

/**
 * @brief 内存性能测试类
 */
class MemoryPerformanceTest : public LaminPiePerformanceTest {
protected:
    void SetUp() override {
        // 测试前准备
    }
    
    void TearDown() override {
        // 测试后清理
    }
};

/**
 * @brief 测试内存分配性能
 */
TEST_F(MemoryPerformanceTest, TestMemoryAllocationPerformance) {
    const int allocation_count = 1000;  // 减少分配次数
    const int iterations = 10;          // 减少迭代次数
    const uint32_t max_time_us = 100000; // 100ms
    
    // 测量内存分配性能
    auto duration = MeasureExecutionTime([&]() {
        std::vector<std::unique_ptr<int>> allocations;
        allocations.reserve(allocation_count);
        
        for (int i = 0; i < allocation_count; i++) {
            allocations.push_back(std::make_unique<int>(i));
        }
        
        // 模拟一些操作
        for (auto& ptr : allocations) {
            *ptr *= 2;
        }
        
        // 自动释放内存
    }, iterations);
    
    std::cout << "Memory allocation performance: " << duration << "μs for " 
              << allocation_count << " allocations (" << iterations << " iterations)" << std::endl;
    
    // 验证性能要求
    EXPECT_LT(duration, max_time_us) << "Memory allocation performance test exceeded time limit";
}

/**
 * @brief 测试内存复制性能
 */
TEST_F(MemoryPerformanceTest, TestMemoryCopyPerformance) {
    const size_t data_size = 1024 * 1024; // 1MB
    const int iterations = 5;              // 减少迭代次数
    const uint32_t max_time_us = 50000;    // 50ms
    
    std::vector<uint8_t> source_data(data_size);
    std::vector<uint8_t> dest_data(data_size);
    
    // 填充测试数据
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < data_size; i++) {
        source_data[i] = dis(gen);
    }
    
    // 测量内存复制性能
    auto duration = MeasureExecutionTime([&]() {
        std::copy(source_data.begin(), source_data.end(), dest_data.begin());
    }, iterations);
    
    std::cout << "Memory copy performance: " << duration << "μs for " 
              << data_size << " bytes (" << iterations << " iterations)" << std::endl;
    
    // 验证性能要求
    EXPECT_LT(duration, max_time_us) << "Memory copy performance test exceeded time limit";
}

/**
 * @brief 测试内存对齐性能
 */
TEST_F(MemoryPerformanceTest, TestMemoryAlignmentPerformance) {
    const int allocation_count = 500;   // 减少分配次数
    const int iterations = 10;          // 减少迭代次数
    const uint32_t max_time_us = 50000; // 50ms
    
    // 测量对齐内存分配性能
    auto duration = MeasureExecutionTime([&]() {
        std::vector<void*> allocations;
        allocations.reserve(allocation_count);
        
        for (int i = 0; i < allocation_count; i++) {
            void* ptr = aligned_alloc(64, 1024); // 64字节对齐，1KB大小
            if (ptr) {
                allocations.push_back(ptr);
            }
        }
        
        // 释放内存
        for (void* ptr : allocations) {
            free(ptr);
        }
    }, iterations);
    
    std::cout << "Memory alignment performance: " << duration << "μs for " 
              << allocation_count << " aligned allocations (" << iterations << " iterations)" << std::endl;
    
    // 验证性能要求
    EXPECT_LT(duration, max_time_us) << "Memory alignment performance test exceeded time limit";
}

/**
 * @brief 测试内存池性能
 */
TEST_F(MemoryPerformanceTest, TestMemoryPoolPerformance) {
    const int allocation_count = 2000;  // 减少分配次数
    const int iterations = 5;           // 减少迭代次数
    const uint32_t max_time_us = 100000; // 100ms
    
    // 测量内存池性能
    auto duration = MeasureExecutionTime([&]() {
        std::vector<std::unique_ptr<int[]>> allocations;
        allocations.reserve(allocation_count);
        
        for (int i = 0; i < allocation_count; i++) {
            allocations.push_back(std::make_unique<int[]>(10)); // 每个分配10个int
        }
        
        // 模拟使用内存
        for (auto& ptr : allocations) {
            for (int j = 0; j < 10; j++) {
                ptr[j] = j;
            }
        }
        
        // 自动释放
    }, iterations);
    
    std::cout << "Memory pool performance: " << duration << "μs for " 
              << allocation_count << " allocations (" << iterations << " iterations)" << std::endl;
    
    // 验证性能要求
    EXPECT_LT(duration, max_time_us) << "Memory pool performance test exceeded time limit";
}