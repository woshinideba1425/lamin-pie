#pragma once
#include "system_data_def.h"
#include "framework/framwork.h"

namespace LAMINATEPIE
{

    class Laminate
    {
    private:
        I2CManager i2cManager;
        DatabaseManager dbManager;
        Framework *framework; // 保存 Framework 单例的指针

        static void _lvgl_app_event(lv_event_t* e);


    public:
        // 在构造函数中获取 Framework 的单例
        Laminate()
            : i2cManager(I2C_NUM_0, 1000), // 确保在 Laminate 中初始化 i2cManager
              dbManager(),
              framework(nullptr)// 初始化时 framework 指针为空
        {
            i2cManager.begin(I2C_SDA,I2C_SCL,400000);
        } 

        ~Laminate() = default;

        // 系统初始化函数，用于初始化 Framework 单例
        void sys_init()
        {
            // 调用 Framework 的初始化方法，获取单例实例
            framework = Framework::initialize(i2cManager, dbManager);

            assert(framework != nullptr); 
            if (framework == nullptr) {
                ESP_LOGE("Laminate", "Failed to initialize Framework.");
                return;
            }
            bool init_success = framework->init();
            if (!init_success) {
                ESP_LOGE("Laminate", "Framework initialization failed.");
            } else {
                
            }
        }

        // 之后可以在 Laminate 类中使用 framework 指针来调用 Framework 的功能
        Framework *getFrameworkInstance()
        {
            return framework;
        }

        void app_in_use();
        void star_app_framework();
    };
}