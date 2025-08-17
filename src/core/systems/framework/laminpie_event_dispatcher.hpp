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
/**
 * @brief Central event dispatcher singleton.
 * @details Provides synchronous (dispatchEvent) and asynchronous (postEvent) delivery
 * of strongly-typed events. Listeners are indexed by a composite key of
 * (event enum type, enum value). Asynchronous delivery is performed by an
 * internal worker thread that processes a FIFO queue.
 *
 * Thread-safety: All public APIs synchronize on an internal mutex. The queue
 * is protected by a condition variable. Call start() before using postEvent();
 * call stop() during shutdown to join the worker thread.
 */
class LaminPie_EventDispatcher {
public:
    // 获取单例实例
    static LaminPie_EventDispatcher& getInstance() {
        static LaminPie_EventDispatcher instance;
        return instance;
    }
    
    // 初始化函数
    void Init() {
        SYSTEM_EVENT_LOG_INFO("LaminPie system event module initialized");
    }
    
    // 清理函数（可选）
    void Cleanup() {
        SYSTEM_EVENT_LOG_INFO("LaminPie system event module cleaning up...");
        
        // 清理资源
        std::lock_guard<std::mutex> lock(_mutex);
        _listeners.clear();
        
        // 清空事件队列
        while (!_eventQueue.empty()) {
            _eventQueue.pop();
        }
        
        SYSTEM_EVENT_LOG_INFO("LaminPie system event module cleanup completed");
    }

    // 通用事件监听器结构
    /**
     * @brief Listener descriptor with type-erased callback.
     * @note The callback receives a base IEvent reference and is wrapped to
     *       safely downcast to the concrete EventType before invocation.
     */
    struct IEventListener {
        uint32_t id;
        std::function<void(const IEvent&)> callback;
        std::type_index eventType;
        
        IEventListener(uint32_t listener_id, std::function<void(const IEvent&)> cb, std::type_index type)
            : id(listener_id), callback(std::move(cb)), eventType(type) {}
    };

    // 添加事件监听器 - 模板方法，自动推断事件类型
    /**
     * @brief Register a listener for a specific event enum value.
     * @tparam EventType Concrete event type deriving from IEvent.
     * @tparam CallbackType Callable with signature void(const EventType&).
     * @param event_type Enum value to listen for.
     * @param callback User callback to invoke when the matching event is delivered.
     * @return Assigned listener id which can be used with removeEventListener().
     */
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

        if(_listeners.find(key) == _listeners.end()){
            _listeners[key] = std::make_shared<std::vector<IEventListener>>();
        }

        _listeners[key]->emplace_back(id, std::move(wrapper), std::type_index(typeid(EventType)));
        return id;
    }

    // 特殊的UI事件监听器（保持兼容性）
    /**
     * @brief Register an LVGL UI event callback for a given object/event code.
     * @param ui_event_type Unused dispatcher's enum channel, kept for API symmetry.
     * @param ui_update_data LVGL callback, event code and user data holder.
     * @return 0 (listener id not tracked by dispatcher since LVGL owns it).
     * @note This keeps compatibility with LVGL's event model by delegating to lv_obj_add_event_cb().
     */
    uint32_t addEventListener(Laminpie_UI_Event_Type ui_event_type, Ui_Update_Event_t ui_update_data) {
        lv_obj_t *obj = ui_update_data.obj;
        lv_event_code_t event = ui_update_data.event;
        lv_event_cb_t cb = ui_update_data.cb;
        void *user_data = ui_update_data.user_data;

        lv_obj_add_event_cb(obj, cb, event, user_data);
        return 0;
    }
    
    // 移除事件监听器
    /**
     * @brief Remove a previously registered listener by id.
     * @param listenerId The id returned by addEventListener().
     * @return true if a matching listener existed and was removed; false otherwise.
     */
    bool removeEventListener(uint32_t listenerId) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& [key, listeners] : _listeners) {
            for (auto it = listeners->begin(); it != listeners->end(); ++it) {
                if (it->id == listenerId) {
                    listeners->erase(it);
                    SYSTEM_EVENT_LOG_DEBUG("Removed event listener with ID: %u", listenerId);
                    return true;
                }
            }
        }
        SYSTEM_EVENT_LOG_WARN("Event listener with ID %u not found", listenerId);
        return false;
    }
    
    // 分发事件 - 模板方法，自动推断事件类型
    /**
     * @brief Synchronously dispatch an event to all matching listeners.
     * @tparam EventType Concrete event type deriving from IEvent.
     * @param event Event instance to deliver immediately on the calling thread.
     * @note This call blocks until all callbacks complete. Prefer short, non-blocking
     *       callbacks; use postEvent() for decoupled/long-running work.
     */
    template<typename EventType>
    void dispatchEvent(const EventType& event) {
        static_assert(std::is_base_of_v<IEvent, EventType>, "EventType must inherit from IEvent");
        
        SYSTEM_EVENT_LOG_DEBUG("Dispatching [%s] event: %s", event.GetEventName().cstr(), event.GetTypeIndexString().cstr());
        
        std::shared_ptr<std::vector<IEventListener>> listeners_to_call;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto key = std::make_pair(std::type_index(typeid(typename EventType::EnumType)), static_cast<int>(event.type));
            auto it = _listeners.find(key);
            if (it != _listeners.end()) {
                listeners_to_call = it->second;
            }
        }

        // 调用所有匹配的监听器
        for (const auto& listener : *listeners_to_call) {
            try {
                listener.callback(event);
            } catch (const std::exception& e) {
                SYSTEM_EVENT_LOG_ERROR("Exception in event callback: %s", e.what());
            }
        }
    }
    
    // 异步分发事件到事件队列
    /**
     * @brief Asynchronously enqueue an event for later delivery on the worker thread.
     * @tparam EventType Concrete event type deriving from IEvent.
     * @param event Shared pointer to the event instance to enqueue (FIFO).
     * @post Notifies the condition variable to wake the event loop.
     * @note Requires start() to be called beforehand; otherwise the queue will not be processed.
     */
    template<typename EventType>
    void postEvent(std::shared_ptr<EventType> event) {
        static_assert(std::is_base_of_v<IEvent, EventType>, "EventType must inherit from IEvent");
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _eventQueue.push(event);
            _condition.notify_one();
        }
        
        SYSTEM_EVENT_LOG_DEBUG("Posted [%s] event to queue: %s", event->GetEventName().cstr(), event->GetTypeIndexString().cstr());
    }
    
    
    // 获取监听器数量（用于调试）
    /**
     * @brief Get the total count of registered listeners (for diagnostics).
     * @return Number of listener entries across all keys.
     */
    size_t getListenerCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
        size_t count = 0;
        for (const auto& [key, listeners] : _listeners) {
            count += listeners->size();
        }
        return count;
    }
    
private:
    LaminPie_EventDispatcher() : _nextListenerId(1) {}
    ~LaminPie_EventDispatcher() {}

    // 禁止拷贝和移动
    LaminPie_EventDispatcher(const LaminPie_EventDispatcher&) = delete;
    LaminPie_EventDispatcher& operator=(const LaminPie_EventDispatcher&) = delete;
    LaminPie_EventDispatcher(LaminPie_EventDispatcher&&) = delete;
    LaminPie_EventDispatcher& operator=(LaminPie_EventDispatcher&&) = delete;

    // 使用复合key：(枚举类型，枚举值) -> 监听器列表
    std::unordered_map<std::pair<std::type_index, int>, 
                       std::shared_ptr<std::vector<IEventListener>>, 
                       std::hash<std::pair<std::type_index, int>>> _listeners;
    
    std::queue<std::shared_ptr<IEvent>> _eventQueue;
    
    mutable std::mutex _mutex;
    std::condition_variable _condition;
    uint32_t _nextListenerId;
    

    // 事件循环处理函数
    /**
     * @brief Worker thread main loop waiting on and processing queued events.
     * @details Waits on the condition variable until running is false and the queue drains,
     * then dispatches events in FIFO order via processQueuedEvent().
     */
    void eventHandler() {
        SYSTEM_EVENT_LOG_DEBUG("Event loop started");
        
        
        while (true ) {
            std::shared_ptr<IEvent> event;
            
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _condition.wait(lock, [this] { 
                    return !_eventQueue.empty(); 
                });
                
                if (_eventQueue.empty()) {
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
    /**
     * @brief Dispatch a queued event to all listeners whose enum type matches.
     * @param event Base event reference popped from the queue.
     * @note Matching is performed by comparing the stored enum type index.
     */
    void processQueuedEvent(const IEvent& event) {
        std::shared_ptr<std::vector<IEventListener>> listeners_to_call;
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            // 遍历所有监听器，找到匹配的类型
            for (const auto& [key, listeners] : _listeners) {
                if (key.first == event.GetTypeIndex()) {
                    for (const auto& listener : *listeners) {
                        listeners_to_call->push_back(listener);
                    }
                }
            }
        }
        
        // 调用所有匹配的监听器
        for (const auto& listener : *listeners_to_call) {
            try {
                listener.callback(event);
            } catch (const std::exception& e) {
                SYSTEM_EVENT_LOG_ERROR("Exception in queued event callback: %s", e.what());
            }
        }
    }
};

} // namespace laminpie::system::event 