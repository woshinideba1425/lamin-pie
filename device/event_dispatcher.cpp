#include "event_dispatcher.h"

EventDispatcher::EventDispatcher() 
    : _running(false), _nextListenerId(1) {
}

EventDispatcher::~EventDispatcher() {
    stop();
}

uint32_t EventDispatcher::addEventListener(EventType type, EventCallback callback) {
    std::lock_guard<std::mutex> lock(_mutex);
    uint32_t id = _nextListenerId++;
    _listeners[type].push_back({id, callback});
    return id;
}

bool EventDispatcher::removeEventListener(uint32_t listenerId) {
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

void EventDispatcher::dispatchEvent(std::shared_ptr<Event> event) {
    std::lock_guard<std::mutex> lock(_mutex);
    _eventQueue.push(event);
    _condition.notify_one();
}

void EventDispatcher::start() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_running) {
        _running = true;
        _eventThread = std::thread(&EventDispatcher::eventLoop, this);
    }
}

void EventDispatcher::stop() {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _running = false;
        _condition.notify_one();
    }
    
    if (_eventThread.joinable()) {
        _eventThread.join();
    }
}

void EventDispatcher::eventLoop() {
    while (true) {
        std::shared_ptr<Event> event;
        
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
            std::vector<EventCallback> callbacks;
            
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