#pragma once
#include "laminpie_app_base.hpp"
#include "laminpie_system_internal.h"

namespace laminpie::test {
using namespace laminpie::system::app;
/**
 * @brief 最小化测试应用类
 * 
 * 用于测试AppManager的基本功能，实现所有必需的虚函数
 */
class MinimalTestApp1 : public Laminpie_App_Base {
public:
    explicit MinimalTestApp1(const Laminpie_App_Base_Data_t &data) 
        : Laminpie_App_Base(data), 
          _test_data("test_value"),
          _is_initialized(false) {
        LOGI("MinimalTestApp1 created: %s", data.name.c_str());
    }

    virtual ~MinimalTestApp1() {
        LOGI("MinimalTestApp1 destroyed: %s", GetName().c_str());
    }

    // 实现基类虚函数
    bool OnCreate() override {
        LOGI("MinimalTestApp1 OnCreate called: %s", GetName().c_str());
        _is_initialized = true;
        return true;
    }

    bool OnClose() override {
        LOGI("MinimalTestApp1 OnClose called: %s", GetName().c_str());
        _is_initialized = false;
        return true;
    }

    bool OnResume() override {
        LOGI("MinimalTestApp1 OnResume called: %s", GetName().c_str());
        return true;
    }

    bool OnPause() override {
        LOGI("MinimalTestApp1 OnPause called: %s", GetName().c_str());
        return true;
    }

    bool OnLoop() override {
        LOGD("MinimalTestApp1 OnLoop called: %s", GetName().c_str());
        return true;
    }

    bool OnSetup() override {
        LOGI("MinimalTestApp1 OnSetup called: %s", GetName().c_str());
        SetRunningBG(true);
        return true;
    }

    bool OnRunningBG() override {
        LOGI("MinimalTestApp1 OnRunningBG called: %s", GetName().c_str());
        return true;
    }

    // 测试辅助方法
    bool IsInitialized() const { return _is_initialized; }
    const std::string& GetTestData() const { return _test_data; }

private:
    std::string _test_data;
    bool _is_initialized;
};



/**
 * @brief 最小化测试应用类
 * 
 * 用于测试AppManager的基本功能，实现所有必需的虚函数
 */
 class MinimalTestApp2 : public Laminpie_App_Base {
    public:
        explicit MinimalTestApp2(const Laminpie_App_Base_Data_t &data) 
            : Laminpie_App_Base(data), 
              _test_data("test_value"),
              _is_initialized(false) {
            LOGI("MinimalTestApp2 created: %s", data.name.c_str());
        }
    
        virtual ~MinimalTestApp2() {
            LOGI("MinimalTestApp2 destroyed: %s", GetName().c_str());
        }
    
        // 实现基类虚函数
        bool OnCreate() override {
            LOGI("MinimalTestApp2 OnCreate called: %s", GetName().c_str());
            _is_initialized = true;
            return true;
        }
    
        bool OnClose() override {
            LOGI("MinimalTestApp2 OnClose called: %s", GetName().c_str());
            _is_initialized = false;
            return true;
        }
    
        bool OnResume() override {
            LOGI("MinimalTestApp2 OnResume called: %s", GetName().c_str());
            return true;
        }
    
        bool OnPause() override {
            LOGI("MinimalTestApp2 OnPause called: %s", GetName().c_str());
            return true;
        }
    
        bool OnLoop() override {
            LOGD("MinimalTestApp2 OnLoop called: %s", GetName().c_str());
            return true;
        }

        bool OnRunningBG() override {
            LOGI("MinimalTestApp2 OnRunningBG called: %s", GetName().c_str());
            return true;
        }
        // 测试辅助方法
        bool IsInitialized() const { return _is_initialized; }
        const std::string& GetTestData() const { return _test_data; }
    
    private:
        std::string _test_data;
        bool _is_initialized;
    };
} // namespace laminpie::test
