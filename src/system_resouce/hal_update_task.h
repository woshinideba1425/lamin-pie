#pragma once

#include "hal.hpp"
#include "DatabaseManager.hpp"
#define LCD_INITIALIZED_BIT (1 << 0)

namespace ST{
    
    #define NVS_BASE_NAME "storage"
    #define SGP_TVOC_BASE_KEY "SGP_TVOC2"
    #define SGP_ECO2_BASE_KEY "SGP_ECO22"

    struct RtcData_t {
        /* update time in 1Hz */
        int64_t update_interval = 100 * 0000;
        int64_t update_count = 0;
        LAMINATEPIE::DataTime_t* time_ptr = nullptr;
        bool* time_just_set_ptr = nullptr;
        LAMINATEPIE::DataTime_t rtc_time;
    };


    struct PowerInfos_t {
        uint8_t* battery_level_ptr = nullptr;
        bool* battery_is_charging_ptr = nullptr;
    };


    struct ImuData_t {
        /* update IMU data in 50Hz */
        int64_t update_interval = 20;
        int64_t update_count = 0;
        uint32_t* steps = nullptr;
    };


    enum PowerMode_t {
        mode_normal = 0,
        mode_going_sleep,
        mode_sleeping,
        mode_cold_sleeping
    };


    struct SystemData_t {
        bool* just_wake_up_ptr = nullptr;
    };


    struct PowerManager_t {
        PowerMode_t power_mode = mode_normal;

        uint32_t auto_sleep_time = 8000;
    };
    
    struct EnvironmentalData {
        int64_t update_interval = 200 * 1000;
        int64_t update_count = 0;
        uint32_t* pressure = 0;
        uint32_t* Humidity = 0;
        int32_t* Tempture = 0;
        uint16_t* eCO2 = 0;
        uint16_t* tVoC = 0;
    };

    struct MaxManager {
        /* update IMU data in 50Hz */
        int64_t update_interval = 20;
        int64_t update_count = 0;
        PowerMode_t power_mode = mode_normal;
    };

    struct BaselineInfo {
        uint16_t tVocBase;
        uint16_t eCo2Base;
        uint8_t burnTime;
        bool isBurned;
        int64_t baseline_start_time;
        int64_t start_time;
    };

    class  SystemTask : public HAL
    {
    private:
        I2CManager& _i2cManager;   
        DatabaseManager& _databaseManager;  // 引用数据库管理器

        RtcData_t _rtc_data;
        PowerInfos_t _power_infos;
        PowerManager_t _power_manager;
        ImuData_t _imu_data;
        SystemData_t _system_data;
        EnvironmentalData _Env_data;
        MaxManager _max_manager;
        BaselineInfo baselineInfo;
        EventGroupHandle_t system_events;


        void putI2cDevicesToSleep();

        void wrist_raise_to_wake();

        void _update_rtc_time();
        void _update_imu_data();
        void _update_power_infos();
        void _update_Env_data();

        void _update_go_sleep();
        void _update_power_mode();
        void _update_ir_control();

        void initializeBaselineData();
        static void baselineCorrectionTask(void *pvParameters);
        void saveBaselineToNVS();

    public:
        SystemTask(I2CManager& i2cManager, DatabaseManager& dbManager)
        :   HAL(i2cManager), 
            _i2cManager(i2cManager),
            _databaseManager(dbManager),
            s_data_t(nullptr), 
            lcd_task_handle(nullptr),
            _custom_wakeup_duration_us(0),      // 初始化新成员
            _is_custom_wakeup_timer_active(false) // 初始化新成员
            {system_events = xEventGroupCreate();}   
        ~SystemTask(){}
        TaskHandle_t s_data_t;  // 存储 SystemTask 任务的句柄
        TaskHandle_t lcd_task_handle;     // 存储 LCD 更新任务的句柄
        // 唤醒定时器
        uint64_t _custom_wakeup_duration_us; // 以微秒为单位的唤醒时长
        bool _is_custom_wakeup_timer_active; // 标记自定义唤醒定时器是否激活

        void systen_manage_task();

        void lcd_update_task();
        
        void setLaminatepie(LAMINATEPIE::DataTime_t);

        // 新增：公共方法来设置和清除自定义唤醒定时器
        /**
         * @brief 设置一个自定义的唤醒定时器。
         * 
         * @param duration_seconds 定时器时长，单位为秒。如果为0，则清除定时器。
         */
        void set_custom_wakeup_alarm(uint32_t duration_seconds);

        /**
         * @brief 清除（禁用）自定义唤醒定时器。
         */
        void clear_custom_wakeup_alarm();

        void wait_lcd_initialized();
        
        // 获取 HAL 实例
        HAL& getHAL() {return *this;}

        // 获取 I2CManager 实例
        I2CManager& getI2CManager() {return i2cManager;  }
        
        TaskHandle_t get_sm_handle() {return s_data_t;}
        TaskHandle_t get_lcd_handle() {return lcd_task_handle;}
        
        inline SIMPLEKV::SimpleKV_ESP *getDatabase() { return _databaseManager.getDatabase(); }
        inline SIMPLEKV::SimpleKV_ESP_NVS *getDatabase_nvs() { return _databaseManager.getDatabase_nvs(); }
    };

}


