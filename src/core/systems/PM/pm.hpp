#ifndef LAMINATEPIE_PMSYSTEM_H
#define LAMINATEPIE_PMSYSTEM_H

#include "esp_pm.h"
#include "esp_log.h"
#include <memory>

namespace LAMINATEPIE
{
    enum PowerMode_t {
        MODE_HIGH_PERFORMANCE,
        MODE_LOW_POWER
    };

    class CPULock {
    public:
        CPULock(esp_pm_lock_type_t lock_type, const std::string& name) : _lock_handle(nullptr) {
            esp_err_t ret = esp_pm_lock_create(lock_type, 0, name.c_str(), &_lock_handle);
            if (ret == ESP_OK) {
                esp_pm_lock_acquire(_lock_handle);  // 获取锁
                ESP_LOGI("CPULock", "CPU frequency lock acquired for %s.", name.c_str());
            } else {
                ESP_LOGE("CPULock", "Failed to create CPU frequency lock for %s.", name.c_str());
            }
        }

        CPULock() : _lock_handle(nullptr) {}

        ~CPULock() {
            if (_lock_handle) {
                esp_pm_lock_release(_lock_handle);  // 释放锁
                esp_pm_lock_delete(_lock_handle);   // 删除锁
                ESP_LOGI("CPULock", "CPU frequency lock released.");
            }
        }

        void acquire() {
            if (_lock_handle && !_is_locked) {
                esp_pm_lock_acquire(_lock_handle);
                _is_locked = true;
                ESP_LOGI("CPULock", "CPU frequency lock acquired.");
            }
        }

        // 显式释放锁
        void release() {
            if (_lock_handle && _is_locked) {
                esp_pm_lock_release(_lock_handle);
                _is_locked = false;
                ESP_LOGI("CPULock", "CPU frequency lock released.");
            }
        }

        CPULock(const CPULock&) = delete;
        CPULock& operator=(const CPULock&) = delete;

    private:
        esp_pm_lock_handle_t _lock_handle;
        bool _is_locked;
    };

    class ScopedCPULock {
    public:
        explicit ScopedCPULock(CPULock& lock) : _lock(lock) {
            _lock.acquire(); // 进入作用域时获取锁
        }

        ~ScopedCPULock() {
            _lock.release(); // 离开作用域时自动释放锁
        }

    private:
        CPULock& _lock;
    };

    class PMSystem {
    public:
        PMSystem(int max_freq_mhz, int min_freq_mhz, bool light_sleep_enable, esp_pm_lock_type_t lock_type);
        PMSystem() = delete;

        void setPowerMode(PowerMode_t mode);
        void enableLowPowerMode();
        void enableHignPowerMode();
        static void cpu_freq_estimation_task(void *param);
        void estimate_cpu_frequency_by_tick();

    private:
        int _max_freq_mhz;
        int _min_freq_mhz;
        bool _light_sleep_enable;
        std::unique_ptr<CPULock> _cpu_lock;
    };
}// namespace LAMINATEPIE

#endif // LAMINATEPIE_PMSYSTEM_H