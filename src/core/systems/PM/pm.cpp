#include "pm.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include <atomic>
#include "driver/timer.h"
#include "esp_private/esp_clk.h"

#define MHZ (1000000)

namespace LAMINATEPIE {

    PMSystem::PMSystem(int max_freq_mhz, int min_freq_mhz, bool light_sleep_enable, esp_pm_lock_type_t lock_type)
        : _max_freq_mhz(max_freq_mhz), _min_freq_mhz(min_freq_mhz), _light_sleep_enable(light_sleep_enable) {
        
        // 配置电源管理参数
        esp_pm_config_t pm_config = {
            .max_freq_mhz = max_freq_mhz,
            .min_freq_mhz = min_freq_mhz,
            .light_sleep_enable = light_sleep_enable
        };
        
        esp_err_t ret = esp_pm_configure(&pm_config);
        if (ret == ESP_OK) {
            ESP_LOGI("PMSystem", "Power management configured successfully with max_freq: %d, min_freq: %d",
                     max_freq_mhz, min_freq_mhz);
        } else {
            ESP_LOGE("PMSystem", "Failed to configure power management: %s", esp_err_to_name(ret));
        }

        timer_config_t config;
        config.alarm_en = TIMER_ALARM_EN;
        config.counter_en = TIMER_PAUSE;
        config.intr_type = TIMER_INTR_LEVEL;
        config.counter_dir = TIMER_COUNT_UP;
        config.auto_reload = TIMER_AUTORELOAD_EN;
        config.clk_src = TIMER_SRC_CLK_APB;
        config.divider = 80;

        timer_init(TIMER_GROUP_0, TIMER_0, &config);
        timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
        timer_start(TIMER_GROUP_0, TIMER_0);

        // 创建并获取锁
        _cpu_lock = std::make_unique<CPULock>(lock_type, "app_cpu_lock");
    }

    void PMSystem::setPowerMode(PowerMode_t mode) {

        //_cpu_lock.reset();

        esp_pm_config_t pm_config;
        esp_err_t ret;
        int xtal_freq_mhz = esp_clk_xtal_freq() / MHZ;

        if (mode == MODE_HIGH_PERFORMANCE) {
            // 设置高性能模式的频率配置，240 MHz为最大频率
            pm_config = {
                .max_freq_mhz = 240,
                .min_freq_mhz = std::min(xtal_freq_mhz, 240),
                .light_sleep_enable = false
            };
            ret = esp_pm_configure(&pm_config);
            if (ret == ESP_OK) {
                ESP_LOGI("PMSystem", "Configured for high performance mode with max CPU frequency at 240 MHz.");
            } else {
                ESP_LOGE("PMSystem", "Failed to set high performance mode: %s", esp_err_to_name(ret));
            }

        } else if (mode == MODE_LOW_POWER) {
            // 设置低功耗模式的频率配置，80 MHz为最大频率
            pm_config = {
                .max_freq_mhz = 80,
                .min_freq_mhz = std::min(80, xtal_freq_mhz),
                .light_sleep_enable = false
            };
            ret = esp_pm_configure(&pm_config);
            if (ret == ESP_OK) {
                ESP_LOGI("PMSystem", "Configured for low power mode with max CPU frequency at 80 MHz.");
            } else {
                ESP_LOGE("PMSystem", "Failed to set low power mode: %s", esp_err_to_name(ret));
            }
        }
    }


    void PMSystem::enableLowPowerMode()
    {
        setPowerMode(MODE_LOW_POWER);
    }

    void PMSystem::enableHignPowerMode()
    {
        setPowerMode(MODE_HIGH_PERFORMANCE);
    }

    void PMSystem::estimate_cpu_frequency_by_tick() {
        ESP_LOGI("CPU_FREQ_MONITOR", "CPU Frequency: %d MHz",  esp_clk_cpu_freq() / MHZ);
    }
} // namespace LAMINATEPIE
