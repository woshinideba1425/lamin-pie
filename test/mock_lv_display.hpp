/**
 * @file mock_lv_display.hpp
 * @brief LVGL显示设备模拟类 - 用于测试环境
 * @author LaminPie Team
 * @date 2024
 */

#pragma once

#include "lvgl.h"
#include "laminpie_log.hpp"
#include <memory>

namespace laminpie::test {

/**
 * @brief LVGL显示设备模拟类
 * 
 * 提供完整的lv_display_t模拟，避免nullptr导致的死锁问题
 */
class MockLvDisplay {
public:
    MockLvDisplay(int32_t hor_res = 800, int32_t ver_res = 600) 
        : _hor_res(hor_res), _ver_res(ver_res) {
        InitializeMockDisplay();
    }
    
    ~MockLvDisplay() {
        CleanupMockDisplay();
    }
    
    // 禁用拷贝构造和赋值
    MockLvDisplay(const MockLvDisplay&) = delete;
    MockLvDisplay& operator=(const MockLvDisplay&) = delete;
    
    /**
     * @brief 获取模拟的显示设备指针
     * @return lv_display_t* 模拟的显示设备指针，如果初始化失败返回nullptr
     */
    lv_display_t* GetDisplay() const { 
        return _display; 
    }
    
    /**
     * @brief 检查显示设备是否有效
     * @return bool true表示有效，false表示无效
     */
    bool IsValid() const { 
        return _display != nullptr; 
    }
    
    /**
     * @brief 获取最后刷新的缓冲区指针（用于测试验证）
     * @return uint8_t* 最后刷新的缓冲区指针
     */
    uint8_t* GetLastFlushedBuffer() const { 
        return _last_flushed_buf; 
    }
    
    /**
     * @brief 获取刷新次数（用于测试验证）
     * @return uint32_t 刷新次数
     */
    uint32_t GetFlushCount() const { 
        return _flush_count; 
    }
    
    /**
     * @brief 重置刷新计数器
     */
    void ResetFlushCount() { 
        _flush_count = 0; 
    }

private:
    void InitializeMockDisplay() {
        LOGI( "Initializing mock LVGL display: %dx%d", _hor_res, _ver_res);
        
        // 初始化LVGL（如果尚未初始化）
        static bool lvgl_initialized = false;
        if (!lvgl_initialized) {
            lv_init();
            lvgl_initialized = true;
            LOGI( "LVGL initialized for testing");
        }
        
        // 创建测试帧缓冲区
        size_t buffer_size = (_hor_res + LV_DRAW_BUF_STRIDE_ALIGN - 1) * _ver_res + LV_DRAW_BUF_ALIGN;
        _test_fb = std::make_unique<lv_color32_t[]>(buffer_size);
        
        if (!_test_fb) {
            LOGE("Failed to allocate test frame buffer");
            return;
        }
        
        // 创建显示设备
        _display = lv_display_create(_hor_res, _ver_res);
        if (!_display) {
            LOGE("Failed to create LVGL display");
            return;
        }
        
        // 设置缓冲区
        lv_display_set_buffers(_display, 
                              lv_draw_buf_align(_test_fb.get(), LV_COLOR_FORMAT_ARGB8888), 
                              nullptr,  // 单缓冲模式
                              _hor_res * _ver_res * 4, 
                              LV_DISPLAY_RENDER_MODE_DIRECT);
        
        // 设置模拟刷新回调
        lv_display_set_flush_cb(_display, MockFlushCallback);
        
        // 设置用户数据指针，用于回调函数访问实例
        lv_display_set_user_data(_display, this);
        
        LOGI( "Mock display created successfully");
    }
    
    void CleanupMockDisplay() {
        if (_display) {
            LOGI( "Cleaning up mock display");
            lv_display_delete(_display);
            _display = nullptr;
        }
        _test_fb.reset();
    }
    
    /**
     * @brief 模拟刷新回调函数
     * @param disp 显示设备指针
     * @param area 刷新区域
     * @param color_p 颜色数据指针
     */
    static void MockFlushCallback(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p) {
        // 获取用户数据（MockLvDisplay实例）
        MockLvDisplay* mock_display = static_cast<MockLvDisplay*>(lv_display_get_user_data(disp));
        
        if (mock_display) {
            // 记录刷新信息
            mock_display->_last_flushed_buf = color_p;
            mock_display->_flush_count++;
            
            LOGD("Mock flush: area(%d,%d,%d,%d), count=%u", 
                        area->x1, area->y1, area->x2, area->y2, mock_display->_flush_count);
        }
        
        // 模拟刷新完成
        lv_display_flush_ready(disp);
    }
    
private:
    int32_t _hor_res;
    int32_t _ver_res;
    lv_display_t* _display = nullptr;
    std::unique_ptr<lv_color32_t[]> _test_fb;
    
    // 测试验证用的数据
    mutable uint8_t* _last_flushed_buf = nullptr;
    mutable uint32_t _flush_count = 0;
};

/**
 * @brief MockLvDisplay的智能指针类型
 */
using MockLvDisplayPtr = std::unique_ptr<MockLvDisplay>;

} // namespace laminpie::test



