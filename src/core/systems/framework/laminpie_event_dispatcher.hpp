#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>
#include <type_traits>
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <thread>
#include "laminpie_system_event_type.hpp"
#include "../laminpie_system_internal.h"


namespace laminpie::system::event {

template<typename EventArgT, typename EnumT>
concept IsDerivedFromEvent = std::is_base_of_v<Event<EnumT>, EventArgT>;

template<typename CallbackT, typename EventArgT>
concept IsCallbackForEvent = requires(CallbackT callback, EventArgT event){
    callback(event);
};

template<EnumType EventType>
class LaminPie_EventDispatcher {
public:
    static LaminPie_EventDispatcher& getInstance() {
        static LaminPie_EventDispatcher instance;
        return instance;
    }
    
    // 注册事件监听器
    uint32_t addEventListener(T type, EventCallback<T> callback) {
        SYSTEM_EVENT_LOG_DEBUG("addEventListener: type=%d, callback=%p", type, callback);
        std::lock_guard<std::mutex> lock(_mutex);
        uint32_t id = _nextListenerId++;
        _listeners[type].push_back({id, callback});
        return id;
    }

    uint32_t addEventListener(Ui_Event_t type, EventCallback<Ui_Event_t> callback) {
        SYSTEM_EVENT_LOG_DEBUG("addEventListener: type=%d, callback=%p", type, callback);
        std::lock_guard<std::mutex> lock(_mutex);
        lv_obj_send_event(lv_obj_get_screen(lv_scr_act()), type, callback);
        return id;
    }
    
    // 移除事件监听器
    bool removeEventListener(uint32_t listenerId) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& [type, listeners] : _listeners) {
            for (auto it = listeners.begin(); it != listeners.end(); ++it) {
                if (it->first == listenerId) {
                    listeners.erase(it);
                    return true;
                }
            }
        }
        return false;
    }
    
    // 分发事件
    void dispatchEvent(std::shared_ptr<Event<T>> event) {
        std::lock_guard<std::mutex> lock(_mutex);
        _eventQueue.push(event);
        _condition.notify_one();
    }
    
    // 启动和停止事件循环
    void start() {
        SYSTEM_EVENT_LOG_INFO("LaminPie system event module starting...");
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_running) {
            _running = true;
            _eventThread = std::thread(&LaminPie_EventDispatcher::eventLoop, this);
        }
    }
    
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
    
private:
    LaminPie_EventDispatcher() : _running(false), _nextListenerId(1) {}
    ~LaminPie_EventDispatcher() { stop(); }

    std::unordered_map<T, std::vector<std::pair<uint32_t, EventCallback<T>>>> _listeners;
    std::queue<std::shared_ptr<Event<T>>> _eventQueue;
    
    std::mutex _mutex;
    std::condition_variable _condition;
    bool _running;
    uint32_t _nextListenerId;
    
    std::thread _eventThread;
    void eventLoop() {
        while (true) {
            std::shared_ptr<Event<T>> event;
            
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
                // 复制回调列表，避免回调过程中修改列表导致问题
                SYSTEM_EVENT_LOG_DEBUG("eventLoop: event=%p", event);
                std::vector<EventCallback<T>> callbacks;
                
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    auto it = _listeners.find(event->type);
                    if (it != _listeners.end()) {
                        for (const auto& [id, callback] : it->second) {
                            callbacks.push_back(callback);
                        }
                    }
                }
                
                // 调用所有注册的回调
                for (const auto& callback : callbacks) {
                    callback(*event);
                }
            }
        }
    }
};
}