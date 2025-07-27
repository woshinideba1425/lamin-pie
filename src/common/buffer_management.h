#ifndef BUFFER_MANAGEMENT_H
#define BUFFER_MANAGEMENT_H

#include <vector>
#include <memory>
#include <cstring>
#include "circular_buffer.hpp"

/**
 * @brief 轻量级内存区域视图，用于实现零拷贝操作
 * 
 * BufferView提供对内存区域的轻量级引用，不拥有也不管理底层内存。
 * 主要用于访问CircularBuffer中预分配的内存区域，避免额外的内存拷贝。
 */
class BufferView {
public:
    // 现有构造函数
    BufferView() : data_(nullptr), size_(0), owned_(false) {}
    BufferView(uint8_t* data, size_t size) : data_(data), size_(size), owned_(false) {}
    
    // 添加支持所有权的构造函数
    BufferView(std::unique_ptr<uint8_t[]>&& data, size_t size) 
        : data_(data.release()), size_(size), owned_(true) {}
    
    // 移动构造函数
    BufferView(BufferView&& other) noexcept 
        : data_(other.data_), size_(other.size_), owned_(other.owned_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.owned_ = false;
    }
    
    // 移动赋值运算符
    BufferView& operator=(BufferView&& other) noexcept {
        if (this != &other) {
            if (owned_ && data_) {
                delete[] data_;
            }
            data_ = other.data_;
            size_ = other.size_;
            owned_ = other.owned_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.owned_ = false;
        }
        return *this;
    }
    
    // 析构函数，释放拥有的内存
    ~BufferView() {
        if (owned_ && data_) {
            delete[] data_;
        }
    }
    
    // 现有方法
    uint8_t* data() const { return data_; }
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    
    // 添加转移所有权到 std::vector 的方法
    std::vector<uint8_t> to_vector() && {
        std::vector<uint8_t> result;
        if (data_ && size_ > 0) {
            if (owned_) {
                // 如果我们拥有数据，使用移动语义
                result.reserve(size_);
                for (size_t i = 0; i < size_; ++i) {
                    result.push_back(std::move(data_[i]));
                }
                delete[] data_;
                owned_ = false;
            } else {
                // 如果不拥有数据，复制
                result.assign(data_, data_ + size_);
            }
            data_ = nullptr;
            size_ = 0;
        }
        return result;
    }
    
private:
    uint8_t* data_;
    size_t size_;
    bool owned_;  // 是否拥有数据的所有权
};

/**
 * @brief 用于安全管理缓冲区资源的RAII工具类
 * 
 * BufferGuard确保对CircularBuffer的reserve和commit操作正确配对，
 * 防止资源泄露和异常情况下的问题。
 */
class BufferGuard {
public:
    /**
     * @brief 构造函数，预留缓冲区空间
     * @param buffer 引用的CircularBuffer对象
     * @param size 要预留的空间大小（字节）
     */
    BufferGuard(Assist::CircularBuffer<uint8_t>& buffer, size_t size);
    
    /**
     * @brief 析构函数，如未手动提交将自动提交预留的空间
     */
    ~BufferGuard();
    
    /**
     * @brief 手动提交预留的空间
     */
    void commit();
    
    /**
     * @brief 获取预留的缓冲区指针
     * @return 指向预留区域的指针，预留失败返回nullptr
     */
    uint8_t* ptr();
    
    /**
     * @brief 检查预留是否成功
     * @return 预留成功返回true，失败返回false
     */
    bool valid() const;
    
private:
    Assist::CircularBuffer<uint8_t>& buffer_; ///< 引用的循环缓冲区
    uint8_t* ptr_;                            ///< 预留的内存区域指针
    size_t size_;                             ///< 预留的大小
    bool committed_;                          ///< 是否已提交标志
};


#endif