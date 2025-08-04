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
#include "laminpie_system_event_type.hpp"
#include "../laminpie_system_internal.h"
#include "lvgl.h"


namespace laminpie::system::event {

// 概念1: 检查 EventArgT 是否为 Event<EnumT> 的派生类
template<typename EventArgT, typename EnumT>
concept IsDerivedFromEvent = std::is_base_of_v<laminpie::system::event::Event<EnumT>, EventArgT>;

// 概念2: 检查 CallbackT 是否为接受特定事件参数的回调函数
// 这个概念会检查一个可调用对象（函数指针、lambda等）是否能接收一个特定类型的参数
template<typename CallbackT, typename EventArgT>
concept IsCallbackForEvent = requires(CallbackT callback, EventArgT event) {
    callback(event);
};

template<EnumType T>
class LaminPie_EventDispatcher {
public:
    // 使用 std::function 进行类型擦除，存储接受基类引用的回调
    using GenericEventCallback = std::function<void(const laminpie::system::event::Event<T>&)>;
    
    struct Listener {
        uint32_t id;
        GenericEventCallback callback;
    };

    static LaminPie_EventDispatcher& getInstance() {
        static LaminPie_EventDispatcher instance;
        return instance;
    }
    
    // addEventListener 现在是一个模板方法，可以推断回调和事件的具体类型
    template<typename EventCallback, typename EventArgType>
        requires IsCallbackForEvent<EventCallback, const EventArgType&> && 
                 IsDerivedFromEvent<EventArgType, T>
    uint32_t addEventListener(T type, EventCallback callback) {
        SYSTEM_EVENT_LOG_DEBUG("Adding event listener for type: %d", static_cast<int>(type));
        
        // 将具体的回调 (e.g., void(const MyEvent&)) 包装成通用的回调 (void(const Event<T>&))
        GenericEventCallback wrapper = [cb = std::move(callback)](const laminpie::system::event::Event<T>& event) {
            // 在调用前回进行安全的类型转换
            if (auto* specificEvent = dynamic_cast<const EventArgType*>(&event)) {
                cb(*specificEvent);
            }
        };

        std::lock_guard<std::mutex> lock(_mutex);
        uint32_t id = _nextListenerId++;
        _listeners[type].push_back({id, std::move(wrapper)});
        return id;
    }

    uint32_t addEventListener(Laminpie_AppEventType ui_data, Ui_Update_Event_t ui_update_data){
        lv_obj_t *obj = ui_update_data.obj;
        lv_theme_t *theme = ui_update_data.theme;
        lv_event_code_t event = ui_update_data.event;
        lv_event_cb_t cb = ui_update_data.cb;
        void *user_data = ui_update_data.user_data;

        lv_obj_add_event_cb(obj, cb, event, user_data);
        return 0;
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
    template<typename EventArgType>
        requires IsDerivedFromEvent<EventArgType, T>
    void dispatchEvent(const EventArgType& event) {
        std::vector<Listener> listeners_to_call;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_listeners.count(event.type)) {
                listeners_to_call = _listeners.at(event.type);
            }
        }

        for (const auto& listener : listeners_to_call) {
            // 直接调用包装好的通用回调
            listener.callback(event);
        }
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

    std::unordered_map<T, std::vector<std::pair<uint32_t, GenericEventCallback>>> _listeners;
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
                std::vector<GenericEventCallback> callbacks;
                
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