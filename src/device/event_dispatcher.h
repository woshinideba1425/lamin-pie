#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <thread>
#include "device_types.h"

// 事件类型枚举
enum class EventType {
    DEVICE_ADDED,
    DEVICE_REMOVED,
    DEVICE_ERROR,
    DEVICE_STATUS_CHANGED,
    DEVICE_DATA_READY,
    DEVICE_READY,
    DRIVER_REGISTERED,
    BUS_SCAN_COMPLETE
};

// 事件基类
struct Event {
    EventType type;
    std::string sourceId;
    
    Event(EventType t, const std::string& source) : type(t), sourceId(source) {}
    virtual ~Event() = default;
};

// 设备事件
struct DeviceEvent : public Event {
    std::shared_ptr<DeviceIdentifier> device;
    std::vector<std::shared_ptr<DeviceIdentifier>> devices;
    
    DeviceEvent(EventType t, std::shared_ptr<DeviceIdentifier> dev) 
        : Event(t, dev->id), device(dev) {}
        
    DeviceEvent(EventType t, const std::vector<std::shared_ptr<DeviceIdentifier>>& devList)
        : Event(t, "device_list"), devices(devList) {
        if (!devList.empty()) {
            device = devList[0];
        }
    }
};

// 事件回调函数类型
using EventCallback = std::function<void(const Event&)>;

class EventDispatcher {
public:
    static EventDispatcher& getInstance() {
        static EventDispatcher instance;
        return instance;
    }
    
    // 注册事件监听器
    uint32_t addEventListener(EventType type, EventCallback callback);
    
    // 移除事件监听器
    bool removeEventListener(uint32_t listenerId);
    
    // 分发事件
    void dispatchEvent(std::shared_ptr<Event> event);
    
    // 启动和停止事件循环
    void start();
    void stop();
    
private:
    EventDispatcher();
    ~EventDispatcher();

    std::unordered_map<EventType, std::vector<std::pair<uint32_t, EventCallback>>> _listeners;
    std::queue<std::shared_ptr<Event>> _eventQueue;
    
    std::mutex _mutex;
    std::condition_variable _condition;
    bool _running;
    uint32_t _nextListenerId;
    
    std::thread _eventThread;
    void eventLoop();
};