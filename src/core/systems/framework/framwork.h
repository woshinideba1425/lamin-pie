#ifndef FRAMWORK_H
#define FRAMWORK_H

#include "app/app_fram.hpp"
#include "system_resouce/hal_update_task.h"
#include "PM/pm.hpp"
#include <functional>
#include <type_traits>
#include <atomic>
#include <memory>


namespace LAMINATEPIE{

    struct FrameworkConfig_t {
        SIMPLEKV::SimpleKV_ESP* database = nullptr;
        SIMPLEKV::SimpleKV_ESP_NVS* database_nvs = nullptr;
        FreeRTOSAppBase* launcher = nullptr;
        bool playBootAnim = true;
        bool useLauncher = true;

        /* Hardware */
        uint16_t displayHor = 448;
        uint16_t displayVer = 368;

        /*Taskhandle*/
        TaskHandle_t sys_res_updateTask;
        TaskHandle_t sys_lcd_updateTask;
        
    };

    class Framework : public AppManager, public PMSystem
    {
    private:
        static Framework* _instance; // 单例实例

        FrameworkConfig_t _config;
        ST::SystemTask systemTask;   // 管理系统任务如显示刷新、资源更新
        DatabaseManager& _dbManager; // 管理硬件资源并初始化数据库
        bool persistent = true;
        bool _using_builtin_database;
        bool _using_builtin_launcher;
        bool _inited;
        static void lcd_update_task_wrapper(void* pvParameters) {
            Framework* framework = static_cast<Framework*>(pvParameters);
            framework->systemTask.lcd_update_task();  
        }

        static void data_update_task_wrapper(void* pvParameters) {
            Framework* framework = static_cast<Framework*>(pvParameters);
            framework->systemTask.systen_manage_task();
        }

        static void pm_task_wrapper(void* pvParameters) {
            Framework* framework = static_cast<Framework*>(pvParameters);
            framework->dynamicPmControl();
        }

        Framework(I2CManager& i2cManager, DatabaseManager& dbManager)
            : PMSystem(240, 80, false, ESP_PM_CPU_FREQ_MAX),
              systemTask(i2cManager, dbManager),
              _dbManager(dbManager),
              _using_builtin_database(true),
              _using_builtin_launcher(true),
              _inited(false) {}
    public:
        Framework(const Framework&) = delete;
        Framework& operator=(const Framework&) = delete;    // 禁止拷贝构造和赋值运算符，防止多实例化

        TaskHandle_t system_task_handle;


        // 初始化单例实例
        static Framework* initialize(I2CManager& i2cManager, DatabaseManager& dbManager) {
            if (_instance == nullptr) {
                _instance = new Framework(i2cManager, dbManager);
            }
            return _instance;
        }

         /*Key API*/
        static Framework* getInstance() {
            if (_instance == nullptr) {
                // 如果实例尚未初始化，抛出异常或在主函数中确保初始化
                //throw std::runtime_error("Framework instance not initialized. Call initialize first.");
            }
            return _instance;
        }
        I2CManager& getI2CManager() {return systemTask.getI2CManager();}
        HAL& getHAL() { return systemTask.getHAL();}
        ST::SystemTask& getSystemTask() {return systemTask;}
        // void enableHighPerformanceMode() {setPowerMode(MODE_HIGH_PERFORMANCE);}
        // void enableLowPowerMode() {setPowerMode(MODE_LOW_POWER);}
        inline SIMPLEKV::SimpleKV_ESP *getDatabase() { return _config.database; }
        inline SIMPLEKV::SimpleKV_ESP_NVS *getDatabase_nvs() { return _config.database_nvs; }

        /*System API*/
        void _system_data_init();
        void LAsystem();
        void anim_boot();
        void development_boot();
        bool init();
        bool install(FreeRTOSAppBase* app, void* userData);
        void activity();
        void dynamicPmControl();
        inline void setLauncher(FreeRTOSAppBase *luancher)
        {
            if (_inited)
            {
                return;
            }
            else
            {
                _config.launcher = luancher;
            }
        }
    };
}

#endif
