#pragma once
#include "simplekv_esp.hpp"
#include "nvs_flash.h"

namespace SIMPLEKV {

    class SimpleKV_ESP_NVS : public SimpleKV_ESP {
    protected:
        nvs_handle_t _nvs_handle;



        // 将键值对存储到 NVS
        esp_err_t saveToNVS(const char* key, void* value, size_t size) {
            if (size == sizeof(int)) {
                int val = *(int*)value;
                return nvs_set_i32(_nvs_handle, key, val);
            } else if (size == sizeof(float)) {
                float val = *(float*)value;
                return nvs_set_blob(_nvs_handle, key, &val, sizeof(val));
            } else if (size == sizeof(bool)) {
                bool val = *(bool*)value;
                return nvs_set_u8(_nvs_handle, key, (uint8_t)val);
            } else {
                return nvs_set_blob(_nvs_handle, key, value, size); // 保存其他类型的值，如字符串
            }
        }

        esp_err_t loadFromNVS(const char* key, void* out_value, size_t size) {
            if (size == sizeof(int)) {
                return nvs_get_i32(_nvs_handle, key, (int32_t*)out_value);
            } else if (size == sizeof(float)) {
                size_t required_size = sizeof(float);
                return nvs_get_blob(_nvs_handle, key, out_value, &required_size);
            } else if (size == sizeof(bool)) {
                uint8_t val;
                esp_err_t err = nvs_get_u8(_nvs_handle, key, &val);
                *(bool*)out_value = (bool)val;
                return err;
            } else {
                size_t required_size = size;    // 读取其他类型的数据
                return nvs_get_blob(_nvs_handle, key, out_value, &required_size);
            }
        }

    public:
        SimpleKV_ESP_NVS(bool use_psram = false) : SimpleKV_ESP(use_psram) {
            initNVS();  // 初始化 NVS
        }
        
        // 初始化 NVS handle
        bool initNVS(const char* namespace_name = "storage") {
                esp_err_t err = nvs_flash_init();
            if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                ESP_ERROR_CHECK(nvs_flash_erase());
                err = nvs_flash_init();
            }
            // 初始化 NVS 的互斥锁
            _nvs_mutex = xSemaphoreCreateMutex();
            return (nvs_open(namespace_name, NVS_READWRITE, &_nvs_handle) == ESP_OK);
        }
        // 增加 Add 函数，支持 NVS 持久化
        int Add(const char* key, void* value, size_t size, bool persistent = false, TickType_t xTicksToWait = portMAX_DELAY) {
            
            ret_buffer = SimpleKV_ESP::Add(key, value, size);  // 调用基类的 Add 函数
            if (ret_buffer == 0 && persistent) {
                if (xSemaphoreTake(_nvs_mutex, xTicksToWait) == pdTRUE) {
                    saveToNVS(key, value, size);
                    nvs_commit(_nvs_handle);  // 提交到 NVS
                    xSemaphoreGive(_nvs_mutex);
                }
            }
            
            return ret_buffer;
        }

        // 扩展 Put 函数，更新内存并同步到 NVS
        int Put(const char* key, void* value, size_t size, bool persistent = false, TickType_t xTicksToWait = portMAX_DELAY){
            ret_buffer = SimpleKV_ESP::Put(key, value, size, xTicksToWait);
            
            if (ret_buffer == 0 && persistent) {
                if (xSemaphoreTake(_nvs_mutex, xTicksToWait) == pdTRUE) {
                saveToNVS(key, value, size);  
                nvs_commit(_nvs_handle);      
                xSemaphoreGive(_nvs_mutex);
                }  
            }
            
            return ret_buffer;
        }

        // 在获取数据时尝试从 NVS 加载数据
        ValueInfo_t* Get(const char* key, TickType_t xTicksToWait = portMAX_DELAY) {
            ValueInfo_t* val_info = SimpleKV_ESP::Get(key);
            if (val_info && val_info->addr == nullptr) {
                loadFromNVS(key, val_info->addr, val_info->size);// 如果没有在内存中找到，从 NVS 中加载
            }
            return val_info;
        }

        ~SimpleKV_ESP_NVS() {
            nvs_close(_nvs_handle);  // 关闭 NVS handle
        }

        private:
            int ret_buffer;
            SemaphoreHandle_t _nvs_mutex; 
    };


}
