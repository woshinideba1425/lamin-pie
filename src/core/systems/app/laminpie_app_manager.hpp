#pragma once
#include <vector>
#include "laminpie_app_base.hpp"
#include "laminpie_app_register.h"
#include "laminpie_system_internal.h"

namespace laminpie::system::app {
    enum AppState {
        ON_CREATE,      // 应用创建状态
        ON_RESUME,      // 应用恢复到前台状态
        ON_RUNNING,     // 应用在前台运行状态
        ON_PAUSE,       // 应用暂停状态
        ON_DESTROY,     // 应用销毁状态
        ON_BACKGROUND,  // 应用在后台运行状态
    };

    class Laminpie_App_Manager : public Laminpie_App_Register {
    public:
        struct AppEntry {
            app::Laminpie_App_Base* app;
            AppState state;
            std::chrono::steady_clock::time_point lastStateChangeTime;  // 记录状态改变的时间
            bool hasResumedBefore = false;
        };

        Laminpie_App_Manager() : _foreground_app(nullptr) {}
        
        ~Laminpie_App_Manager() {
            destroyAllApps();
        }

        void app_manager_init() {
            // xTaskCreatePinnedToCore(updateTaskFunction, "MUpdateTask", 4096, this, 5, &_updateTaskHandle, 1);
        }

        // 持续更新应用状态的 FreeRTOS 任务函数
        static void updateTaskFunction(void* param) {
            Laminpie_App_Manager* appManager = static_cast<Laminpie_App_Manager*>(param);
            while (true) {
                appManager->update();  // 调用 update 函数
                std::this_thread::sleep_for(std::chrono::milliseconds(50));  // 每50毫秒调用一次 update()
            }
        }

        // 启动一个应用，并将其设为前台应用
        bool startApp(app::Laminpie_App_Base* app, size_t stack_size = 3096) {

            return true;
        }

        // 将应用移至后台运行
        bool moveAppToBackground(Laminpie_App_Base* app) {

            return false;
        }

        bool pauseApp(Laminpie_App_Base* app) {
            return false;
        }

        bool destroyApp(Laminpie_App_Base* app) {
            return false;
        }

        void destroyAllApps() {
        }

        void update() {
            
        }

        bool isAppRunning(Laminpie_App_Base* app) const {

        }

        Laminpie_App_Base* getForegroundApp() const {

        }

        bool isForegroundAppRunning() const {
            return false;
        }

    private:
        std::vector<AppEntry> _running_apps;   // 运行中的应用列表
        Laminpie_App_Base* _foreground_app;      // 当前前台应用
    };
}