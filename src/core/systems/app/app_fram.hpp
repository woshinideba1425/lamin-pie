#pragma once
#include <vector>
#include "app.hpp"
#include "app_register.h"
#include "esp_log.h"

namespace LAMINATEPIE{
    enum AppState {
        ON_CREATE,      // 应用创建状态
        ON_RESUME,      // 应用恢复到前台状态
        ON_RUNNING,     // 应用在前台运行状态
        ON_PAUSE,       // 应用暂停状态
        ON_DESTROY,     // 应用销毁状态
        ON_BACKGROUND,  // 应用在后台运行状态
        ON_SUSPENDED    // 应用被挂起状态
    };

    class AppManager : public APP_Register {
    public:
        struct AppEntry {
            FreeRTOSAppBase* app;
            AppState state;
            TickType_t lastStateChangeTime;  // 记录状态改变的时间
            bool hasResumedBefore = false;
        };

        AppManager() : _foreground_app(nullptr), _updateTaskHandle(nullptr) {
            app_mutex = xSemaphoreCreateMutex();  // 创建互斥锁
        }
        
        ~AppManager() {
            destroyAllApps();
            if (app_mutex) {
                vSemaphoreDelete(app_mutex);  // 删除互斥锁
            }
        }

        void app_manager_init() {
            xTaskCreatePinnedToCore(updateTaskFunction, "MUpdateTask", 4096, this, 5, &_updateTaskHandle, 1);
        }

        // 持续更新应用状态的 FreeRTOS 任务函数
        static void updateTaskFunction(void* param) {
            AppManager* appManager = static_cast<AppManager*>(param);
            while (true) {
                appManager->update();  // 调用 update 函数
                vTaskDelay(pdMS_TO_TICKS(50));  // 每50毫秒调用一次 update()
            }
        }

        // 启动一个应用，并将其设为前台应用
        bool startApp(FreeRTOSAppBase* app, size_t stack_size = 3096) {
            if (app == nullptr) return false;

            ESP_LOGI("AppManager", "Attempting to start app: %s", app->getAppName().c_str());

            // 获取互斥锁，防止竞争条件
            xSemaphoreTake(app_mutex, portMAX_DELAY);

            // 检查应用是否已经在运行
            for (auto& entry : _running_apps) {
                if (entry.app == app) {
                    if (entry.state == ON_RUNNING) {
                        ESP_LOGI("AppManager", "App %s is already running", app->getAppName().c_str());
                        xSemaphoreGive(app_mutex);  
                        return true;
                    }
                    
                    // 如果应用在后台运行或挂起状态，改为恢复状态
                    if (entry.state == ON_BACKGROUND || entry.state == ON_SUSPENDED) {
                        entry.state = ON_RESUME;
                        _foreground_app = app;
                        
                        // 确保立即切换到应用的屏幕
                        lv_obj_t* screen = app->getScreen();
                        if (screen != nullptr) {
                            ESP_LOGI("AppManager", "Immediately switching to screen for app: %s", app->getAppName().c_str());
                            _ui_screen_change_resum(&screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0);
                        }
                        
                        ESP_LOGI("AppManager", "App %s is resumed from background", app->getAppName().c_str());
                        xSemaphoreGive(app_mutex);  
                        return true;
                    }
                    
                    entry.state = ON_RESUME;
                    _foreground_app = app;
                    ESP_LOGI("AppManager", "App %s is resumed", app->getAppName().c_str());
                    xSemaphoreGive(app_mutex);  
                    return true;
                }
            }

            // 检查任务是否已经存在，防止重复启动
            if (FreeRTOSAppBase::getTaskHandleByName(app->getAppName()) != nullptr) {
                ESP_LOGE("AppManager", "Task for app %s is already created!", app->getAppName().c_str());
                xSemaphoreGive(app_mutex);
                return false;
            }

            // 创建新的应用条目并启动它
            AppEntry new_entry;
            new_entry.app = app;
            new_entry.state = ON_CREATE;
            _running_apps.push_back(new_entry);
            
            if (app->getStack()) {
                stack_size = app->getStack();
            }
            ESP_LOGI("AppManager", "App %s is not running, starting it now with stack size %d...", 
                    app->getAppName().c_str(), stack_size);

            // 使用指定的堆栈大小创建任务
            app->createTask(app->getAppName(), stack_size);

            // 检查任务是否成功创建
            if (FreeRTOSAppBase::getTaskHandleByName(app->getAppName()) == nullptr) {
                ESP_LOGE("AppManager", "Failed to create task for app: %s", app->getAppName().c_str());
                xSemaphoreGive(app_mutex);
                return false;
            }

            // 切换到应用的界面（如果不是系统应用或是Launcher）
            lv_obj_t* screen = app->getScreen();
            if (!APP_Register::isSystemApp(app) || app->getAppName() == "Launcher") {
                if (!screen) {
                    ESP_LOGI("AppManager", "App %s's screen is uninitialized.", app->getAppName().c_str());
                } else {
                    ESP_LOGI("AppManager", "Switching to screen for new app: %s", app->getAppName().c_str());
                    _ui_screen_change_resum(&screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0);
                }
            }

            // 设置前台应用
            _foreground_app = app;
            ESP_LOGI("AppManager", "App %s is now in the foreground", app->getAppName().c_str());

            // 释放互斥锁
            xSemaphoreGive(app_mutex);
            return true;
        }

        // 将应用移至后台运行
        bool moveAppToBackground(FreeRTOSAppBase* app) {
            if (app == nullptr) return false;

            xSemaphoreTake(app_mutex, portMAX_DELAY);
            for (auto& entry : _running_apps) {
                if (entry.app == app) {
                    // 只有允许后台运行的应用才能转为后台运行状态
                    if (entry.app->isAllowBgRunning()) {
                        entry.state = ON_BACKGROUND;
                        if (entry.app == _foreground_app) {
                            _foreground_app = nullptr;
                        }
                        xSemaphoreGive(app_mutex);
                        return true;
                    } else {
                        // 不允许后台运行的应用转为暂停状态
                        entry.state = ON_PAUSE;
                        if (entry.app == _foreground_app) {
                            _foreground_app = nullptr;
                        }
                        xSemaphoreGive(app_mutex);
                        return true;
                    }
                }
            }
            xSemaphoreGive(app_mutex);
            return false;
        }

        bool pauseApp(FreeRTOSAppBase* app) {
            if (app == nullptr) return false;

            xSemaphoreTake(app_mutex, portMAX_DELAY);
            for (auto& entry : _running_apps) {
                if (entry.app == app) {
                    // 系统应用不能被暂停，只能进入后台
                    if (APP_Register::isSystemApp(app) && app->getAppName() != "Launcher") {
                        entry.state = ON_BACKGROUND;
                    } else {
                        entry.state = ON_PAUSE;
                    }
                    
                    if (entry.app == _foreground_app) {
                        _foreground_app = nullptr;
                    }
                    
                    xSemaphoreGive(app_mutex);
                    return true;
                }
            }
            xSemaphoreGive(app_mutex);
            return false;
        }

        bool destroyApp(FreeRTOSAppBase* app) {
            if (app == nullptr) return false;

            xSemaphoreTake(app_mutex, portMAX_DELAY);
            
            // 系统应用不能被销毁
            for (auto iter = _running_apps.begin(); iter != _running_apps.end(); ++iter) {
                if (iter->app == app) {
                    if (APP_Register::isSystemApp(app)) {
                        // 系统应用不销毁，改为后台运行
                        iter->state = ON_BACKGROUND;
                        if (_foreground_app == app) {
                            _foreground_app = nullptr;
                        }
                        xSemaphoreGive(app_mutex);
                        return false;
                    }
                    
                    iter->state = ON_DESTROY;
                    iter->app->terminateTask();
                    
                    if (_foreground_app == app) {
                        _foreground_app = nullptr;
                        
                        // 如果销毁了前台应用，自动切换回Launcher
                        FreeRTOSAppBase* launcher = getApp("Launcher");
                        if (launcher != nullptr) {
                            startApp(launcher);
                        }
                    }
                    
                    _running_apps.erase(iter);
                    xSemaphoreGive(app_mutex);
                    return true;
                }
            }
            xSemaphoreGive(app_mutex);
            return false;
        }

        void destroyAllApps() {
            xSemaphoreTake(app_mutex, portMAX_DELAY);
            
            auto iter = _running_apps.begin();
            while (iter != _running_apps.end()) {
                if (!APP_Register::isSystemApp(iter->app)) {
                    iter->app->terminateTask();
                    iter->app->onDestroy();
                    iter = _running_apps.erase(iter);
                } else {
                    // 系统应用不销毁，改为后台运行状态
                    iter->state = ON_BACKGROUND;
                    ++iter;
                }
            }
            
            _foreground_app = nullptr;
            xSemaphoreGive(app_mutex);
        }

        void update() {
            TickType_t currentTime = xTaskGetTickCount();
            
            for (auto& entry : _running_apps) {
                lv_obj_t* screen = nullptr;

                switch (entry.state) {
                    case ON_CREATE:
                        entry.state = ON_RESUME;
                        entry.lastStateChangeTime = currentTime;
                        break;
                        
                    case ON_RESUME:
                        if (entry.hasResumedBefore) {
                            screen = entry.app->getScreen();
                            entry.app->resumeTask();
                            
                            // 确保当应用恢复时始终切换屏幕
                            if (screen != nullptr) {
                                ESP_LOGI("AppManager", "resuming app: %s with screen change", entry.app->getAppName().c_str());
                                _ui_screen_change_resum(&screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0);
                            }
                        } else {
                            entry.hasResumedBefore = true;
                            entry.app->resumeTask();
                        }
                        entry.state = ON_RUNNING;
                        break;
                        
                    case ON_RUNNING:
                        // 检查应用是否请求关闭
                        if (entry.app->isGoingClose()) {
                            // 系统应用改为后台运行
                            if (APP_Register::isSystemApp(entry.app)) {
                                entry.state = ON_BACKGROUND;
                            } else {
                                entry.state = ON_PAUSE;
                            }
                        } 
                        // 如果不是前台应用且不是系统应用，暂停
                        else if (entry.app != _foreground_app && !APP_Register::isSystemApp(entry.app)) {
                            entry.state = ON_PAUSE;
                        }
                        break;
                        
                    case ON_PAUSE:
                        entry.app->pauseTask();  // 暂停应用任务
                        
                        // 检查是否可以在后台运行
                        if (entry.app->isAllowBgRunning()) {
                            entry.state = ON_BACKGROUND;
                        } else {
                            entry.state = ON_SUSPENDED; // 不能后台运行就挂起
                        }
                        break;
                        
                    case ON_BACKGROUND:
                        // 应用在后台运行，调用后台运行回调
                        if (!entry.app->isGoingBgRunning()) {
                            entry.app->BgRunning(); // 标记为后台运行
                        }
                        break;
                        
                    case ON_SUSPENDED:
                        // 应用被挂起，无操作
                        break;
                        
                    case ON_DESTROY:
                        // 如果是系统应用，不进行销毁，改为后台运行
                        if (APP_Register::isSystemApp(entry.app)) {
                            entry.state = ON_BACKGROUND;
                        } else {
                            entry.app->onDestroy();
                            // 注意：此应用条目将在下次循环中被移除
                        }
                        break;
                }
            }
            
            // 清理已销毁的应用
            auto iter = _running_apps.begin();
            while (iter != _running_apps.end()) {
                if (iter->state == ON_DESTROY && !APP_Register::isSystemApp(iter->app)) {
                    iter = _running_apps.erase(iter);
                } else {
                    ++iter;
                }
            }
            
        }

        bool isAppRunning(FreeRTOSAppBase* app) const {
            if (app == nullptr) return false;
            
            xSemaphoreTake(app_mutex, portMAX_DELAY);
            for (const auto& entry : _running_apps) {
                if (entry.app == app && (entry.state == ON_RUNNING || entry.state == ON_BACKGROUND)) {
                    xSemaphoreGive(app_mutex);
                    return true;
                }
            }
            xSemaphoreGive(app_mutex);
            return false;
        }

        FreeRTOSAppBase* getForegroundApp() const {
            xSemaphoreTake(app_mutex, portMAX_DELAY);
            FreeRTOSAppBase* result = _foreground_app;
            xSemaphoreGive(app_mutex);
            return result;
        }

        bool isForegroundAppRunning() const {
            xSemaphoreTake(app_mutex, portMAX_DELAY);
            bool isRunning = (_foreground_app != nullptr) && 
                           (_foreground_app->isGoingClose() == false);
            xSemaphoreGive(app_mutex);
            return isRunning;
        }

    private:
        std::vector<AppEntry> _running_apps;   // 运行中的应用列表
        FreeRTOSAppBase* _foreground_app;      // 当前前台应用
        TaskHandle_t _updateTaskHandle;        // 用于管理 FreeRTOS 任务句柄
        SemaphoreHandle_t app_mutex;           // 互斥锁，保证线程安全
    };
}