#pragma once
#include <string>
#include "../simplekv_nvs/simplekv_nvs.hpp"
#include "unordered_map"
#include "ui.h"
#include "esp_log.h"
#include "Lcd/hal_lcd.h"
#include <memory>

// 定义 TaskHandle_t 的删除器，用于 unique_ptr 的自定义删除操作
struct TaskHandleDeleter {
    void operator()(tskTaskControlBlock* handle) const {
        if (handle) {
            vTaskDelete(handle);  // 删除任务
        }
    }
};

class FreeRTOSAppBase {
private:
    lv_obj_t* _screen;  // 每个任务绑定一块 LVGL 屏幕
    std::string _name;
    std::unique_ptr<tskTaskControlBlock, TaskHandleDeleter> _task_handle;  // 使用 unique_ptr 管理任务句柄
    SIMPLEKV::SimpleKV* _database;
    int _stack;
    lv_event_cb_t _event_callback;
    int app_proity = 4;
    void* _user_data;
    void* _icon_addr;
    bool _allow_bg_running;
    bool _task_should_close;
    bool _task_should_destroy;
    bool _task_should_BgRunning;
    bool _is_system_app;  // 标记是否为系统应用
    static std::unordered_map<std::string, TaskHandle_t> task_handle_map;
    
    // 后台运行时的优先级
    int _background_priority = 2;

protected:
    inline void setTaskName(const std::string& name) { _name = name; }

    inline void setAllowBgRunning(bool allow) { _allow_bg_running = allow; }

    inline void setAppIcon(void* icon) { _icon_addr = icon; }
    
    inline void setSystemApp(bool is_system) { _is_system_app = is_system; }

    inline void* getUserData() { return _user_data; }

    inline void setScreen(lv_obj_t* app_screen) { _screen = app_screen; }

    inline void setStack(int stack) { _stack = stack; }
    
    // 设置后台运行时的优先级
    inline void setBackgroundPriority(int priority) { _background_priority = priority; }

    // FreeRTOS 任务函数，静态函数将调度到对象的成员函数
    static void taskFunction(void* pvParameters) {
        if (pvParameters == nullptr) {
            ESP_LOGE("FreeRTOSAppBase", "Task parameters are null!");
            vTaskDelete(NULL); 
            return;
        }
        FreeRTOSAppBase* app = static_cast<FreeRTOSAppBase*>(pvParameters);

        assert(app != nullptr);

        bool wasPaused = false;
        bool wasInBackground = false;

        if (lvgl_lock(100)) {
            app->onCreate();
            lvgl_unlock();
        }

        // 进入任务主循环
        while (!app->isGoingDestroy()) {

            // 暂停处理
            if (app->isGoingClose() && !wasPaused) {
                if (lvgl_lock(100)) {
                    app->onPause();  // 调用 onPause 处理逻辑
                    lvgl_unlock();
                }
                wasPaused = true;
                
                // 如果应用支持后台运行，不挂起任务，而是降低优先级
                if (app->isAllowBgRunning()) {
                    // 降低任务优先级，但不挂起
                    vTaskPrioritySet(app->getTaskHandle(), app->getBackgroundPriority());
                    wasInBackground = true;
                } else {
                    // 不支持后台运行的应用则挂起任务
                    vTaskSuspend(NULL);
                }
            }

            // 从暂停/后台状态恢复
            if (!app->isGoingClose() && (wasPaused || wasInBackground)) {
                if (lvgl_lock(100)) {
                    app->onResume();  
                    lvgl_unlock();
                }
                wasPaused = false; 
                
                // 如果之前在后台运行，恢复正常优先级
                if (wasInBackground) {
                    vTaskPrioritySet(app->getTaskHandle(), app->getNormalPriority());
                    wasInBackground = false;
                }
            }

            // 正常循环或后台运行
            if (wasPaused && app->isAllowBgRunning() && wasInBackground) {
                app->onRunningBG();  // 在后台执行特定逻辑
            } else if (!wasPaused) {
                app->taskLoop();     // 前台正常循环
            }

            vTaskDelay(10);
        }
        
        // 应用销毁前的清理工作
        if (lvgl_lock(100)) {
            app->onDestroy();
            lvgl_unlock();
        }
        vTaskDelete(NULL);  
    }

public:
    FreeRTOSAppBase()
        : _screen(nullptr),
          _name(""),
          _task_handle(nullptr),
          _database(nullptr),
          _stack(0),
          _user_data(nullptr),
          _icon_addr(nullptr),
          _allow_bg_running(false),
          _task_should_close(false),
          _task_should_destroy(false),
          _task_should_BgRunning(false),
          _is_system_app(false),
          _background_priority(2) {}

    virtual ~FreeRTOSAppBase() {}

    /* API for App manager */
    inline void setDatabase(SIMPLEKV::SimpleKV* db) { _database = db; }
    inline void setUserData(void* userData) { _user_data = userData; }

    /* API for lvgl callback */
    void bindEventCallback(lv_event_cb_t event_cb) { _event_callback = event_cb; }

    /* Basic API */
    inline std::string getAppName() const { return _name; }
    inline void* getAppIcon() const { return _icon_addr; }
    inline lv_obj_t* getScreen() const { return _screen; }
    lv_event_cb_t getEventCallback() const { return _event_callback; }
    inline int getStack() const { return _stack; }
    inline int getNormalPriority() const { return app_proity; }
    inline int getBackgroundPriority() const { return _background_priority; }
    
    // 获取任务句柄
    TaskHandle_t getTaskHandle() const { 
        return _task_handle.get();
    }
        
    inline bool isAllowBgRunning() const { return _allow_bg_running; }
    inline bool isGoingClose() const { return _task_should_close; }
    inline bool isGoingDestroy() const { return _task_should_destroy; }
    inline bool isGoingBgRunning() const { 
        if (_task_handle && isAllowBgRunning()) {
            return _task_should_BgRunning;
        }
        return false;
    }
    inline bool isSystemApp() const { return _is_system_app; }

    inline void closeTask() { _task_should_close = true; }
    inline void destroyTask() { _task_should_destroy = true; }
    inline void BgRunning() { _task_should_BgRunning = true; }

    static TaskHandle_t getTaskHandleByName(const std::string& name) {
        auto it = task_handle_map.find(name);
        if (it != task_handle_map.end()) {
            return it->second;
        }
        return nullptr;  // Not found
    }

    void createTask(const std::string& name, uint32_t stack_size) {
        // 任务已存在检查
        if (getTaskHandleByName(name) != nullptr) {
            ESP_LOGE("FreeRTOSAppBase", "Task %s already exists, not creating again.", name.c_str());
            return;  // 任务已存在，退出
        }

        // 创建任务
        setTaskName(name);
        TaskHandle_t task_handle = nullptr;  // 定义裸指针类型的 TaskHandle_t
        _task_should_BgRunning = false;
        xTaskCreatePinnedToCore(taskFunction, name.c_str(), stack_size, this, app_proity, &task_handle, 0);

        if (task_handle != nullptr) {
            // 使用 unique_ptr 管理 task_handle，直接传入 task_handle
            _task_handle.reset(task_handle);  // 将裸指针 task_handle 传给 unique_ptr 管理
            task_handle_map[_name] = task_handle;  // 将裸指针存储到任务表中
            ESP_LOGI("FreeRTOSAppBase", "Task %s created successfully.", name.c_str());
        } else {
            ESP_LOGE("FreeRTOSAppBase", "Failed to create task %s.", name.c_str());
        }
    }

    void pauseTask() {
        _task_should_close = true;
    }

    void resumeTask() {
        TaskHandle_t handle = xTaskGetHandle(getAppName().c_str());
        if (handle) {
            // 如果任务被挂起（不支持后台运行的应用），恢复它
            if (eTaskGetState(handle) == eSuspended) {
                vTaskResume(handle);
            }
            // 如果应用支持后台运行且正在后台运行，恢复原始优先级
            else if (isAllowBgRunning() && isGoingBgRunning()) {
                vTaskPrioritySet(handle, app_proity);
                _task_should_BgRunning = false;
            }
            
            _task_should_close = false;  // 任务恢复时重置暂停状态
        } else {
            ESP_LOGE("FreeRTOSAppBase", "Failed to resume task %s.", getAppName().c_str());
        }
    }

    void terminateTask() {
        if (_task_handle) {
            destroyTask();
            
            // 等待任务结束
            TickType_t timeout = pdMS_TO_TICKS(1000); // 设置超时时间
            TickType_t start = xTaskGetTickCount();
            
            while (eTaskGetState(_task_handle.get()) != eDeleted) {
                if ((xTaskGetTickCount() - start) > timeout) {
                    ESP_LOGW("FreeRTOSAppBase", "Task %s termination timeout", getAppName().c_str());
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            _task_handle.reset();
        }
    }

    virtual void taskLoop() = 0;
    virtual void onCreate() {}

    /// @brief 用于注册app环节的代码执行
    virtual void onSetup() {}
    virtual void wattingCallback(lv_event_t* e) {}
    virtual void onResume() {}
    virtual void onPause() {}
    virtual void onDestroy() {}
    virtual void onRunningBG() {}
};
