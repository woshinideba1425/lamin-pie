## ESP Brookesia 事件系统架构

### 1. **事件类型定义**


Read file: core/brookesia_core/systems/core/esp_brookesia_core_event.hpp

Read file: core/brookesia_core/systems/core/esp_brookesia_core.hpp



Read file: core/brookesia_core/systems/core/esp_brookesia_core_manager.hpp
现在让我查看事件处理的具体实现：


Read file: core/brookesia_core/systems/core/esp_brookesia_core_event.cpp

Read file: core/brookesia_core/systems/core/esp_brookesia_core_manager.cpp

Read file: core/brookesia_core/systems/core/esp_brookesia_core_manager.cpp
现在让我查看应用启动器如何发送事件：


Read file: core/brookesia_core/systems/speaker/widgets/app_launcher/esp_brookesia_app_launcher_icon.cpp
## ESP Brookesia 事件系统总结

### 1. **事件类型层次结构**

#### **核心事件类型**：
```cpp
// 1. 系统级事件 (ESP_Brookesia_CoreEvent)
enum class ID {
    APP,           // 应用事件
    STYLESHEET,    // 样式表事件
    NAVIGATION,    // 导航事件
    CUSTOM,        // 自定义事件
};

// 2. 应用事件类型
typedef enum {
    ESP_BROOKESIA_CORE_APP_EVENT_TYPE_START = 0,    // 启动应用
    ESP_BROOKESIA_CORE_APP_EVENT_TYPE_STOP,         // 停止应用
    ESP_BROOKESIA_CORE_APP_EVENT_TYPE_OPERATION,    // 应用操作
    ESP_BROOKESIA_CORE_APP_EVENT_TYPE_MAX,
} ESP_Brookesia_CoreAppEventType_t;

// 3. 导航事件类型
typedef enum {
    ESP_BROOKESIA_CORE_NAVIGATE_TYPE_BACK,          // 返回
    ESP_BROOKESIA_CORE_NAVIGATE_TYPE_HOME,          // 主页
    ESP_BROOKESIA_CORE_NAVIGATE_TYPE_RECENTS_SCREEN, // 最近应用
    ESP_BROOKESIA_CORE_NAVIGATE_TYPE_MAX,
} ESP_Brookesia_CoreNavigateType_t;
```

### 2. **事件数据结构**

#### **应用事件数据**：
```cpp
typedef struct {
    int id;                                    // 应用ID
    ESP_Brookesia_CoreAppEventType_t type;    // 事件类型
    void *data;                               // 事件数据
} ESP_Brookesia_CoreAppEventData_t;

// 核心事件数据
struct HandlerData {
    ID id;           // 事件ID
    void *object;    // 事件对象
    void *param;     // 事件参数
    void *user_data; // 用户数据
};
```

### 3. **事件传递机制**

#### **事件注册**：
```cpp
// 注册应用事件回调
bool ESP_Brookesia_Core::registerAppEventCallback(lv_event_cb_t callback, void *user_data) const
{
    return lv_obj_add_event_cb(_event_obj.get(), callback, _app_event_code, user_data);
}

// 注册导航事件回调
bool ESP_Brookesia_Core::registerNavigateEventCallback(lv_event_cb_t callback, void *user_data) const
{
    return lv_obj_add_event_cb(_event_obj.get(), callback, _navigate_event_code, user_data);
}
```

#### **事件发送**：
```cpp
// 发送应用事件
bool ESP_Brookesia_Core::sendAppEvent(const ESP_Brookesia_CoreAppEventData_t *data) const
{
    return lv_obj_send_event(_event_obj.get(), _app_event_code, (void *)data) == LV_RES_OK;
}

// 发送导航事件
bool ESP_Brookesia_Core::sendNavigateEvent(ESP_Brookesia_CoreNavigateType_t type) const
{
    return lv_obj_send_event(_event_obj.get(), _navigate_event_code, (void *)type) == LV_RES_OK;
}
```

### 4. **事件处理流程**

#### **应用启动事件流程**：

1. **用户交互触发**：
```cpp
// 用户点击应用图标
void AppLauncherIcon::onIconTouchEventCallback(lv_event_t *event)
{
    case LV_EVENT_CLICKED:
        ESP_Brookesia_CoreAppEventData_t app_event_data = {
            .id = icon->_info.id,
            .type = ESP_BROOKESIA_CORE_APP_EVENT_TYPE_START,
        };
        icon->_core.sendAppEvent(&app_event_data);  // 发送应用启动事件
        break;
}
```

2. **事件发送**：
```cpp
// 通过LVGL事件系统发送
bool ESP_Brookesia_Core::sendAppEvent(const ESP_Brookesia_CoreAppEventData_t *data) const
{
    return lv_obj_send_event(_event_obj.get(), _app_event_code, (void *)data) == LV_RES_OK;
}
```

3. **事件处理**：
```cpp
// 应用管理器处理事件
void ESP_Brookesia_CoreManager::onAppEventCallback(lv_event_t *event)
{
    ESP_Brookesia_CoreAppEventData_t *event_data = (ESP_Brookesia_CoreAppEventData_t *)lv_event_get_param(event);
    
    switch (event_data->type) {
    case ESP_BROOKESIA_CORE_APP_EVENT_TYPE_START:
        manager->startApp(id);  // 启动应用
        break;
    case ESP_BROOKESIA_CORE_APP_EVENT_TYPE_STOP:
        manager->processAppClose(app);  // 关闭应用
        break;
    }
}
```

### 5. **事件系统架构**

#### **多层事件系统**：

1. **LVGL事件层**：
   - 基于LVGL的事件循环
   - 处理UI事件（触摸、点击等）
   - 在LVGL线程中执行

2. **核心事件层**：
   - `ESP_Brookesia_CoreEvent`：通用事件系统
   - 支持对象-事件-处理器映射
   - 支持多个处理器注册

3. **应用事件层**：
   - 应用生命周期事件
   - 导航事件
   - 数据更新事件

#### **事件处理器注册**：
```cpp
// 在应用管理器初始化时注册事件处理器
bool ESP_Brookesia_CoreManager::beginCore(void)
{
    // 注册应用事件回调
    _core.registerAppEventCallback(onAppEventCallback, this);
    // 注册导航事件回调
    _core.registerNavigateEventCallback(onNavigationEventCallback, this);
}
```

### 6. **事件处理特点**

#### **同步处理**：
- 所有事件在LVGL线程中同步处理
- 避免了多线程竞争问题
- 确保事件处理的原子性

#### **事件优先级**：
- 触摸事件优先处理
- 应用事件按顺序处理
- 系统事件最后处理

#### **错误处理**：
```cpp
// 事件处理错误检查
if (!handler(data)) {
    ret = false;
    ESP_UTILS_LOGE("Do handler failed");
}
```

### 7. **事件系统优势**

#### **解耦合**：
- 事件发送者和处理者解耦
- 支持多个处理器注册
- 支持动态注册/注销

#### **可扩展性**：
- 支持自定义事件类型
- 支持事件参数传递
- 支持事件处理器链

#### **线程安全**：
- 单线程事件处理
- 避免了复杂的同步机制
- 提高了系统稳定性

### 8. **完整事件流程示例**

```
用户点击应用图标
    ↓
LVGL触摸事件 (LV_EVENT_CLICKED)
    ↓
AppLauncherIcon::onIconTouchEventCallback()
    ↓
创建应用事件数据 (ESP_Brookesia_CoreAppEventData_t)
    ↓
调用 core.sendAppEvent()
    ↓
LVGL事件系统发送事件
    ↓
ESP_Brookesia_CoreManager::onAppEventCallback()
    ↓
根据事件类型调用相应处理函数
    ↓
应用启动/停止/操作
```

这个事件系统设计简洁高效，通过LVGL事件系统实现了统一的事件处理机制，确保了系统的稳定性和可维护性。
