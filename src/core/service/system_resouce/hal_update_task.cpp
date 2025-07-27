#include "hal_update_task.h"
#include "esp_sleep.h"
#include "ui.h"
#include "esp_private/rtc_clk.h"

static const char* TAG = "HM";


void ST::SystemTask::_update_rtc_time()
{
    /* If time just set */
    if (*_rtc_data.time_just_set_ptr) {
        
        /* Reset flag */
        *_rtc_data.time_just_set_ptr = false;

        /* Write into RTC data buffer */
        _rtc_data.rtc_time.hour = _rtc_data.time_ptr->hour;
        _rtc_data.rtc_time.min = _rtc_data.time_ptr->min;
        _rtc_data.rtc_time.sec = _rtc_data.time_ptr->sec;
        _rtc_data.rtc_time.year = _rtc_data.time_ptr->year;
        _rtc_data.rtc_time.mon = _rtc_data.time_ptr->mon;
        _rtc_data.rtc_time.mday = _rtc_data.time_ptr->mday;
        _rtc_data.rtc_time.wday = _rtc_data.time_ptr->wday;

        /* Set RTC time */
        bm8563.setTime(_rtc_data.rtc_time);
        

    }
    else {

        /* Read RTC */
        bm8563.getTime(_rtc_data.rtc_time);
        // printf("%02d:%02d:%02d %d-%d-%d-%d\n",
        //     _rtc_data.rtc_time.tm_hour, _rtc_data.rtc_time.tm_min, _rtc_data.rtc_time.tm_sec,
        //     _rtc_data.rtc_time.tm_year, _rtc_data.rtc_time.tm_mon + 1, _rtc_data.rtc_time.tm_mday, _rtc_data.rtc_time.tm_wday);
        // ESP_LOGI(TAG, "update RTC time: %04d-%02d-%02d %02d:%02d:%02d",
        //     _rtc_data.rtc_time.year, _rtc_data.rtc_time.mon, _rtc_data.rtc_time.mday,
        //     _rtc_data.rtc_time.hour, _rtc_data.rtc_time.min, _rtc_data.rtc_time.sec);
        /* Write into database */
        _rtc_data.time_ptr->hour = _rtc_data.rtc_time.hour;
        _rtc_data.time_ptr->min = _rtc_data.rtc_time.min;
        _rtc_data.time_ptr->sec = _rtc_data.rtc_time.sec;
        _rtc_data.time_ptr->year = _rtc_data.rtc_time.year;
        _rtc_data.time_ptr->mon = _rtc_data.rtc_time.mon;
        _rtc_data.time_ptr->mday = _rtc_data.rtc_time.mday;
        _rtc_data.time_ptr->wday = _rtc_data.rtc_time.wday;

    }

    /* Update power infos also */
    _update_power_infos();
}


void ST::SystemTask::_update_imu_data()
{
    /* Read IMU */

    *_imu_data.steps = posture.get_step();
    //ESP_LOGI("MPU6050", "Step Counter: %d", posture.get_step());
}


void ST::SystemTask::_update_power_infos()
{
    *_power_infos.battery_level_ptr = axp2101.batteryLevel();
    *_power_infos.battery_is_charging_ptr = axp2101.isCharging();
}

void ST::SystemTask::_update_Env_data()
{   

    float temperature, pressure, humidity;
    if (bmp280.read_float(&temperature, &pressure, &humidity)) {
        //ESP_LOGI("Main", "Temperature: %.2f C", temperature);
        //ESP_LOGI("Main", "Pressure: %.2f hPa", pressure);
    } else {
        ESP_LOGE("Main", "Failed to read data from BMP280");
    }

    /*更新 SHT30 数据*/
    sht30.updateSHT30();
    float rawTemperature = sht30.getCtemp();
    float rawHumidity = sht30.getHumidity();

    /*更新 sgp30 数据*/
    sgp30.sgp30_ReadData();
    uint16_t co2 = sgp30.getCO2();
    uint16_t tvoc = sgp30.getTVOC();
    //ESP_LOGI("EnvData", "tVoC: %dppb, eCO2: %dppm", tvoc, co2);

    /*更新数据库*/
    *_Env_data.pressure = static_cast<uint32_t>(std::round(pressure));
    *_Env_data.Tempture = static_cast<uint32_t>(std::round(rawTemperature)); 
    *_Env_data.Humidity = static_cast<uint32_t>(std::round(rawHumidity));
    *_Env_data.eCO2 = static_cast<uint16_t>(co2); 
    *_Env_data.tVoC = static_cast<uint16_t>(tvoc);
}


void ST::SystemTask::_update_go_sleep()
{
    /* Check lvgl inactive time */
    if (lv_disp_get_inactive_time(NULL) > _power_manager.auto_sleep_time) {
        _power_manager.power_mode = mode_sleeping;
    }

}
void ST::SystemTask::putI2cDevicesToSleep() {
    ESP_LOGI("SystemTask", "Putting devices to sleep...");

    // 调用每个设备的 sleep 函数
    max30105.sleep();
    bm8563.sleep();
    sht30.sleep();
    posture.sleep();
    bmp280.sleep();
    sgp30.sleep();
    axp2101.sleep();
    touchDevice.sleep();

    ESP_LOGI("SystemTask", "All devices are now in sleep mode.");
}

void ST::SystemTask::wrist_raise_to_wake()
{
    

}

void ST::SystemTask::_update_power_mode()
{       
    const char dt[10] = "pm Task";
    TaskHandle_t th = xTaskGetHandle(dt);

    const char ldt[10] = "lcd Task";
    TaskHandle_t lth = xTaskGetHandle(ldt);


    if (_power_manager.power_mode == mode_sleeping) {
        
        ESP_LOGI(TAG, "going sleep...");
        
        /* Close display */
        this->lcd_screen_on_off(false);
        
        vTaskDelay(200);
        /*close i2c device*/
        putI2cDevicesToSleep();


        /* Setup wakeup pins */
        /* Key Up */
        // gpio_reset_pin(GPIO_NUM_0);
        // gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
        // gpio_sleep_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);
        // gpio_wakeup_enable(GPIO_NUM_0, GPIO_INTR_LOW_LEVEL);


        /* Touch pad */
        gpio_set_direction(GPIO_NUM_12, GPIO_MODE_INPUT);
        gpio_sleep_set_pull_mode(GPIO_NUM_12, GPIO_PULLUP_ONLY);
        gpio_wakeup_enable(GPIO_NUM_12, GPIO_INTR_LOW_LEVEL);
        
        esp_sleep_enable_gpio_wakeup();

        // 新增：检查并启用自定义的 RTC 唤醒定时器
        if (_is_custom_wakeup_timer_active && _custom_wakeup_duration_us > 0) {
            ESP_LOGI(TAG, "Enabling custom RTC timer wakeup for %llu us.", _custom_wakeup_duration_us);
            esp_sleep_enable_timer_wakeup(_custom_wakeup_duration_us);
        }

        //this->axp2101.powerOff();
        /* Go to sleep :) */
        ESP_LOGI(TAG, "Entering light sleep...");
        vTaskSuspend(th);
        vTaskSuspend(lth);
        esp_light_sleep_start();
        ESP_LOGI(TAG, "Woke up from light sleep."); // 日志记录唤醒事件

        /* ---------------------------------------------------------------- */
        // 新增：检查唤醒原因
        esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
        ESP_LOGI(TAG, "Wakeup cause: %d", wakeup_cause);

        if (wakeup_cause == ESP_SLEEP_WAKEUP_TIMER) {
            ESP_LOGI(TAG, "Woken up by RTC timer.");
            motor.shake(0.8, 2000);
            clear_custom_wakeup_alarm(); 
        } else if (wakeup_cause == ESP_SLEEP_WAKEUP_GPIO) {
            ESP_LOGI(TAG, "Woken up by GPIO.");
            // 如果是GPIO唤醒，并且自定义定时器仍然是激活状态，
            // 你可能希望它在下次睡眠时依然生效，或者清除它。
            // 当前逻辑下，如果定时器未到期而被GPIO唤醒，下次睡眠时它仍会尝试设置。
        }
        


        /* Wake up o.O */
        _power_manager.power_mode = mode_normal;

        /* Update data at once */
        _update_rtc_time();
        //_update_imu_data();
        /* Clear key pwr */
        ///*unkonw*/axp2101.isKeyPressed();

        *_system_data.just_wake_up_ptr = true;

        gpio_intr_disable(GPIO_NUM_12);

        
        vTaskDelay(10);
        this->lcd_screen_on_off(true);

        /* Reset lvgl inactive time */
        //this->lcd_set_brightness(200);
        gpio_intr_enable(GPIO_NUM_12);
        /* Reset lvgl inactive time */
        lv_disp_trig_activity(NULL);

        this->touchDevice.reset();

        vTaskResume(th);
        vTaskResume(lth);
    }

    if (_power_manager.power_mode == mode_cold_sleeping)
    {
        /*future*/
        this->axp2101.powerOff();
    }
}
    
void ST::SystemTask::systen_manage_task()
{   
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    this->i2c_device_init();
    LAMINATEPIE::DataTime_t timeData;
    setLaminatepie(timeData);
    ESP_LOGI("Laminate", "Framework initialized successfully.");
    vTaskDelay(pdMS_TO_TICKS(1000));
    lv_disp_trig_activity(NULL);

    _update_Env_data();
    _update_rtc_time();

    // 初始化计时变量
    baselineInfo.start_time = esp_timer_get_time();
    baselineInfo.burnTime = 0;
    baselineInfo.isBurned = false;

    // 初始化基线数据
    initializeBaselineData();

    // 创建基线校正任务
    xTaskCreate(&baselineCorrectionTask, "baselineCorrectionTask", 4096, this, 10, NULL);
    unselect(ENV_ID);
    while (1)
    {   
        int64_t current_time = esp_timer_get_time();

        if ((current_time - _rtc_data.update_count) > _rtc_data.update_interval) {
            _update_rtc_time();
            _rtc_data.update_count = current_time;
        }

        if((current_time - _imu_data.update_count) > _imu_data.update_interval){
            _update_imu_data();
            _imu_data.update_count = current_time;
        }

        // 更新电源控制
        _update_go_sleep();
        _update_power_mode();

        // 每秒更新环境数据
        if ((current_time - baselineInfo.start_time) > 1LL * 1000000LL && ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(150))) {
            _update_Env_data();
            baselineInfo.start_time = current_time;
        }

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void ST::SystemTask::wait_lcd_initialized()
{
    ESP_LOGI("SystemTask", "Waiting for LCD initialization...");
    xEventGroupWaitBits(system_events, LCD_INITIALIZED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    ESP_LOGI("SystemTask", "LCD initialization notification received");
}

void ST::SystemTask::lcd_update_task()
{
    //this->i2c_init();
    this->_lcd_init();
    if((bool*)getDatabase_nvs()->Get(LA_SYSTEM_DEVELOPMENT)){
        //this->_lcd_verbose();
    }
    xTaskNotifyGive(s_data_t);
    xEventGroupSetBits(system_events, LCD_INITIALIZED_BIT);
    ESP_LOGI(TAG, "Starting LVGL task");
    this->_lcd_ui();
    ESP_LOGI(TAG, "UI init");
    uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
    while (1) {
        // Lock the mutex due to the LVGL APIs are not thread-safe
        if (lvgl_lock(0)) {
            task_delay_ms = lv_timer_handler();
            // Release the mutex
            ESP_LOGD(TAG, "unlock LVGL");
            lvgl_unlock();
        }
        if (task_delay_ms > LVGL_TASK_MAX_DELAY_MS) {
            task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = LVGL_TASK_MIN_DELAY_MS;
        }
        ESP_LOGD(TAG, "task_delay_ms:%d", task_delay_ms);
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }

}



void ST::SystemTask::setLaminatepie(LAMINATEPIE::DataTime_t)
{

    /* Get data's pointer in database */

    /* Time */
    _rtc_data.time_ptr = (LAMINATEPIE::DataTime_t*)getDatabase()->Get(LA_TIME)->addr;
    _rtc_data.time_just_set_ptr = (bool*)getDatabase()->Get(LA_TIME_JSUT_SET)->addr;

    /* Power infos */
    _power_infos.battery_level_ptr = (uint8_t*)getDatabase()->Get(LA_BATTERY_LEVEL)->addr;
    _power_infos.battery_is_charging_ptr = (bool*)getDatabase()->Get(LA_BATTERY_IS_CHARGING)->addr;

    /* IMU */
    _imu_data.steps = (uint32_t*)getDatabase()->Get(LA_STEPS)->addr;

    /* System data */
    _system_data.just_wake_up_ptr = (bool*)getDatabase()->Get(LA_JUST_WAKEUP)->addr;

    /*enviromental sensor*/
    _Env_data.Humidity = (uint32_t*)getDatabase()->Get(LA_HUMIDITY)->addr;
    _Env_data.pressure = (uint32_t*)getDatabase()->Get(LA_PRESSURE)->addr;
    _Env_data.Tempture = (int32_t*)getDatabase()->Get(LA_TEMPERATURE)->addr;
    _Env_data.eCO2 = (uint16_t*)getDatabase()->Get(LA_CO2)->addr;
    _Env_data.tVoC = (uint16_t*)getDatabase()->Get(LA_TVOC)->addr;

}

void ST::SystemTask::_update_ir_control(){

}

void ST::SystemTask::initializeBaselineData() {
    // 初始化基线数据
    baselineInfo.tVocBase = 0xFFFF;
    baselineInfo.eCo2Base = 0xFFFF;
    bool loadOk = true;
    nvs_handle nvsReadHandle;
    esp_err_t ret = nvs_open(NVS_BASE_NAME, NVS_READWRITE, &nvsReadHandle);
    ESP_ERROR_CHECK(ret);

    if ((ret = nvs_get_u16(nvsReadHandle, SGP_TVOC_BASE_KEY, &baselineInfo.tVocBase)) != ESP_OK || baselineInfo.tVocBase == 0xFFFF) {
        ESP_LOGW("initializeBaselineData", "Invalid tVocBase, setting default value");
        baselineInfo.tVocBase = 0;
        loadOk = false;
    }
    if ((ret = nvs_get_u16(nvsReadHandle, SGP_ECO2_BASE_KEY, &baselineInfo.eCo2Base)) != ESP_OK || baselineInfo.eCo2Base == 0xFFFF) {
        ESP_LOGW("initializeBaselineData", "Invalid eCo2Base, setting default value");
        baselineInfo.eCo2Base = 0;
        loadOk = false;
    }
    nvs_close(nvsReadHandle);

    if (loadOk) {
        // 如果成功从 NVS 读取有效的基线数据，则设置到 SGP30
        esp_err_t err = sgp30.sgp30_SetBaseline(baselineInfo.tVocBase, baselineInfo.eCo2Base);
        if (err == ESP_OK) {
            baselineInfo.isBurned = true;
            ESP_LOGI("initializeBaselineData", "Baseline set successfully: tVocBase=%u, eCo2Base=%u", baselineInfo.tVocBase, baselineInfo.eCo2Base);
        } else {
            ESP_LOGE("initializeBaselineData", "Failed to set baseline: %s(0x%X)", esp_err_to_name(err), err);
        }
    } else {
        // 如果基线数据无效，初始化为默认值
        ESP_LOGI("initializeBaselineData", "Using default baseline values");
        baselineInfo.isBurned = false;
    }
}

void ST::SystemTask::baselineCorrectionTask(void* pvParameters) {
    ST::SystemTask* self = (ST::SystemTask*)pvParameters;

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(3600 * 1000)); // 等待 1 小时

        self->baselineInfo.burnTime++;

        if (self->baselineInfo.burnTime >= 12) {
            self->baselineInfo.isBurned = true;
        }

        if (self->baselineInfo.isBurned) {
            // 获取基线并保存到 NVS
            if (self->sgp30.sgp30_GetBaseline() == ESP_OK) {
                self->saveBaselineToNVS();
            } else {
                ESP_LOGE("BaselineCorrection", "SGP30 baseline correction failed");
            }
        }
    }
}

void ST::SystemTask::saveBaselineToNVS() {
    baselineInfo.tVocBase = sgp30.getTVOCBaseline();
    baselineInfo.eCo2Base = sgp30.getCO2Baseline();

    if (baselineInfo.tVocBase != 0xFFFF && baselineInfo.eCo2Base != 0xFFFF) {
        nvs_handle nvsHandle;
        esp_err_t err = nvs_open(NVS_BASE_NAME, NVS_READWRITE, &nvsHandle);
        ESP_ERROR_CHECK(err);

        if ((err = nvs_set_u16(nvsHandle, SGP_TVOC_BASE_KEY, baselineInfo.tVocBase)) == ESP_OK) {
            if ((err = nvs_set_u16(nvsHandle, SGP_ECO2_BASE_KEY, baselineInfo.eCo2Base)) == ESP_OK) {
                if ((err = nvs_commit(nvsHandle)) == ESP_OK) {
                    ESP_LOGI("saveBaselineToNVS", "SGP BASELINE CORRECTION INFO SAVED TO NVS");
                } else {
                    ESP_LOGE("saveBaselineToNVS", "Failed to commit NVS: %s(0x%X)", esp_err_to_name(err), err);
                }
            } else {
                ESP_LOGE("saveBaselineToNVS", "Failed to save eCo2Base to NVS: %s(0x%X)", esp_err_to_name(err), err);
            }
        } else {
            ESP_LOGE("saveBaselineToNVS", "Failed to save tVocBase to NVS: %s(0x%X)", esp_err_to_name(err), err);
        }
        nvs_close(nvsHandle);
    } else {
        ESP_LOGE("saveBaselineToNVS", "Invalid baseline values, not saving to NVS");
    }
}

// 新增：实现设置和清除自定义唤醒定时器的方法
void ST::SystemTask::set_custom_wakeup_alarm(uint32_t duration_seconds) {
    if (duration_seconds > 0) {
        _is_custom_wakeup_timer_active = true;
        _custom_wakeup_duration_us = duration_seconds * 1000000;
        ESP_LOGI(TAG, "Custom wakeup alarm set for %u seconds (%llu us).", duration_seconds, _custom_wakeup_duration_us);
    } else {
        // 如果 duration_seconds 为 0，则清除定时器
        clear_custom_wakeup_alarm();
        ESP_LOGI(TAG, "Custom wakeup alarm cleared (duration 0 specified).");
    }
}

void ST::SystemTask::clear_custom_wakeup_alarm() {
    _is_custom_wakeup_timer_active = false;
    ESP_LOGI(TAG, "Custom wakeup alarm has been cleared.");
}
