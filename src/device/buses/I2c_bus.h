#pragma once

#include <memory>
#include <mutex>
#include <sys/_stdint.h>
#include <driver/i2c_master.h>
#include <cstring>
#include "bus.h"
#include <async_deque.hpp>
#include <circular_buffer.hpp>
#include <buffer_management.h>
/**
 * @brief I2C总线命令结构体，描述一个完整的I2C传输
 */
struct I2CBusCommand {
    uint16_t deviceAddress;  ///< 7位设备地址
    
    /**
     * @brief I2C传输类型枚举
     */
    enum TransferType {
        WRITE_ONLY,  ///< 只写模式
        READ_ONLY,   ///< 只读模式
        WRITE_READ,  ///< 写后读模式（带重复起始位）
        PROBE_ONLY   ///< 只探测模式
    } transferType = WRITE_ONLY;  ///< 传输类型，默认为只写

    BufferView writeData;  ///< 写入数据视图
    size_t readSize = 0;   ///< 期望读取的数据大小
    
    /**
     * @brief I2C传输特有选项
     */
    struct Options {
        bool use10BitAddr = false;    ///< 使用10位地址模式
        bool noStop = false;          ///< 写操作结束后不发送停止位
        bool clockStretch = true;     ///< 允许时钟拉伸
        uint32_t timeoutMs = 1000;    ///< 操作超时时间(ms)
    } options;  ///< I2C传输选项

    // 默认构造函数
    I2CBusCommand() = default;
    
    // 移动构造函数
    I2CBusCommand(I2CBusCommand&& other) noexcept
        : deviceAddress(other.deviceAddress),
          transferType(other.transferType),
          writeData(std::move(other.writeData)),
          readSize(other.readSize),
          options(other.options) {
    }
    
    // 移动赋值运算符
    I2CBusCommand& operator=(I2CBusCommand&& other) noexcept {
        if (this != &other) {
            deviceAddress = other.deviceAddress;
            transferType = other.transferType;
            writeData = std::move(other.writeData);
            readSize = other.readSize;
            options = other.options;
        }
        return *this;
    }

    // 拷贝构造函数
    I2CBusCommand(const I2CBusCommand& other)
        : deviceAddress(other.deviceAddress),
        transferType(other.transferType),
        readSize(other.readSize),
        options(other.options) {
        // 如果原始BufferView有数据，创建一个新的内存块并复制数据
        if (other.writeData.data() != nullptr && other.writeData.size() > 0) {
            std::unique_ptr<uint8_t[]> newData(new uint8_t[other.writeData.size()]);
            std::memcpy(newData.get(), other.writeData.data(), other.writeData.size());
            writeData = BufferView(std::move(newData), other.writeData.size());
        }
    }
    // 禁用拷贝赋值
    I2CBusCommand& operator=(const I2CBusCommand&) = delete;
};

/**
 * @brief I2C总线抽象类，提供零拷贝式I2C通信功能
 * 
 * 该类使用CircularBuffer和BufferView实现高效的内存管理，
 * 提供异步通信支持，并通过锁机制确保线程安全。
 * 派生类需要实现具体的底层I2C操作。
 */
class I2c_bus : public Bus {
public:
    /**
     * @brief 构造函数
     * @param busNumber I2C总线编号
     * @param sda_pin SDA引脚号
     * @param scl_pin SCL引脚号
     * @param frequency 通信频率，单位Hz
     */
    I2c_bus(int busNumber, int sda_pin, int scl_pin, uint32_t frequency);

    /**
     * @brief 使用BusCommand执行总线传输
     * @param command 总线命令
     * @return 传输结果
     * @note 线程安全，内部使用access_mutex_保护
     */
    BusTransferResult transfer(const BusCommand& command) override;
    
    /**
     * @brief 使用I2CBusCommand执行总线传输
     * @param command I2C总线命令
     * @return 传输结果
     * @note 线程安全，内部使用access_mutex_保护
     */
    BusTransferResult transfer(const I2CBusCommand& command);
    
    /**
     * @brief 执行I2C命令的虚函数，由派生类实现
     * @param command I2C总线命令
     * @return 传输结果
     */
    virtual BusTransferResult executeCommand(const I2CBusCommand& command) = 0;
    
    /**
     * @brief 基本写操作
     * @param deviceAddr 设备地址
     * @param data 待写入数据
     * @param length 数据长度
     * @return 操作成功返回true，失败返回false
     */
    virtual bool write(uint16_t deviceAddr, const uint8_t* data, size_t length) = 0;
    
    /**
     * @brief 基本读操作
     * @param deviceAddr 设备地址
     * @param data 读取数据存储缓冲区
     * @param length 期望读取的数据长度
     * @return 操作成功返回true，失败返回false
     */
    virtual bool read(uint16_t deviceAddr, uint8_t* data, size_t length) = 0;
    
    /**
     * @brief 写后读操作(带重复起始位)
     * @param deviceAddr 设备地址
     * @param writeData 待写入数据
     * @param writeLen 写入数据长度
     * @param readData 读取数据存储缓冲区
     * @param readLen 期望读取的数据长度
     * @return 操作成功返回true，失败返回false
     */
    virtual bool writeRead(uint16_t deviceAddr, 
                          const uint8_t* writeData, size_t writeLen,
                          uint8_t* readData, size_t readLen) = 0;
    
    /**
     * @brief 从接收缓冲区读取数据
     * @param buffer 用户提供的目标缓冲区
     * @param size 期望读取的数据大小
     * @return 完全读取返回true，否则返回false
     * @note 线程安全，内部使用rx_buffer_的锁保护
     */
    bool getReadData(uint8_t* buffer, size_t size);
    
    /**
     * @brief 将数据存储到接收缓冲区
     * @param data 数据源
     * @param size 数据大小
     * @note 线程安全，内部使用rx_buffer_的锁保护
     */
    void storeReadData(const uint8_t* data, size_t size);

    /**
     * @brief 准备寄存器操作数据
     * @param addr 设备地址
     * @param reg 寄存器地址
     * @param data 数据指针
     * @param dataSize 数据大小
     * @return 缓冲区视图，失败时返回空视图
     */
    BufferView prepareRegisterData(uint16_t addr, uint8_t reg, const uint8_t* data, size_t dataSize);
    
    BufferView prepareDeviceData(const std::vector<uint8_t>& data, I2CBusCommand& cmd);
    
    /**
     * @brief 准备单字节寄存器写操作命令
     * @param addr 设备地址
     * @param reg 寄存器地址
     * @param value 要写入的值
     * @return I2C命令对象
     */
    I2CBusCommand prepareRegisterWrite(uint16_t addr, uint8_t reg, uint8_t value);
    
    /**
     * @brief 准备多字节寄存器写操作命令
     * @param addr 设备地址
     * @param reg 寄存器地址
     * @param values 要写入的值数组
     * @return I2C命令对象
     * @note 模板函数根据数组大小自动推导
     */
    template<size_t N>
    I2CBusCommand prepareRegisterWrite(uint16_t addr, uint8_t reg, const uint8_t (&values)[N]);
    
    I2CBusCommand prepareWriteRead(uint16_t addr, uint8_t reg, const uint8_t* data, size_t dataSize);

    /**
     * @brief 准备探测命令
     * @param addr 设备地址
     * @return I2C命令对象
     */
    I2CBusCommand prepareProbeCommand(uint16_t addr);
    
    /**
     * @brief 添加数据到写缓冲区并更新命令
     * @param data 源数据
     * @param cmd 将更新writeData字段的命令
     * @return 缓冲区视图，失败时返回空视图
     * @note 线程安全，内部使用tx_mutex_保护
     */
    BufferView addToWriteBuffer(const std::vector<uint8_t>& data, I2CBusCommand& cmd);
        
protected:

    /**
     * @brief 添加寄存器及数据到写缓冲区
     * @param reg 寄存器地址
     * @param data 数据指针
     * @param dataSize 数据大小
     * @return 缓冲区视图，失败时返回空视图
     * @note 线程安全，内部使用tx_mutex_保护
     */
    BufferView addToWriteBuffer(uint8_t reg, const uint8_t* data, size_t dataSize);

    BufferView addToWriteBuffer(uint16_t deviceAddr, const uint8_t* data, size_t length);
    
    /**
     * @brief 预留读缓冲区空间
     * @param size 预留大小
     * @return 缓冲区视图，失败时返回空视图
     * @note 线程安全，内部使用rx_mutex_保护
     */
    BufferView addToReadBuffer(size_t size);

private:
    const char* TAG = "I2cBus";  ///< 日志标签
    
    int busNumber_;   ///< I2C总线号
    int sda_pin_;                ///< SDA引脚
    int scl_pin_;                ///< SCL引脚
    uint32_t frequency_;         ///< 通信频率
    
    /** @brief 异步命令队列，处理I2C传输请求 */
    Assist::AsyncDeque::AsyncWorkerDeque<I2CBusCommand, BusTransferResult> async_deque_;
    
    Assist::CircularBuffer<uint8_t> tx_buffer_{128, 0.4f};  ///< 发送缓冲区，当使用率超过40%时自动消费
    Assist::CircularBuffer<uint8_t> rx_buffer_{128, 0.4f};  ///< 接收缓冲区，当使用率超过40%时自动消费
    
    std::mutex tx_mutex_;       ///< 发送缓冲区互斥锁
    std::mutex rx_mutex_;       ///< 接收缓冲区互斥锁
    std::mutex access_mutex_;   ///< 总线访问互斥锁
};