#pragma once
#include "service.hpp"
#include <mutex>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define SERVICE_TAG "[ServiceManager]" 

class ServiceManager {
public:
    ServiceManager() = default;
    // 单例访问
    static ServiceManager& getInstance() {
        static ServiceManager instance;
        return instance;
    }

    // 服务状态枚举
    enum class ServiceStatus {
        STOPPED,
        STARTING,
        RUNNING,
        STOPPING,
        ERROR
    };

    // 注册服务（类型安全版本）
    template <typename T>
    void registerService(const std::string& name, std::shared_ptr<T> service) {
        static_assert(std::is_base_of_v<ServiceBase, T>, 
                     "Service must inherit from ServiceBase");
        
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_services.contains(name)) {
            m_services[name] = {
                std::static_pointer_cast<ServiceBase>(service),
                ServiceStatus::STOPPED
            };
            m_dependencyGraph[name] = {};  // 初始化依赖关系
        }
    }

    // 带依赖关系的启动
    bool startAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto startupOrder = resolveDependencies();
        
        bool overallSuccess = true;
        for (const auto& name : startupOrder) {
            auto& [service, status] = m_services[name];
            if (status == ServiceStatus::STOPPED) {
                status = ServiceStatus::STARTING;
                bool success = service->start();
                status = success ? ServiceStatus::RUNNING : ServiceStatus::ERROR;
                overallSuccess &= success;
                
                if (!success) {
                    notifyServiceEvent(name, "start_failed");
                }
            }
        }
        return overallSuccess;
    }

    // 添加服务依赖
    void addDependency(const std::string& service, 
                      const std::vector<std::string>& dependencies) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_dependencyGraph[service] = dependencies;
    }

    // 获取服务状态
    ServiceStatus getServiceStatus(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_services.find(name);
        if(it != m_services.end()){
            return it->second.second;
        }
        return ServiceStatus::ERROR;
    }

    std::shared_ptr<ServiceBase> getService(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_services.find(name);
        if (it != m_services.end()) {
            return it->second.first;
        }
        return nullptr; // 如果服务不存在，返回空指针
    }

    TaskHandle_t serviceHandel_t; 
private:
    std::mutex m_mutex;
    std::unordered_map<std::string, 
                      std::pair<std::shared_ptr<ServiceBase>, ServiceStatus>> m_services;
    std::unordered_map<std::string, std::vector<std::string>> m_dependencyGraph;

    // 禁用构造/拷贝
    ServiceManager(const ServiceManager&) = delete;
    ServiceManager& operator=(const ServiceManager&) = delete;

    // 依赖解析（拓扑排序）
    std::vector<std::string> resolveDependencies() {
        std::vector<std::string> order;
        std::unordered_map<std::string, bool> visited;
        std::unordered_map<std::string, bool> tempMark;

        for (const auto& [name, _] : m_services) {
            if (!visited[name]) {
                visitDependencies(name, visited, tempMark, order);
            }
        }

        std::reverse(order.begin(), order.end());
        return order;
    }

    void visitDependencies(const std::string& name,
                          std::unordered_map<std::string, bool>& visited,
                          std::unordered_map<std::string, bool>& tempMark,
                          std::vector<std::string>& order) {
        if (tempMark[name]) throw std::runtime_error("Cyclic dependency detected");
        if (visited[name]) return;

        tempMark[name] = true;
        for (const auto& dep : m_dependencyGraph[name]) {
            visitDependencies(dep, visited, tempMark, order);
        }
        tempMark[name] = false;
        visited[name] = true;
        order.push_back(name);
    }

    // 事件通知
    void notifyServiceEvent(const std::string& name, const std::string& event) {
        // 可扩展为观察者模式
        LOGI(SERVICE_TAG,"%s %s\n", name.c_str(), event.c_str());
    }
};