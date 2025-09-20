#include "test_common.h"
#include "laminpie_app_manager.hpp"
#include "laminpie_app_base.hpp"
#include <memory>

// Mock应用类定义
class MockApp : public laminpie::system::app::Laminpie_App_Base {
public:
    std::string name;
    MockApp(const std::string& app_name) : name(app_name) {}
    
    // 实现基类虚函数
    bool OnCreate() override {return true;}
    bool OnClose() override {return true;}
    bool OnResume() override {return true;}
    bool OnPause() override {return true;}
    bool OnLoop() override {return true;}
};

class AppSchedulerTest {
private:
    std::unique_ptr<laminpie::system::app::Laminpie_App_Manager> app_manager;
    std::unique_ptr<laminpie::system::framework::Laminpie_Core_Framework> framework;
    
public:
    AppSchedulerTest() {
        // 创建框架和AppManager
        framework = std::make_unique<laminpie::system::framework::Laminpie_Core_Framework>();
        laminpie::system::app::Laminpie_App_ManagerData_t data = {};
        data.app.max_running_num = 5;
        app_manager = std::make_unique<laminpie::system::app::Laminpie_App_Manager>(framework.get(), data);
    }
    
    TestResult test_app_startup() {
        // 测试应用启动
        auto mock_app = std::make_shared<MockApp>("test_app");
        auto result = app_manager->StartApp(mock_app.get());
        
        TEST_ASSERT(result);
        TEST_ASSERT(app_manager->IsAppRunning(mock_app.get()));
        TEST_ASSERT(app_manager->GetForegroundApp() == mock_app.get());
        
        return TestResult::kPass;
    }
    
    TestResult test_app_lifecycle() {
        // 测试应用生命周期
        auto mock_app = std::make_shared<MockApp>("lifecycle_app");
        
        // 启动应用
        TEST_ASSERT(app_manager->StartApp(mock_app.get()));
        TEST_ASSERT(app_manager->IsAppRunning(mock_app.get()));
        
        // 暂停应用
        TEST_ASSERT(app_manager->PauseApp(mock_app.get()));
        
        // 恢复应用
        TEST_ASSERT(app_manager->StartApp(mock_app.get()));
        
        // 销毁应用
        TEST_ASSERT(app_manager->DestroyApp(mock_app.get()));
        TEST_ASSERT(!app_manager->IsAppRunning(mock_app.get()));
        
        return TestResult::kPass;
    }
    
    TestResult test_multiple_apps() {
        // 测试多应用管理
        auto app1 = std::make_shared<MockApp>("app1");
        auto app2 = std::make_shared<MockApp>("app2");
        auto app3 = std::make_shared<MockApp>("app3");
        
        // 启动多个应用
        TEST_ASSERT(app_manager->StartApp(app1.get()));
        TEST_ASSERT(app_manager->StartApp(app2.get()));
        TEST_ASSERT(app_manager->StartApp(app3.get()));
        
        // 验证前台应用
        TEST_ASSERT(app_manager->GetForegroundApp() == app3.get());
        
        // 将app2移到前台
        TEST_ASSERT(app_manager->StartApp(app2.get()));
        TEST_ASSERT(app_manager->GetForegroundApp() == app2.get());
        
        return TestResult::kPass;
    }
};