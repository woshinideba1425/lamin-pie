/**
 * @file test_app_manager_integration_gtest.cpp
 * @brief 应用管理器集成测试 - Google Test版本
 * @author LaminPie Team
 * @date 2024
 */

#include "test_common_gtest.h"
#include "test_platform.h"
#include "minimal_test_app.hpp"
#include "laminpie_app_manager.hpp"
#include "laminpie_core_framework.hpp"
#include "laminpie_event_dispatcher.hpp"
#include "laminpie_app_navigation.hpp"
#include "laminpie_core_display.hpp"
#include "laminpie_device_manager.h"
#include <memory>
#include <cassert>
#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>

using namespace laminpie::system::app;
using namespace laminpie::system::framework;
using namespace laminpie::system::event;
using namespace laminpie::device;
using namespace laminpie::test;

DeviceManager::DeviceManager() {
}

DeviceManager::~DeviceManager() {

}

// 测试应用数据定义
Laminpie_App_Base_Data_t MinimalTestApp1Data = {
    .name = "MinimalTestApp1",
    .launcher_icon = {.resource = nullptr, .recolor = {.color = 0xFFFFFF, .opacity = 255}, .flags = {.enable_recolor = false}},
    .screen_size = {.width = 100, .height = 100},
    .flags = {.enable_default_screen = true, .enable_recycle_resource = true, .enable_resize_visual_area = true, .enable_running_bg = true},
    .app_proity = 1,
};

Laminpie_App_Base_Data_t MinimalTestApp2Data = {
    .name = "MinimalTestApp2",
    .launcher_icon = {.resource = nullptr, .recolor = {.color = 0xFFFFFF, .opacity = 255}, .flags = {.enable_recolor = false}},
    .screen_size = {.width = 100, .height = 100},
    .flags = {.enable_default_screen = true, .enable_recycle_resource = true, .enable_resize_visual_area = true, .enable_running_bg = true},
    .app_proity = 1,
};

Laminpie_App_Base_Data_t MinimalTestApp5Data = {
    .name = "MinimalTestApp5",
    .launcher_icon = {.resource = nullptr, .recolor = {.color = 0xFFFFFF, .opacity = 255}, .flags = {.enable_recolor = false}},
    .screen_size = {.width = 100, .height = 100},
    .flags = {.enable_default_screen = true, .enable_recycle_resource = true, .enable_resize_visual_area = true, .enable_running_bg = false},
    .app_proity = 4,
};

/**
 * @brief 测试用的核心显示类
 * 
 * 继承自Laminpie_CoreDisplay，提供测试环境的最小实现
 */
class TestCoreDisplay : public Laminpie_CoreDisplay {
public:
    TestCoreDisplay(Laminpie_Core_Framework &core, const Laminpie_CoreDisplayData &data) 
        : Laminpie_CoreDisplay(core, data) {
        LOGI("TestCoreDisplay created");
    }
    
    ~TestCoreDisplay() {
        LOGI("TestCoreDisplay destroyed");
    }
    
    bool ProcessAppInstall(Laminpie_App_Base *app) override { return true; }
    bool ProcessAppUninstall(Laminpie_App_Base *app) override { return true; }
    bool ProcessAppCreate(Laminpie_App_Base *app) override { return true; }
};

/**
 * @brief 测试用的核心Home类
 * 
 * 继承自Laminpie_CoreHome，提供测试环境的最小实现
 */
class TestCoreHome : public Laminpie_CoreHome {
public:
    TestCoreHome(Laminpie_Core_Framework &core, const Laminpie_CoreHomeData_t &data) 
        : Laminpie_CoreHome(core, data) {
        LOGI("TestCoreHome created");
    }
    
    virtual ~TestCoreHome() {
        LOGI("TestCoreHome destroyed");
    }

    bool ProcessAppInstall(Laminpie_App_Base *app) override { return true; }
    bool ProcessAppUninstall(Laminpie_App_Base *app) override { return true; }
    bool ProcessAppCreate(Laminpie_App_Base *app) override { return true; }
};

/**
 * @brief Mock LVGL 显示设备类
 */
class MockLvDisplay {
public:
    MockLvDisplay(int32_t hor_res = 100, int32_t ver_res = 100) 
        : _hor_res(hor_res), _ver_res(ver_res) {
        InitializeMockDisplay();
    }
    
    ~MockLvDisplay() {
        if (_display) {
            lv_display_delete(_display);
        }
    }
    
    lv_display_t* GetDisplay() const { return _display; }
    
private:
    void InitializeMockDisplay() {
        // 初始化 LVGL（如果尚未初始化）
        static bool lvgl_initialized = false;
        if (!lvgl_initialized) {
            lv_init();
            lvgl_initialized = true;
        }
        
        // 创建测试帧缓冲区
        lv_color32_t test_fb[(_hor_res + LV_DRAW_BUF_STRIDE_ALIGN - 1) * _ver_res + LV_DRAW_BUF_ALIGN];
        
        // 创建显示设备
        assert(_display = lv_display_create(_hor_res, _ver_res));
        
        // 设置缓冲区
        lv_display_set_buffers(_display, 
                              lv_draw_buf_align(test_fb, LV_COLOR_FORMAT_ARGB8888), 
                              NULL, 
                              _hor_res * _ver_res * 4, 
                              LV_DISPLAY_RENDER_MODE_DIRECT);
        
        // 设置模拟刷新回调
        lv_display_set_flush_cb(_display, MockFlushCallback);
    }
    
    static void MockFlushCallback(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p) {
        LV_UNUSED(area);
        LV_UNUSED(color_p);
        // 模拟刷新完成
        lv_display_flush_ready(disp);
    }
    
    int32_t _hor_res;
    int32_t _ver_res;
    lv_display_t* _display = nullptr;
};

MockLvDisplay _mock_display(100, 100);
/**
 * @brief AppManager集成测试类
 * 
 * 测试AppManager启动和管理最小应用的基本功能
 */
class AppManagerIntegrationTest : public ::testing::Test, public Laminpie_Core_Framework {
public:
    AppManagerIntegrationTest():
            Laminpie_Core_Framework(
            _core_data,
            _test_core_home,
            _app_manager,        // 传递引用，利用C++引用延迟绑定特性
            _app_navigation,
            LaminPie_EventDispatcher::getInstance(),  // 直接传递单例引用
            DeviceManager::getInstance(),
            _mock_display.GetDisplay()  // 模拟显示设备
        ),
        _test_core_home(*this, _core_home_data),   // 实际构造，引用会自动绑定
        _app_manager(*this, _app_manager_data),    // 实际构造，引用会自动绑定
        _app_navigation() {
        // 不再需要 _core_app_manager.reset(&_app_manager);
        // 基类构造函数已经通过引用获得了正确的对象
    }
protected:
    void SetUp() override {
        LOGI("Setting up test environment");
        
        // 初始化核心数据
        _core_data.name = "TestFramework";
        _core_data.screen_size = {100, 100};
        _core_data.manager.app.max_running_num = 5;
        _core_data.manager.flags.enable_app_save_snapshot = false;
        
        // 初始化核心Home数据
        _core_home_data = {};
        
        // 初始化AppManager数据
        _app_manager_data = {};
        _app_manager_data.app.max_running_num = 5;
        _app_manager_data.flags.enable_app_save_snapshot = false;
        
        LOGI("Test environment setup completed");
    }
    
    void TearDown() override {
        LOGI("Cleaning up AppManager integration test");
        
        
        LOGI("AppManager integration test cleanup completed");
    }

protected:
    // 核心数据
    Laminpie_Core_Data_t _core_data;
    Laminpie_CoreHomeData_t _core_home_data;
    Laminpie_App_ManagerData_t _app_manager_data;
    
    // 真实组件实例 - 必须在基类构造函数之前声明
    // 注意：声明顺序决定了初始化顺序，必须与基类构造函数参数顺序一致
    // 基类构造函数参数顺序：data, core_display, core_manager, core_navigation, core_event, core_device_manager, device
    TestCoreHome _test_core_home;        // core_display
    Laminpie_App_Manager _app_manager;  // core_manager
    Laminpie_App_Navigation _app_navigation;  // core_navigation
};

/**
 * @brief 测试基本应用启动功能
 */
TEST_F(AppManagerIntegrationTest, TestBasicAppStart) {
    LOGI("Testing basic app start");
    
    // 创建测试应用
    MinimalTestApp1Data.name = "TestApp1";
    auto test_app = std::make_unique<MinimalTestApp1>(MinimalTestApp1Data);
    
    // 验证应用创建成功
    EXPECT_NE(test_app, nullptr) << "Test app should be created successfully";
    
    // 安装应用
    bool register_result = _app_manager.Install(test_app.get());
    EXPECT_TRUE(register_result) << "App should be installed successfully";
    
    // 启动应用
    bool start_result = _app_manager.StartApp(test_app.get());
    EXPECT_TRUE(start_result) << "App should start successfully";
    
    // 验证应用是否在运行
    bool is_running = _app_manager.IsAppRunning(test_app.get());
    EXPECT_TRUE(is_running) << "App should be running";
    
    // 验证前台应用
    auto foreground_app = _app_manager.GetForegroundApp();
    EXPECT_EQ(foreground_app, test_app.get()) << "Foreground app should be the started app";
    
    // 更新应用管理器以处理状态转换
    _app_manager.Update();
    
    // 验证应用是否已初始化
    EXPECT_TRUE(test_app->IsInitialized()) << "App should be initialized";
    
    LOGI("Basic app start test passed");
}

/**
 * @brief 测试应用状态管理
 */
TEST_F(AppManagerIntegrationTest, TestAppStateManagement) {
    LOGI("Testing app state management");
    
    MinimalTestApp1Data.name = "TestApp2";
    auto test_app = std::make_unique<MinimalTestApp1>(MinimalTestApp1Data);
    
    // 安装应用
    EXPECT_TRUE(_app_manager.Install(test_app.get())) << "App should be installed successfully";
    
    // 启动应用
    EXPECT_TRUE(_app_manager.StartApp(test_app.get())) << "App should start successfully";
    
    // 更新以处理状态转换
    _app_manager.Update();
    
    // 验证应用正在运行
    EXPECT_TRUE(_app_manager.IsAppRunning(test_app.get())) << "App should be running";
    
    // 暂停应用
    EXPECT_TRUE(_app_manager.PauseApp(test_app.get())) << "App should pause successfully";
    
    // 更新以处理暂停状态
    _app_manager.Update();
    
    // 恢复应用
    EXPECT_TRUE(_app_manager.StartApp(test_app.get())) << "App should resume successfully";
    
    // 更新以处理恢复状态
    _app_manager.Update();
    
    // 验证应用仍在运行
    EXPECT_TRUE(_app_manager.IsAppRunning(test_app.get())) << "App should still be running after resume";
    
    LOGI("App state management test passed");
}

/**
 * @brief 测试多应用管理
 */
TEST_F(AppManagerIntegrationTest, TestMultipleAppsManagement) {
    LOGI("Testing multiple apps management");
    
    MinimalTestApp1Data.name = "TestApp3";
    MinimalTestApp2Data.name = "TestApp4";
    auto app1 = std::make_unique<MinimalTestApp1>(MinimalTestApp1Data);
    auto app2 = std::make_unique<MinimalTestApp2>(MinimalTestApp2Data);

    // 安装两个应用
    EXPECT_TRUE(_app_manager.Install(app1.get())) << "App1 should be installed successfully";
    EXPECT_TRUE(_app_manager.Install(app2.get())) << "App2 should be installed successfully";
    
    // 启动第一个应用
    EXPECT_TRUE(_app_manager.StartApp(app1.get())) << "App1 should start successfully";
    
    // 启动第二个应用
    EXPECT_TRUE(_app_manager.StartApp(app2.get())) << "App2 should start successfully";
    
    // 更新以处理状态转换
    _app_manager.Update();
    
    // 验证两个应用都在运行
    EXPECT_TRUE(_app_manager.IsAppRunning(app1.get())) << "App1 should be running";
    EXPECT_TRUE(_app_manager.IsAppRunning(app2.get())) << "App2 should be running";
    
    // 验证前台应用是最后启动的应用
    auto foreground_app = _app_manager.GetForegroundApp();
    EXPECT_EQ(foreground_app, app2.get()) << "Foreground app should be the last started app";
    
    LOGI("Multiple apps management test passed");

    // 不需要手动清理，框架的析构函数会处理
    // _app_manager.DestroyAllApps();
}

/**
 * @brief 测试应用生命周期
 */
TEST_F(AppManagerIntegrationTest, TestAppLifecycle) {
    LOGI("Testing app lifecycle");
    
    auto test_app = std::make_unique<MinimalTestApp1>(MinimalTestApp5Data);
    
    // 安装应用
    EXPECT_TRUE(_app_manager.Install(test_app.get())) << "App should be installed successfully";
    
    // 启动应用
    EXPECT_TRUE(_app_manager.StartApp(test_app.get())) << "App should start successfully";
    
    // 更新以处理创建和恢复状态
    _app_manager.Update();
    
    // 验证应用已初始化
    EXPECT_TRUE(test_app->IsInitialized()) << "App should be initialized";
    
    // 验证应用正在运行
    EXPECT_TRUE(_app_manager.IsAppRunning(test_app.get())) << "App should be running";
    
    // 销毁应用
    EXPECT_TRUE(_app_manager.DestroyApp(test_app.get())) << "App should be destroyed successfully";
    
    // 更新以处理销毁状态
    _app_manager.Update();
    
    // 验证应用不再运行
    EXPECT_FALSE(_app_manager.IsAppRunning(test_app.get())) << "App should not be running after destruction";
    
    LOGI("App lifecycle test passed");
}

/**
 * @brief 测试应用错误处理
 */
TEST_F(AppManagerIntegrationTest, TestAppErrorHandling) {
    LOGI("Testing app error handling");
    
    // 测试空指针应用
    std::shared_ptr<MinimalTestApp1> null_app = nullptr;
    
    // 这些调用应该安全处理空指针
    EXPECT_NO_THROW({
        if (null_app) {
            null_app->OnCreate();
        }
    }) << "Null app initialization should not throw";
    
    EXPECT_NO_THROW({
        if (null_app) {
            null_app->OnLoop();
        }
    }) << "Null app execution should not throw";
    
    EXPECT_NO_THROW({
        if (null_app) {
            null_app->OnClose();
        }
    }) << "Null app cleanup should not throw";
    
    // 测试正常应用的错误处理
    Laminpie_App_Base_Data_t app_data;
    app_data.name = "ErrorTestApp";
    auto test_app = std::make_shared<MinimalTestApp1>(app_data);
    EXPECT_NE(test_app, nullptr) << "Test app should be created successfully";
    
    // 多次调用应该安全
    EXPECT_NO_THROW({
        test_app->OnCreate();
        test_app->OnCreate(); // 重复初始化
    }) << "Repeated initialization should not throw";
    
    EXPECT_NO_THROW({
        test_app->OnLoop();
        test_app->OnLoop(); // 重复运行
    }) << "Repeated execution should not throw";
    
    EXPECT_NO_THROW({
        test_app->OnClose();
        test_app->OnClose(); // 重复清理
    }) << "Repeated cleanup should not throw";
    
    LOGI("App error handling test passed");
}
