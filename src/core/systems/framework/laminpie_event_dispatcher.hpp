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
#include "laminpie_thread.h"
#include "laminpie_thread_errors.h"
#include <any>
#include <atomic>

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
        LOGI("LaminPie system event module initialized");
    }
    
    // 清理函数（可选）
    void Cleanup() {
        LOGI("LaminPie system event module cleaning up...");
        
        // 清理资源
        std::lock_guard<std::mutex> lock(_mutex);
        _listeners.clear();
        
        // 清空事件队列
        while (!_eventQueue.empty()) {
            _eventQueue.pop();
        }
        
        LOGI("LaminPie system event module cleanup completed");
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
    uint32_t addEventListener(typename EventType::EnumTypeAlias event_type, CallbackType callback) {
        static_assert(std::is_base_of_v<IEvent, EventType>, "EventType must inherit from IEvent");
        
        LOGD("Adding event listener for type: %d", static_cast<int>(event_type));
        
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
        auto key = std::make_pair(std::type_index(typeid(typename EventType::EnumTypeAlias)), static_cast<int>(event_type));

        if(_listeners.find(key) == _listeners.end()){
            _listeners[key] = std::make_shared<std::vector<IEventListener>>();
        }

        _listeners[key]->emplace_back(id, std::move(wrapper), std::type_index(typeid(EventType)));
        return id;
    }

    // 方案1：添加监听整个枚举类型的方法
    template<typename EventType, typename CallbackType>
    uint32_t addEventListenerForAll(CallbackType callback) {
        static_assert(std::is_base_of_v<IEvent, EventType>, "EventType must inherit from IEvent");
        
        LOGD("Adding event listener for all events of type: %s", 
                              typeid(typename EventType::EnumTypeAlias).name());
        
        // 包装回调，监听所有该枚举类型的事件
        auto wrapper = [cb = std::move(callback)](const IEvent& event) {
            if (auto* specificEvent = dynamic_cast<const EventType*>(&event)) {
                cb(*specificEvent);
            }
        };

        std::lock_guard<std::mutex> lock(_mutex);
        uint32_t id = _nextListenerId++;
        
        // 使用特殊key表示监听整个枚举类型
        // 使用-1作为特殊值表示监听所有枚举值
        auto key = std::make_pair(std::type_index(typeid(typename EventType::EnumTypeAlias)), -1);

        if(_listeners.find(key) == _listeners.end()){
            _listeners[key] = std::make_shared<std::vector<IEventListener>>();
        }

        _listeners[key]->emplace_back(id, std::move(wrapper), std::type_index(typeid(EventType)));
        return id;
    }

    // 方案2：重载addEventListener，支持监听所有事件
    template<typename EventType, typename CallbackType>
    uint32_t addEventListener(CallbackType callback) {
        return addEventListenerForAll<EventType>(std::move(callback));
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
                    LOGD("Removed event listener with ID: %" PRIu32, listenerId);
                    return true;
                }
            }
        }
        LOGW("Event listener with ID %" PRIu32 " not found", listenerId);
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
        
        std::vector<std::shared_ptr<std::vector<IEventListener>>> listeners_to_call;
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            
            // 1. 查找特定枚举值的监听器
            auto specific_key = std::make_pair(
                std::type_index(typeid(typename EventType::EnumTypeAlias)), 
                static_cast<int>(event.type)
            );
            auto specific_it = _listeners.find(specific_key);
            if (specific_it != _listeners.end()) {
                listeners_to_call.push_back(specific_it->second);
            }
            
            // 2. 查找监听整个枚举类型的监听器
            auto all_key = std::make_pair(
                std::type_index(typeid(typename EventType::EnumTypeAlias)), 
                -1  // -1表示监听所有枚举值
            );
            auto all_it = _listeners.find(all_key);
            if (all_it != _listeners.end()) {
                listeners_to_call.push_back(all_it->second);
            }
        }

        // 调用所有匹配的监听器
        for (const auto& listener_list : listeners_to_call) {
            for (const auto& listener : *listener_list) {
                try {
                    // 将具体事件类型转换为基类引用
                    const IEvent& baseEvent = static_cast<const IEvent&>(event);
                    listener.callback(baseEvent);
                } catch (const std::exception& e) {
                    LOGE("Exception in event callback: %s", e.what());
                }
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
        
        LOGD("Posted [%s] event to queue: %s", event->GetEventName().c_str(), event->GetTypeIndexString().c_str());
    }
    
    
    // 启动事件循环
    /**
     * @brief Start the event processing worker thread.
     * @note Must be called before using postEvent() for asynchronous event delivery.
     * @return true if thread started successfully, false otherwise
     */
    bool start() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_workerThread != nullptr) {
            LOGW("Event dispatcher worker thread already started");
            return true;
        }
        
        _running = true;
        
        // 使用laminpie_thread创建线程
        auto result = laminpie_thread_create(
            LAMINPIE_THREAD_PRIO_MID,  // 使用中等优先级
            &LaminPie_EventDispatcher::threadEntryPoint,  // 线程入口点
            4096,  // 4KB栈大小
            this   // 传递this指针作为用户数据
        );
        
        if (result.is_err()) {
            LOGE("Failed to create event dispatcher worker thread: %s", 
                                  result.error()->message().c_str());
            _running = false;
            return false;
        }
        
        _workerThread = result.unwrap();
        LOGI("Event dispatcher worker thread started successfully");
        return true;
    }
    
    // 停止事件循环
    /**
     * @brief Stop the event processing worker thread.
     * @note Blocks until the worker thread completes and all queued events are processed.
     * @return true if thread stopped successfully, false otherwise
     */
    bool stop() {
        laminpie_thread_t* thread_to_join = nullptr;
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_workerThread == nullptr) {
                LOGW("Event dispatcher worker thread not started");
                return true;
            }
            
            _running = false;
            _condition.notify_all();
            thread_to_join = _workerThread;
            _workerThread = nullptr;  // 立即清空，避免重复停止
        }
        
        // 等待线程结束
        auto join_result = laminpie_thread_join(thread_to_join, 5000);  // 5秒超时
        if (join_result.is_err()) {
            LOGE("Failed to join event dispatcher worker thread: %s", 
                                  join_result.error()->message().c_str());
            return false;
        }
        
        // 删除线程资源
        auto delete_result = laminpie_thread_delete(thread_to_join);
        if (delete_result.is_err()) {
            LOGE("Failed to delete event dispatcher worker thread: %s", 
                                  delete_result.error()->message().c_str());
            return false;
        }
        
        LOGI("Event dispatcher worker thread stopped successfully");
        return true;
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
    
public:
    LaminPie_EventDispatcher() : _nextListenerId(1), _workerThread(nullptr) {}
    ~LaminPie_EventDispatcher() {
        // 确保在析构时停止线程
        if (_workerThread != nullptr) {
            stop();
        }
    }

    // 静态工厂方法
    static std::unique_ptr<LaminPie_EventDispatcher> Create() {
        return std::make_unique<LaminPie_EventDispatcher>();
    }

private:

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
    
    // 线程管理
    laminpie_thread_t* _workerThread;
    std::atomic<bool> _running{false};
    
    // 线程入口点函数（静态函数）
    /**
     * @brief Static thread entry point for laminpie_thread.
     * @param user_data Pointer to LaminPie_EventDispatcher instance.
     */
    static void threadEntryPoint(void* user_data) {
        LaminPie_EventDispatcher* dispatcher = static_cast<LaminPie_EventDispatcher*>(user_data);
        if (dispatcher) {
            dispatcher->eventHandler();
        }
    }

    // 事件循环处理函数
    /**
     * @brief Worker thread main loop waiting on and processing queued events.
     * @details Waits on the condition variable until running is false and the queue drains,
     * then dispatches events in FIFO order via processQueuedEvent().
     */
    void eventHandler() {
        LOGD("Event loop started");
        
        while (_running.load()) {
            std::shared_ptr<IEvent> event;
            
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _condition.wait(lock, [this] { 
                    return !_eventQueue.empty() || !_running.load(); 
                });
                
                if (!_running.load()) {
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
        
        LOGD("Event loop stopped");
    }

    // 处理队列中的事件
    /**
     * @brief Dispatch a queued event to all listeners whose enum type matches.
     * @param event Base event reference popped from the queue.
     * @note Matching is performed by comparing the stored enum type index.
     */
    void processQueuedEvent(const IEvent& event) {
        std::vector<std::shared_ptr<std::vector<IEventListener>>> listeners_to_call;
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            // 遍历所有监听器，找到匹配的类型
            for (const auto& [key, listeners] : _listeners) {
                if (key.first == event.GetTypeIndex()) {
                    listeners_to_call.push_back(listeners);
                }
            }
        }
        
        // 调用所有匹配的监听器
        for (const auto& listener_list : listeners_to_call) {
            for (const auto& listener : *listener_list) {
                try {
                    listener.callback(event);
                } catch (const std::exception& e) {
                    LOGE("Exception in queued event callback: %s", e.what());
                }
            }
        }
    }
};

} // namespace laminpie::system::event 