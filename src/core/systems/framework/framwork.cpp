#include "../system_data_def.h"
#include "../app_built/builtin_apps.h"
#include "framwork.h"
#include "ui.h"

LAMINATEPIE::Framework* LAMINATEPIE::Framework::_instance = nullptr;

namespace LAMINATEPIE {


    void Framework::_system_data_init()
    {
        /* Clear database */
        _config.database->DeleteAll();

        /* Navigation */
        #ifdef LA_KEY_MENU
        _config.database->Add<bool>(LA_KEY_MENU, bool(false));
        _config.database->Add<bool>(LA_KEY_HOME, bool(false));
        _config.database->Add<bool>(LA_KEY_BACK, bool(false));
        _config.database->Add<bool>(LA_KEY_POWER, bool(false));
        _config.database->Add<bool>(LA_KEY_UP, bool(false));
        _config.database->Add<bool>(LA_KEY_DOWN, bool(false));
        #endif

        /* Time */
        #ifdef LA_TIME
        _config.database->Add<DataTime_t>(LA_TIME, DataTime_t());
        _config.database->Add<bool>(LA_TIME_JSUT_SET, bool(false));
        #endif

        /* Hardware */
        #ifdef LA_DISP_HOR

        /* Display */
        _config.database->Add<int16_t>(LA_DISP_HOR, _config.displayHor);
        _config.database->Add<int16_t>(LA_DISP_VER, _config.displayVer);
        _config.database_nvs->Add(LA_DISP_BRIGHTNESS, new uint8_t(100), sizeof(uint8_t), persistent);

        /* System tick */
        _config.database->Add<uint32_t>(LA_SYSTEM_TICKS, uint32_t(0));

        /* Power */
        _config.database->Add<uint8_t>(LA_BATTERY_LEVEL, uint8_t(100));
        _config.database->Add<bool>(LA_BATTERY_IS_CHARGING, bool(false));

        /* Wireless */
        _config.database->Add<bool>(LA_WIFI_IS_CONNECTED, bool(false));
        _config.database->Add<bool>(LA_BLE_IS_CONNECTED, bool(false));

        /* Notification */
        _config.database->Add<bool>(LA_NOTIFICATION_IS_ON, bool(false));

        /* Steps */
        _config.database->Add<uint32_t>(LA_STEPS, uint32_t(2333));

        /* Flag of just wake up from sleep */
        _config.database->Add<bool>(LA_JUST_WAKEUP, bool(false));

        /*Flag of boot option*/
        bool flag = false;
        _config.database_nvs-> Add(LA_SYSTEM_DEVELOPMENT, &flag, sizeof(bool), true);

        #endif

        /* Environmental Data */
        #ifdef LA_PRESSURE
        _config.database->Add<uint32_t>(LA_PRESSURE, uint32_t(0));  // Pressure
        #endif

        #ifdef LA_TEMPERATURE
        _config.database->Add<int32_t>(LA_TEMPERATURE, int32_t(0));  // Temperature
        #endif

        #ifdef LA_MAGNETIC
        _config.database->Add<float>(LA_MAGNETIC, float(0.0f));  // Magnetic
        #endif

        #ifdef LA_HUMIDITY
        _config.database->Add<uint32_t>(LA_HUMIDITY, uint32_t(0));  // Humidity
        #endif
        
        #ifdef LA_CO2
        _config.database->Add<uint16_t>(LA_CO2, uint16_t(0));  // CO2
        #endif

        #ifdef LA_TVOC
        _config.database->Add<uint16_t>(LA_TVOC, uint16_t(0));  // tVoC
        #endif
        
        #ifdef LA_LUMINES
        _config.database->Add<float>(LA_LUMINES, float(0.0f));  // Lumines
        #endif
    }
    


    void Framework::LAsystem()
    {
        xTaskCreatePinnedToCore(lcd_update_task_wrapper,"lcd Task",23 * 1024, this, 6, &systemTask.lcd_task_handle, 1);
    
        _system_data_init();

        xTaskCreatePinnedToCore(data_update_task_wrapper,"data Task",3 * 1024, this, 4,  &systemTask.s_data_t, 0);
        xTaskCreatePinnedToCore(pm_task_wrapper,"pm Task",3 * 1024, this, 4,  NULL, 0);
    }

    void Framework::anim_boot(){
        ui_init();
    }

    // void Framework::anim_boot(){
    //     ui_init();
    // }

    void Framework::development_boot(){

    }
    
    bool Framework::init()
    {
        if (_inited) {
            return false;
        }

        /* Clear running Apps */
        destroyAllApps();

        /*system data init*/
        _dbManager.initDatabase();
        /* Check if DatabaseManager has initialized the database */
        if (_config.database != nullptr) {
            _using_builtin_database = false;
        } else if (_dbManager.getDatabase() != nullptr) {
            // If DatabaseManager has a valid database, bind it
            _config.database = _dbManager.getDatabase();
        } else {
            // Create a new database if both are nullptr
            return false;
        }

        /* Similarly, handle the NVS database */
        if (_config.database_nvs != nullptr) {
            _using_builtin_database = false;
        } else if (_dbManager.getDatabase_nvs() != nullptr) {
            // Bind the NVS database from DatabaseManager
            _config.database_nvs = _dbManager.getDatabase_nvs();
        } else {
            // Create a new NVS database if both are nullptr
            _config.database_nvs = new SIMPLEKV::SimpleKV_ESP_NVS;
            if (_config.database_nvs == nullptr) {
                return false; // NVS database creation failed
            }
        }
        _config.launcher = new BUILTIN_APP::Launcher;

        if (_config.launcher == nullptr) {
            return false;
        }
        APP_Register::install(_config.launcher, _config.database, (void*)this);
        registerSystemApp(_config.launcher);        
        // 初始化应用管理器
        ESP_LOGI("Framework", "Initializing AppManager...");
        AppManager::app_manager_init();

        /*hardware and data*/
        LAsystem();
        ESP_LOGI("Framework","LASYSTEM.");
        
        ESP_LOGI("Framework","start launcher.");
        /* Start launcher */

        vTaskDelay(600);
        lv_disp_trig_activity(NULL);

        _inited = true;
        return true;
    }

    bool Framework::install(FreeRTOSAppBase* app, void* userData)
    {
        if (!_inited) {
            return -1;
        }
        return APP_Register::install(app, _config.database, userData);
    }

    void Framework::activity(){
        systemTask.wait_lcd_initialized();
        if (!_config.launcher) {
            ESP_LOGE("Framework", "Launcher is not initialized!");
            return;
        }

        // 检查Launcher是否已启动
        if (FreeRTOSAppBase::getTaskHandleByName(_config.launcher->getAppName()) == nullptr) {
            // 启动Launcher应用
            ESP_LOGI("Framework", "Starting Launcher application...");
            bool result = startApp(_config.launcher, 4096);
            if (result) {
                ESP_LOGI("Framework", "Launcher started successfully.");
            } else {
                ESP_LOGE("Framework", "Failed to start Launcher!");
            }
        } else {
            ESP_LOGI("Framework", "Launcher is already running.");
        }
    }

    void Framework::dynamicPmControl(){
        const uint32_t INACTIVE_TIME_THRESHOLD = 2000; // Threshold for inactive time
        bool lowPowerModeEnabled = false; 
        bool highPowerModeEnabled = false; 

        ESP_LOGI("PMSystem", "pmed");
        while (1){
            if (lv_disp_get_inactive_time(NULL) > INACTIVE_TIME_THRESHOLD) {
                if (!lowPowerModeEnabled) { 
                    enableLowPowerMode();
                    lowPowerModeEnabled = true; 
                    highPowerModeEnabled = false; 
                    // ESP_LOGI("PMSystem", "Configured for low power mode.");
                }
            } else {
                if (!highPowerModeEnabled) { 
                    enableHignPowerMode();
                    highPowerModeEnabled = true; 
                    lowPowerModeEnabled = false; 
                    // ESP_LOGI("PMSystem", "Configured for high performance mode with max CPU frequency at 240 MHz.");
                }
            }
            // ESP_LOGI("PMSystem", "pming");
            vTaskDelay(500);
        }
    }
}