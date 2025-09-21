#include "test_common.h"
#include "minimal_test_app.hpp"
#include "laminpie_app_manager.hpp"
#include "laminpie_core_framework.hpp"
#include "laminpie_event_dispatcher.hpp"
#include "laminpie_app_navigation.hpp"
#include "laminpie_core_display.hpp"
#include "laminpie_device_manager.h"
#include <memory>
#include <cassert>

using namespace laminpie::system::app;
using namespace laminpie::system::framework;
using namespace laminpie::system::event;
using namespace laminpie::device;
using namespace laminpie::test;

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

/**
 * @brief 测试用的核心显示类
 * 
 * 继承自Laminpie_CoreDisplay，提供测试环境的最小实现
 */
class TestCoreDisplay : public Laminpie_CoreDisplay {
public:
    TestCoreDisplay(Laminpie_Core_Framework &core, const Laminpie_CoreDisplayData &data) 
        : Laminpie_CoreDisplay(core, data) {
        SYSTEM_CORE_LOG_INFO("TestCoreDisplay created");
    }
    
    ~TestCoreDisplay() {
        SYSTEM_CORE_LOG_INFO("TestCoreDisplay destroyed");
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
        SYSTEM_CORE_LOG_INFO("TestCoreHome created");
    }
    
    virtual ~TestCoreHome() {
        SYSTEM_CORE_LOG_INFO("TestCoreHome destroyed");
    }

    bool ProcessAppInstall(Laminpie_App_Base *app) override { return true; }
    bool ProcessAppUninstall(Laminpie_App_Base *app) override { return true; }
    bool ProcessAppCreate(Laminpie_App_Base *app) override { return true; }
};


/**
 * @brief AppManager集成测试类
 * 
 * 测试AppManager启动和管理最小应用的基本功能
 */
class AppManagerIntegrationTest : public Laminpie_Core_Framework {
public:
    AppManagerIntegrationTest() : 
        Laminpie_Core_Framework(
            _core_data,
            _test_core_home,
            _app_manager,
            _app_navigation,
            _event_dispatcher,
            DeviceManager::getInstance(),
            nullptr  // 模拟显示设备
        ),
        _test_core_home(*this, _core_home_data),
        _app_manager(*this, _app_manager_data),
        _app_navigation(),
        _event_dispatcher() {
        SYSTEM_APP_LOG_INFO("Starting AppManager integration test");
        SetupTestEnvironment();
    }

    ~AppManagerIntegrationTest() {
        CleanupTestEnvironment();
        SYSTEM_APP_LOG_INFO("AppManager integration test completed");
    }

    TestResult RunAllTests() {
        SYSTEM_APP_LOG_INFO("Running all AppManager integration tests");
        
        // 测试1: 基本启动测试
        TestResult result1 = TestBasicAppStart();
        if (result1 != TestResult::kPass) {
            SYSTEM_APP_LOG_ERROR("TestBasicAppStart failed");
            return result1;
        }
        
        // 测试2: 应用状态管理测试
        TestResult result2 = TestAppStateManagement();
        if (result2 != TestResult::kPass) {
            SYSTEM_APP_LOG_ERROR("TestAppStateManagement failed");
            return result2;
        }
        
        // 测试3: 多应用管理测试
        TestResult result3 = TestMultipleAppsManagement();
        if (result3 != TestResult::kPass) {
            SYSTEM_APP_LOG_ERROR("TestMultipleAppsManagement failed");
            return result3;
        }
        
        // 测试4: 应用生命周期测试
        TestResult result4 = TestAppLifecycle();
        if (result4 != TestResult::kPass) {
            SYSTEM_APP_LOG_ERROR("TestAppLifecycle failed");
            return result4;
        }
        
        SYSTEM_APP_LOG_INFO("All AppManager integration tests passed!");
        return TestResult::kPass;
    }

private:
    void SetupTestEnvironment() {
        SYSTEM_APP_LOG_INFO("Setting up test environment");
        
        // 初始化核心数据
        _core_data.name = "TestFramework";
        _core_data.screen_size = {800, 600};
        _core_data.manager.app.max_running_num = 5;
        _core_data.manager.flags.enable_app_save_snapshot = false;
        
        // 初始化核心Home数据
        _core_home_data = {};
        
        // 初始化AppManager数据
        _app_manager_data = {};
        _app_manager_data.app.max_running_num = 5;
        _app_manager_data.flags.enable_app_save_snapshot = false;
        
        SYSTEM_APP_LOG_INFO("Test environment setup completed");
    }

    void CleanupTestEnvironment() {
        SYSTEM_APP_LOG_INFO("Cleaning up test environment");
        
        // 通过框架清理所有应用
        GetAppManager().DestroyAllApps();
        
        SYSTEM_APP_LOG_INFO("Test environment cleanup completed");
    }

    TestResult TestBasicAppStart() {
        SYSTEM_APP_LOG_INFO("Testing basic app start");
        
        // 创建测试应用
        MinimalTestApp1Data.name = "TestApp1";
        auto test_app = std::make_unique<MinimalTestApp1>(MinimalTestApp1Data);
        
        bool register_result = _app_manager.Install(test_app.get());
        TEST_ASSERT(register_result);
        
        // 启动应用
        bool start_result = _app_manager.StartApp(test_app.get());
        TEST_ASSERT(start_result);
        
        // 验证应用是否在运行
        bool is_running = _app_manager.IsAppRunning(test_app.get());
        TEST_ASSERT(is_running);
        
        // 验证前台应用
        auto foreground_app = _app_manager.GetForegroundApp();
        TEST_ASSERT(foreground_app == test_app.get());
        
        // 更新应用管理器以处理状态转换
        _app_manager.Update();
        
        // 验证应用是否已初始化
        TEST_ASSERT(test_app->IsInitialized());
        
        SYSTEM_APP_LOG_INFO("Basic app start test passed");
        return TestResult::kPass;
    }

    TestResult TestAppStateManagement() {
        SYSTEM_APP_LOG_INFO("Testing app state management");
        
        MinimalTestApp1Data.name = "TestApp2";
        auto test_app = std::make_unique<MinimalTestApp1>(MinimalTestApp1Data);
        
        // 启动应用
        TEST_ASSERT(_app_manager.StartApp(test_app.get()));
        
        // 更新以处理状态转换
        _app_manager.Update();
        
        // 暂停应用
        TEST_ASSERT(_app_manager.PauseApp(test_app.get()));
        
        // 更新以处理暂停状态
        _app_manager.Update();
        
        // 恢复应用
        TEST_ASSERT(_app_manager.StartApp(test_app.get()));
        
        // 更新以处理恢复状态
        _app_manager.Update();
        
        SYSTEM_APP_LOG_INFO("App state management test passed");
        return TestResult::kPass;
    }

    TestResult TestMultipleAppsManagement() {
        SYSTEM_APP_LOG_INFO("Testing multiple apps management");
        
        MinimalTestApp1Data.name = "TestApp3";
        MinimalTestApp2Data.name = "TestApp4";
        auto app1 = std::make_unique<MinimalTestApp1>(MinimalTestApp1Data);
        auto app2 = std::make_unique<MinimalTestApp2>(MinimalTestApp2Data);
        
        // 启动第一个应用
        TEST_ASSERT(_app_manager.StartApp(app1.get()));
        
        // 启动第二个应用
        TEST_ASSERT(_app_manager.StartApp(app2.get()));
        
        // 更新以处理状态转换
        _app_manager.Update();
        
        // 验证两个应用都在运行
        TEST_ASSERT(_app_manager.IsAppRunning(app1.get()));
        TEST_ASSERT(_app_manager.IsAppRunning(app2.get()));
        
        // 验证前台应用是最后启动的应用
        auto foreground_app = _app_manager.GetForegroundApp();
        TEST_ASSERT(foreground_app == app2.get());
        
        SYSTEM_APP_LOG_INFO("Multiple apps management test passed");
        return TestResult::kPass;
    }

    TestResult TestAppLifecycle() {
        SYSTEM_APP_LOG_INFO("Testing app lifecycle");
        
        MinimalTestApp1Data.name = "TestApp5";
        auto test_app = std::make_unique<MinimalTestApp1>(MinimalTestApp1Data);
        
        // 启动应用
        TEST_ASSERT(_app_manager.StartApp(test_app.get()));
        
        // 更新以处理创建和恢复状态
        _app_manager.Update();
        _app_manager.Update();
        
        // 验证应用已初始化
        TEST_ASSERT(test_app->IsInitialized());
        
        // 销毁应用
        TEST_ASSERT(_app_manager.DestroyApp(test_app.get()));
        
        // 更新以处理销毁状态
        _app_manager.Update();
        
        // 验证应用不再运行
        TEST_ASSERT(!_app_manager.IsAppRunning(test_app.get()));
        
        SYSTEM_APP_LOG_INFO("App lifecycle test passed");
        return TestResult::kPass;
    }

private:
    // 核心数据
    Laminpie_Core_Data_t _core_data;
    Laminpie_CoreHomeData_t _core_home_data;
    Laminpie_App_ManagerData_t _app_manager_data;
    
    // 真实组件实例 - 必须在基类构造函数之前声明
    TestCoreHome _test_core_home;
    Laminpie_App_Manager _app_manager;
    Laminpie_App_Navigation _app_navigation;
    LaminPie_EventDispatcher _event_dispatcher;
};

// 测试用例定义
TestCase app_manager_test_cases[] = {
    {"test_app_manager_integration", []() -> TestResult {
        AppManagerIntegrationTest test_suite;
        return test_suite.RunAllTests();
    }, "Test AppManager integration functionality", true}
};

/**
 * @brief 主测试函数
 */
int test_app_manager_integration() {
    LP_LOG_INFO("TEST", "Starting AppManager Integration Test Suite");
    
    test_init();
    
    int passed = 0;
    int total = sizeof(app_manager_test_cases) / sizeof(app_manager_test_cases[0]);
    
    for (int i = 0; i < total; i++) {
        if (app_manager_test_cases[i].enabled) {
            LP_LOG_INFO("TEST", "Running test: %s", app_manager_test_cases[i].name);
            TestResult result = run_test_case(&app_manager_test_cases[i]);
            
            if (result == TestResult::kPass) {
                passed++;
                LP_LOG_INFO("TEST", "Test %s PASSED", app_manager_test_cases[i].name);
            } else {
                LP_LOG_ERROR("TEST", "Test %s FAILED", app_manager_test_cases[i].name);
            }
        }
    }
    
    LP_LOG_INFO("TEST", "AppManager Integration Test Suite completed: %d/%d tests passed", passed, total);
    print_test_results();
    test_cleanup();
    
    return (passed == total) ? 0 : 1;
}
