#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <algorithm>

class ServiceBase {
public:
    virtual ~ServiceBase() {}

    virtual bool start() = 0;
    virtual bool restart() {
        stop();
        return start();
    }
    virtual void stop() = 0;

    virtual void registerCallback(int event, std::function<void(void*)> callback) = 0;
    virtual void unregisterCallback(int event, std::function<void(void*)> callback) = 0;

protected:
    std::unordered_map<int, std::vector<std::function<void(void*)>>> m_callbacks;

    // 事件通知方法
    void notify(int event, void* event_data = nullptr) {
        auto it = m_callbacks.find(event);
        if (it != m_callbacks.end()) {
            for (auto& cb : it->second) {
                if (cb) {
                    cb(event_data);
                }
            }
        }
    }
};