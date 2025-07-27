#include "buffer_management.h"


BufferGuard::BufferGuard(Assist::CircularBuffer<uint8_t>& buffer, size_t size)
    : buffer_(buffer), ptr_(nullptr), size_(size), committed_(false) {
    // 尝试在循环缓冲区中预留空间
    if (buffer_.size() <= buffer_.capacity() - size) {
        ptr_ = buffer_.reserve(size);
        if (!ptr_) {
            std::cout << "BufferGuard - Failed to reserve space despite having enough capacity" << std::endl;
        }
    } else {
        ptr_ = nullptr;
        std::cout << "BufferGuard - Failed to reserve buffer space: requested " << size << " bytes, available " << buffer_.capacity() - buffer_.size() << " bytes" << std::endl;
    }
}

BufferGuard::~BufferGuard() {
    // 如果未提交且预留成功，则自动释放预留的空间
    if (!committed_ && ptr_) {
        buffer_.release(size_);
    }
}

void BufferGuard::commit() {
    // 只有当预留成功且尚未提交时才执行
    if (ptr_ && !committed_) {
        buffer_.commit(size_);
        committed_ = true;
    }
}

uint8_t* BufferGuard::ptr() {
    return ptr_;
}

bool BufferGuard::valid() const {
    return ptr_ != nullptr;
} 