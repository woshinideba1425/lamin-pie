#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>
#include <stdint.h>
#include <type_traits>
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <thread>
#include <any>

#include "../laminpie_system_internal.h"
#include "laminpie_system_event_type.hpp"
#include "lvgl.h"

// 为std::pair提供hash函数
namespace std {
    template<>
    struct hash<std::pair<std::type_index, int>> {
        size_t operator()(const std::pair<std::type_index, int>& p) const {
            return std::hash<std::type_index>{}(p.first) ^ (std::hash<int>{}(p.second) << 1);
        }
    };
}

namespace laminpie::system::event {

// 事件分发器 - 单例模式，处理所有类型的事件
class LaminPie_EventDispatcher {
public:
    // 获取单例实例
    static LaminPie_EventDispatcher& getInstance() {
        static LaminPie_EventDispatcher instance;
        return instance;
    }

    // 通用事件监听器结构
    struct IEventListener {
        uint32_t id;
        std::function<void(const IEvent&)> callback;
        std::type_index eventType;
        
        IEventListener(uint32_t listener_id, std::function<void(const IEvent&)> cb, std::type_index type)
            : id(listener_id), callback(std::move(cb)), eventType(type) {}
    };

    // 添加事件监听器 - 模板方法，自动推断事件类型
    template<typename EventType, typename CallbackType>
    uint32_t addEventListener(typename EventType::EnumType event_type, CallbackType callback) {
        static_assert(std::is_base_of_v<IEvent, EventType>, "EventType must inherit from IEvent");
        
        SYSTEM_EVENT_LOG_DEBUG("Adding event listener for type: %d", static_cast<int>(event_type));
        
        // 将具体类型的回调包装为通用回调
        auto wrapper = [cb = std::move(callback), event_type](const IEvent& event) {
            if (auto* specificEvent = dynamic_cast<const EventType*>(&event)) {
                if (specificEvent->type == event_type) {
                    cb(*specificEvent);
                }
            }
        };

        std::lock_guard<std::mutex> lock(_mutex);
        uint32_t id = _nextListenerId++;
        
        // 使用事件类型的枚举类型作为key
        auto key = std::make_pair(std::type_index(typeid(typename EventType::EnumType)), static_cast<int>(event_type));
        _listeners[key].emplace_back(id, std::move(wrapper), std::type_index(typeid(EventType)));
        
        return id;
    }

    // 特殊的UI事件监听器（保持兼容性）
    uint32_t addEventListener(Laminpie_AppEventType ui_event_type, Ui_Update_Event_t ui_update_data) {
        lv_obj_t *obj = ui_update_data.obj;
        lv_event_code_t event = ui_update_data.event;
        lv_event_cb_t cb = ui_update_data.cb;
        void *user_data = ui_update_data.user_data;

        lv_obj_add_event_cb(obj, cb, event, user_data);
        return 0;
    }
    
    // 移除事件监听器
    bool removeEventListener(uint32_t listenerId) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& [key, listeners] : _listeners) {
            for (auto it = listeners.begin(); it != listeners.end(); ++it) {
                if (it->id == listenerId) {
                    listeners.erase(it);
                    SYSTEM_EVENT_LOG_DEBUG("Removed event listener with ID: %u", listenerId);
                    return true;
                }
            }
        }
        SYSTEM_EVENT_LOG_WARN("Event listener with ID %u not found", listenerId);
        return false;
    }
    
    // 分发事件 - 模板方法，自动推断事件类型
    template<typename EventType>
    void dispatchEvent(const EventType& event) {
        static_assert(std::is_base_of_v<IEvent, EventType>, "EventType must inherit from IEvent");
        
        SYSTEM_EVENT_LOG_DEBUG("Dispatching event of type: %d", static_cast<int>(event.type));
        
        std::vector<IEventListener> listeners_to_call;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto key = std::make_pair(std::type_index(typeid(typename EventType::EnumType)), static_cast<int>(event.type));
            auto it = _listeners.find(key);
            if (it != _listeners.end()) {
                listeners_to_call = it->second;
            }
        }

        // 调用所有匹配的监听器
        for (const auto& listener : listeners_to_call) {
            try {
                listener.callback(event);
            } catch (const std::exception& e) {
                SYSTEM_EVENT_LOG_ERROR("Exception in event callback: %s", e.what());
            }
        }
    }
    
    // 异步分发事件到事件队列
    template<typename EventType>
    void postEvent(std::shared_ptr<EventType> event) {
        static_assert(std::is_base_of_v<IEvent, EventType>, "EventType must inherit from IEvent");
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _eventQueue.push(event);
            _condition.notify_one();
        }
        
        SYSTEM_EVENT_LOG_DEBUG("Posted event to queue");
    }
    
    // 启动事件循环
    void start() {
        SYSTEM_EVENT_LOG_INFO("LaminPie system event module starting...");
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_running) {
            _running = true;
            _eventThread = std::thread(&LaminPie_EventDispatcher::eventLoop, this);
        }
    }
    
    // 停止事件循环
    void stop() {
        SYSTEM_EVENT_LOG_INFO("LaminPie system event module stopping...");
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _running = false;
            _condition.notify_one();
        }
        
        if (_eventThread.joinable()) {
            _eventThread.join();
        }
    }
    
    // 获取监听器数量（用于调试）
    size_t getListenerCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
        size_t count = 0;
        for (const auto& [key, listeners] : _listeners) {
            count += listeners.size();
        }
        return count;
    }
    
private:
    LaminPie_EventDispatcher() : _running(false), _nextListenerId(1) {}
    ~LaminPie_EventDispatcher() { 
        stop(); 
    }

    // 禁止拷贝和移动
    LaminPie_EventDispatcher(const LaminPie_EventDispatcher&) = delete;
    LaminPie_EventDispatcher& operator=(const LaminPie_EventDispatcher&) = delete;
    LaminPie_EventDispatcher(LaminPie_EventDispatcher&&) = delete;
    LaminPie_EventDispatcher& operator=(LaminPie_EventDispatcher&&) = delete;

    // 使用复合key：(枚举类型，枚举值) -> 监听器列表
    std::unordered_map<std::pair<std::type_index, int>, std::vector<IEventListener>, 
                       std::hash<std::pair<std::type_index, int>>> _listeners;
    
    std::queue<std::shared_ptr<IEvent>> _eventQueue;
    
    mutable std::mutex _mutex;
    std::condition_variable _condition;
    bool _running;
    uint32_t _nextListenerId;
    
    std::thread _eventThread;

    // 事件循环处理函数
    void eventLoop() {
        SYSTEM_EVENT_LOG_DEBUG("Event loop started");
        
        while (true) {
            std::shared_ptr<IEvent> event;
            
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _condition.wait(lock, [this] { 
                    return !_running || !_eventQueue.empty(); 
                });
                
                if (!_running && _eventQueue.empty()) {
                    break;
                }
                
                if (!_eventQueue.empty()) {
                    event = _eventQueue.front();
                    _eventQueue.pop();
                }
            }
            
            if (event) {
                // 根据事件类型分发
                processQueuedEvent(*event);
            }
        }
        
        SYSTEM_EVENT_LOG_DEBUG("Event loop stopped");
    }

    // 处理队列中的事件
    void processQueuedEvent(const IEvent& event) {
        std::vector<IEventListener> listeners_to_call;
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            // 遍历所有监听器，找到匹配的类型
            for (const auto& [key, listeners] : _listeners) {
                if (key.first == event.getTypeIndex()) {
                    for (const auto& listener : listeners) {
                        listeners_to_call.push_back(listener);
                    }
                }
            }
        }
        
        // 调用所有匹配的监听器
        for (const auto& listener : listeners_to_call) {
            try {
                listener.callback(event);
            } catch (const std::exception& e) {
                SYSTEM_EVENT_LOG_ERROR("Exception in queued event callback: %s", e.what());
            }
        }
    }
};

} // namespace laminpie::system::event 