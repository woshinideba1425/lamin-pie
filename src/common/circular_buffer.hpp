#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <cstring>
#include <algorithm>
#include <memory>
#include <iostream>
#include <cstdlib>
#include "laminpie_log.hpp"

namespace Assist{
template<typename T, size_t Alignment = alignof(T)>
class CircularBuffer{
    public:
        CircularBuffer(std::size_t capacity, float auto_consume_threshold = 0.75f)
            : auto_consume_threshold_(auto_consume_threshold) {
            capacity_ = nextPowerOfTwo(capacity);
            mask_ = capacity_ - 1;
            
            // 使用对齐内存分配
            size_t size = capacity_ * sizeof(T);
            void* ptr = nullptr;
            if (posix_memalign(&ptr, Alignment, size) != 0) {
                LOGE("CircularBuffer", "Failed to allocate aligned memory");
                return;
            }
            aligned_buffer_ = static_cast<T*>(ptr);
            clear();
        }
        ~CircularBuffer() {
            if (aligned_buffer_) {
                free(aligned_buffer_);
                aligned_buffer_ = nullptr;
            }
        }


        void clear(){
            write_index_.store(0, std::memory_order_relaxed);
            read_index_.store(0, std::memory_order_relaxed);
        }

        bool write(const T& item){
            return write(&item, 1) == 1;
        }

        std::size_t write(const T* items, std::size_t count) {
            std::lock_guard<std::mutex> lock(write_mutex_);
            
            std::size_t available = capacity() - size();
            std::size_t write_count = std::min(available, count);
            if (write_count == 0) return 0;
            
            std::size_t write_idx = write_index_.load(std::memory_order_relaxed);
            
            // 第一段：从写指针到缓冲区末尾
            std::size_t first_chunk = std::min(write_count, capacity_ - (write_idx & mask_));
            
            // 确保写入时考虑对齐
            size_t aligned_idx = (write_idx & mask_);
            
            // 使用memcpy确保正确处理对齐要求
            std::memcpy(&aligned_buffer_[aligned_idx], items, first_chunk * sizeof(T));
            
            // 第二段：从缓冲区开始到剩余数量
            if (first_chunk < write_count) {
                std::memcpy(&aligned_buffer_[0], items + first_chunk, (write_count - first_chunk) * sizeof(T));
            }
            
            // 更新写指针，使用 release 确保其他线程能看到数据变化
            write_index_.store(write_idx + write_count, std::memory_order_release);
            
            // 检查是否需要自动消费
            checkAutoConsume();
            
            return write_count;
        }

        bool read(T& item){
            return read(&item, 1) == 1;
        }

        std::size_t read(T* items, std::size_t count) {
            std::lock_guard<std::mutex> lock(read_mutex_);
            
            std::size_t available = size();
            std::size_t read_count = std::min(available, count);
            if (read_count == 0) return 0;
            
            std::size_t read_idx = read_index_.load(std::memory_order_relaxed);
            
            // 第一段：从读指针到缓冲区末尾
            std::size_t first_chunk = std::min(read_count, capacity_ - (read_idx & mask_));
            std::memcpy(items, &aligned_buffer_[read_idx & mask_], first_chunk * sizeof(T));
            
            // 第二段：从缓冲区开始到剩余数量
            if (first_chunk < read_count) {
                std::memcpy(items + first_chunk, &aligned_buffer_[0], (read_count - first_chunk) * sizeof(T));
            }
            
            // 更新读指针，使用 acquire 确保看到最新数据
            read_index_.store(read_idx + read_count, std::memory_order_release);
            
            return read_count;
        }
        // 缓冲区是否为空
        bool empty() const {
            return size() == 0;
        }
        
        // 缓冲区是否已满
        bool full() const {
            return size() == capacity_;
        }
        
        // 当前存储的元素数量
        std::size_t size() const {
            return write_index_.load(std::memory_order_acquire) - 
                read_index_.load(std::memory_order_acquire);
        }
        
        // 缓冲区容量
        std::size_t capacity() const {
            return capacity_;
        }

        T* getLastWritePosition() {
            std::size_t write_idx = write_index_.load(std::memory_order_acquire);
            if (write_idx == 0) return nullptr;
            
            // 返回最后写入位置的起始地址
            return &aligned_buffer_[(write_idx - 1) & mask_];
        }
        
        // 预留空间，返回可写入位置，并记录预留信息
        // 注意：这个方法仅返回第一部分的指针，不处理环绕情况
        // 环绕情况将在 commit 阶段自动处理
        T* reserve(std::size_t count) {
            std::lock_guard<std::mutex> lock(write_mutex_);
            
            std::size_t available = capacity() - size();
            if (count > available) return nullptr;  // 空间不足
            
            std::size_t write_idx = write_index_.load(std::memory_order_relaxed);
            T* result = &aligned_buffer_[write_idx & mask_];
            
            // 记录预留信息供 commit 使用
            reserved_count_ = count;
            reserved_write_idx_ = write_idx;
            
            return result;
        }

        // 提交预留的写入，自动处理环绕情况
        // data 指向要写入的数据，长度必须等于之前 reserve 的 count
        bool commit(const T* data) {
            std::lock_guard<std::mutex> lock(write_mutex_);
            
            if (reserved_count_ == 0) return false;  // 没有预留空间
            
            std::size_t write_idx = write_index_.load(std::memory_order_relaxed);
            if (write_idx != reserved_write_idx_) return false;  // 写指针已被修改
            
            // 处理环绕情况
            std::size_t first_chunk = std::min(reserved_count_, capacity_ - (write_idx & mask_));
            std::memcpy(&aligned_buffer_[write_idx & mask_], data, first_chunk * sizeof(T));
            
            // 如果需要环绕，处理第二部分
            if (first_chunk < reserved_count_) {
                std::memcpy(&aligned_buffer_[0], data + first_chunk, (reserved_count_ - first_chunk) * sizeof(T));
            }
            
            // 更新写指针
            write_index_.store(write_idx + reserved_count_, std::memory_order_release);
            reserved_count_ = 0;  // 清除预留状态
            
            // 检查是否需要自动消费
            checkAutoConsume();
            
            return true;
        }
        
        // 简化版提交，仅更新写指针
        void commit(std::size_t count) {
            std::lock_guard<std::mutex> lock(write_mutex_);
            
            std::size_t write_idx = write_index_.load(std::memory_order_relaxed);
            write_index_.store(write_idx + count, std::memory_order_release);
            
            // 如果有预留，清除预留状态
            if (reserved_count_ > 0) {
                reserved_count_ = 0;
            }
            
            // 检查是否需要自动消费
            checkAutoConsume();
        }

        // 释放预留的空间，不进行实际写入
        void release(std::size_t count) {
            std::lock_guard<std::mutex> lock(write_mutex_);
            
            // 如果有预留，清除预留状态
            if (reserved_count_ > 0) {
                reserved_count_ = 0;
            }
            // 不更新写指针，相当于取消预留
        }

        // 添加移动数据的方法
        std::unique_ptr<T[]> extract(size_t size) {
            std::lock_guard<std::mutex> lock(read_mutex_);
            
            std::size_t available = this->size();
            if (size > available) {
                return nullptr;
            }
            
            std::unique_ptr<T[]> result(new T[size]);
            std::size_t read_idx = read_index_.load(std::memory_order_relaxed);
            
            for (std::size_t i = 0; i < size; ++i) {
                result[i] = std::move(aligned_buffer_[(read_idx + i) & mask_]);
            }
            
            read_index_.store(read_idx + size, std::memory_order_release);
            
            return result;
        }

        // 显式消费数据，仅移动读指针
        void consume(std::size_t count) {
            std::lock_guard<std::mutex> lock(read_mutex_);
            std::size_t available = size();
            std::size_t consume_count = std::min(available, count);
            if (consume_count == 0) return;
            
            std::size_t read_idx = read_index_.load(std::memory_order_relaxed);
            read_index_.store(read_idx + consume_count, std::memory_order_release);
            
        }

        // 设置自动消费阈值
        void setAutoConsumeThreshold(float threshold) {
            if (threshold >= 0.0f && threshold <= 1.0f) {
                auto_consume_threshold_ = threshold;
            }
        }

        // 启用或禁用自动消费
        void enableAutoConsume(bool enable) {
            auto_consume_enabled_ = enable;
        }

        // 添加方法确保对齐访问
        T* getAlignedPointer(size_t index) {
            return &aligned_buffer_[index & mask_];
        }

    private:
        // 检查是否需要自动消费
        void checkAutoConsume() {
            if (!auto_consume_enabled_) return;
            
            float usage_ratio = static_cast<float>(size()) / capacity();
            if (usage_ratio > auto_consume_threshold_) {
                // 计算要消费的数量，消费到只剩25%的数据
                std::size_t current_size = size();
                std::size_t target_size = static_cast<std::size_t>(capacity() * 0.25f);
                if (current_size > target_size) {
                    std::size_t consume_count = current_size - target_size;
 
                    // 执行消费
                    std::size_t read_idx = read_index_.load(std::memory_order_relaxed);
                    read_index_.store(read_idx + consume_count, std::memory_order_release);
                    
                }
            }
        }

        static std::size_t nextPowerOfTwo(std::size_t x) {
            if (x == 0) return 1;
            x--;
            x |= x >> 1;
            x |= x >> 2;
            x |= x >> 4;
            x |= x >> 8;
            x |= x >> 16;
            #if SIZE_MAX > 0xFFFFFFFF // 只有当size_t是64位时才执行32位右移
            x |= x >> 32;
            #endif
            return x + 1;
        }
        T* aligned_buffer_ = nullptr;  // 替代原来的vector<T>
        std::size_t capacity_;
        std::size_t mask_;

        std::atomic<std::size_t> read_index_;
        std::atomic<std::size_t> write_index_;

        std::mutex read_mutex_;
        std::mutex write_mutex_;
        
        // 用于预留机制
        std::size_t reserved_count_{0};
        std::size_t reserved_write_idx_{0};

        std::size_t read_pos_{0};
        std::size_t available_{0};
        std::mutex mutex_;
        
        // 自动消费相关
        float auto_consume_threshold_{0.75f}; // 默认当使用率超过75%时触发自动消费
        bool auto_consume_enabled_{true};     // 默认启用自动消费
    };
}