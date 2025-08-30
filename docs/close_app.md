# SPEAKER 实例返回 App Launcher 的完整流程

## 1. 系统架构概览

SPEAKER 系统采用分层架构来管理应用的生命周期：

```
┌─────────────────────────────────────────────────────────────┐
│                    Core Manager 层                          │
│             负责应用的状态管理和切换                        │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Core Display 层                          │
│             负责显示管理和屏幕切换                          │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                  Speaker Display 层                         │
│           继承自 Core Display，处理 SPEAKER 特定逻辑        │
└─────────────────────────────────────────────────────────────┘
```

## 2. 返回 App Launcher 的完整流程

### 2.1 触发机制

用户通过以下方式触发返回操作：

1. **触摸手势**：从屏幕左/右边缘水平滑动
2. **按键操作**：按返回键
3. **应用调用**：应用内部调用 `notifyCoreClosed()`

### 2.2 手势检测流程

#### 手势检测系统
```cpp
// 在 core/brookesia_core/systems/phone/widgets/gesture/esp_brookesia_gesture.cpp
void ESP_Brookesia_Gesture::onTouchDetectTimerCallback(struct _lv_timer_t *t)
{
    // 1. 检测触摸点位置
    touched = gesture->readTouchPoint(info.stop_x, info.stop_y);
    
    // 2. 判断触摸区域（左边缘、右边缘等）
    info.start_area |= (info.start_x < data.threshold.horizontal_edge) ? ESP_BROOKESIA_GESTURE_AREA_LEFT_EDGE : 0;
    info.start_area |= ((display_w - info.start_x) < data.threshold.horizontal_edge) ? ESP_BROOKESIA_GESTURE_AREA_RIGHT_EDGE : 0;
    
    // 3. 计算手势方向（水平方向）
    if (distance_x > data.threshold.direction_horizon) {
        info.direction = ESP_BROOKESIA_GESTURE_DIR_RIGHT;
    } else if (distance_x < -data.threshold.direction_horizon) {
        info.direction = ESP_BROOKESIA_GESTURE_DIR_LEFT;
    }
    
    // 4. 发送手势事件
    lv_obj_send_event(gesture->_event_mask_obj.get(), event_code, (void *)&gesture->_event_data);
}
```

#### 手势事件回调注册
```cpp
// 在 core/brookesia_core/systems/phone/esp_brookesia_phone_manager.cpp
lv_obj_add_event_cb(gesture->getEventObj(), onGestureNavigationPressingEventCallback,
                    gesture->getPressingEventCode(), this);
```

### 2.3 手势事件处理

#### Phone Manager 中的手势处理
```cpp
// 在 core/brookesia_core/systems/phone/esp_brookesia_phone_manager.cpp
void ESP_Brookesia_PhoneManager::onGestureNavigationPressingEventCallback(lv_event_t *event)
{
    // 1. 检查是否为"返回"手势
    if ((gesture_info->start_area & (ESP_BROOKESIA_GESTURE_AREA_LEFT_EDGE | ESP_BROOKESIA_GESTURE_AREA_RIGHT_EDGE)) &&
        (gesture_info->direction & ESP_BROOKESIA_GESTURE_DIR_HOR) && manager->_flags.enable_gesture_navigation_back) {
        navigation_type = ESP_BROOKESIA_CORE_NAVIGATE_TYPE_BACK;
    }
    
    // 2. 处理导航事件
    if (navigation_type != ESP_BROOKESIA_CORE_NAVIGATE_TYPE_MAX) {
        ESP_UTILS_CHECK_FALSE_EXIT(manager->processNavigationEvent(navigation_type), "Process navigation event failed");
    }
}
```

#### Speaker Manager 中的手势处理
```cpp
// 在 core/brookesia_core/systems/speaker/esp_brookesia_speaker_manager.cpp
bool Manager::processNavigationGesturePressingEvent(lv_event_t *event)
{
    // 1. 检查是否为"返回"手势
    if ((gesture_info->start_area & (GESTURE_AREA_LEFT_EDGE | GESTURE_AREA_RIGHT_EDGE)) &&
        (gesture_info->direction & GESTURE_DIR_HOR) && _flags.enable_gesture_navigation_back) {
        navigation_type = ESP_BROOKESIA_CORE_NAVIGATE_TYPE_BACK;
    }
    
    // 2. 处理导航事件
    if (navigation_type != ESP_BROOKESIA_CORE_NAVIGATE_TYPE_MAX) {
        ESP_UTILS_CHECK_FALSE_RETURN(processNavigationEvent(navigation_type), false, "Process navigation event failed");
    }
}
```

### 2.4 导航事件处理

#### 调用应用的 back() 方法
```cpp
// 在 core/brookesia_core/systems/phone/esp_brookesia_phone_manager.cpp
bool ESP_Brookesia_PhoneManager::processNavigationEvent(ESP_Brookesia_CoreNavigateType_t type)
{
    switch (type) {
    case ESP_BROOKESIA_CORE_NAVIGATE_TYPE_BACK:
        if (active_app == nullptr) {
            goto end;
        }
        // 调用应用的 back() 方法
        ESP_UTILS_CHECK_FALSE_GOTO(ret = (active_app->back()), end, "App(%d) back failed", active_app->getId());
        break;
    }
}
```

## 3. 应用生命周期管理

### 3.1 应用暂停流程（App Pause）

当应用需要暂停时，会触发以下流程：

```cpp
// 在 Core Manager 中
bool ESP_Brookesia_CoreManager::processAppPause(ESP_Brookesia_CoreApp *app)
{
    ESP_Brookesia_CoreHome &home = _core._core_display;
    
    // 1. 调用应用自身的暂停处理
    ESP_UTILS_CHECK_FALSE_RETURN(app->processPause(), false, "App process pause failed");
    
    // 2. 保存应用快照（如果启用）
    if (_core_data.flags.enable_app_save_snapshot) {
        if (!saveAppSnapshot(app)) {
            ESP_UTILS_LOGE("Save app snapshot failed");
        }
    }
    
    // 3. 调用 Display 层的暂停处理
    ESP_UTILS_CHECK_FALSE_GOTO(home.processAppPause(app), err, "Home process load failed");
    
    // 4. 调用额外的暂停处理
    ESP_UTILS_CHECK_FALSE_GOTO(processAppPauseExtra(app), err, "Process app pause extra failed");
    
    return true;
}
```

### 3.2 应用关闭流程（App Close）

当应用完全关闭时，会触发以下流程：

```cpp
bool ESP_Brookesia_CoreManager::processAppClose(ESP_Brookesia_CoreApp *app)
{
    ESP_Brookesia_CoreHome &home = _core._core_display;
    
    // 1. 调用应用自身的关闭处理
    ESP_UTILS_CHECK_FALSE_RETURN(app->processClose(_active_app == app), false, "App process close failed");
    
    // 2. 释放应用快照
    if (_core_data.flags.enable_app_save_snapshot) {
        if (!releaseAppSnapshot(app)) {
            ESP_UTILS_LOGE("Release app snapshot failed");
        }
    }
    
    // 3. 调用 Display 层的关闭处理 - 关键步骤！
    ESP_UTILS_CHECK_FALSE_RETURN(home.processAppClose(app), false, "Home process close failed");
    
    // 4. 从运行列表中移除应用
    ESP_UTILS_CHECK_FALSE_RETURN(_id_running_app_map.erase(app->_id) > 0, false, "Remove app from running map failed");
    if (_active_app == app) {
        _active_app = nullptr;
    }
    
    return true;
}
```

## 4. Display 层的处理

### 4.1 Speaker Display 的关闭处理

在 Speaker Display 中，`processAppClose` 方法被重写：

```cpp
bool Display::processAppClose(ESP_Brookesia_CoreApp *app)
{
    ESP_UTILS_LOG_TRACE_ENTER_WITH_THIS();
    
    App *speaker_app = static_cast<App *>(app);
    
    ESP_UTILS_CHECK_NULL_RETURN(speaker_app, false, "Invalid speaker app");
    ESP_UTILS_CHECK_FALSE_RETURN(checkInitialized(), false, "Not initialized");
    ESP_UTILS_LOGD("Param: app_id(%d)", speaker_app->getId());
    
    ESP_UTILS_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}
```

### 4.2 主屏幕加载（返回 App Launcher）

最关键的是，在 Core Display 基类中，`processMainScreenLoad` 方法负责加载主屏幕：

```cpp
bool ESP_Brookesia_CoreDisplay::processMainScreenLoad(void)
{
    ESP_UTILS_CHECK_FALSE_RETURN(checkCoreInitialized(), false, "Not initialized");
    ESP_UTILS_CHECK_FALSE_RETURN(_main_screen->isValid(), false, "Invalid main screen");
    
    // 加载主屏幕，即 App Launcher
    lv_scr_load(_main_screen->getNativeHandle());
    
    return true;
}
```

## 5. 完整的调用链

**SPEAKER 实例返回 App Launcher 的完整流程如下：**

```
触摸屏幕左/右边缘
    ↓
手势检测系统检测到水平滑动
    ↓
发送手势事件 (LV_EVENT_PRESSING)
    ↓
Phone/Speaker Manager 的手势回调被触发
    ↓
判断为"返回"手势 (左/右边缘 + 水平方向)
    ↓
设置导航类型为 ESP_BROOKESIA_CORE_NAVIGATE_TYPE_BACK
    ↓
调用 processNavigationEvent(ESP_BROOKESIA_CORE_NAVIGATE_TYPE_BACK)
    ↓
调用当前活跃应用的 back() 方法
    ↓
应用在 back() 中调用 notifyCoreClosed()
    ↓
Core 系统关闭应用并返回 App Launcher
```

### 5.1 详细步骤说明

1. **应用触发返回**：用户按返回键或应用调用 `notifyCoreClosed()`

2. **Core Manager 处理**：
   - 调用 `processAppClose(app)` 关闭当前应用
   - 从运行应用列表中移除应用
   - 清空活动应用指针

3. **Display 层处理**：
   - Speaker Display 的 `processAppClose()` 被调用
   - 处理 SPEAKER 特定的清理逻辑

4. **主屏幕加载**：
   - 调用 `home.processMainScreenLoad()` 或 `processMainScreenLoad()`
   - 加载主屏幕（App Launcher）
   - 使用 `lv_scr_load(_main_screen->getNativeHandle())` 切换到主屏幕

5. **App Launcher 显示**：
   - 主屏幕被加载，显示 App Launcher 界面
   - 用户可以看到所有已安装的应用图标

## 6. 手势触发条件

### 6.1 返回手势的触发条件

**返回手势的触发条件：**
- **起始区域**：屏幕左边缘或右边缘
- **手势方向**：水平方向（左滑或右滑）
- **手势导航功能**：必须启用 (`enable_gesture_navigation_back = true`)

**具体代码：**
```cpp
if ((gesture_info->start_area & (ESP_BROOKESIA_GESTURE_AREA_LEFT_EDGE | ESP_BROOKESIA_GESTURE_AREA_RIGHT_EDGE)) &&
    (gesture_info->direction & ESP_BROOKESIA_GESTURE_DIR_HOR) && manager->_flags.enable_gesture_navigation_back) {
    navigation_type = ESP_BROOKESIA_CORE_NAVIGATE_TYPE_BACK;
}
```

### 6.2 手势配置参数

手势检测系统使用以下参数来判断手势：

- `horizontal_edge`：边缘检测阈值
- `direction_horizon`：水平方向检测阈值
- `enable_gesture_navigation_back`：手势导航返回功能开关

## 7. 关键设计特点

### 7.1 架构优势

- **分层架构**：Core Manager 负责应用状态，Display 负责显示
- **虚函数机制**：Speaker Display 可以重写特定的处理方法
- **屏幕管理**：使用 LVGL 的 `lv_scr_load()` 进行屏幕切换
- **资源清理**：自动清理应用资源，包括快照保存和释放

### 7.2 系统稳定性

- **错误检查**：每个关键步骤都有错误检查和处理
- **资源管理**：自动管理应用快照和运行状态
- **状态同步**：确保应用状态与显示状态保持一致

## 8. 总结

这种设计确保了 SPEAKER 实例能够优雅地从应用中返回到 App Launcher，同时保持系统的稳定性和资源的合理管理。

**核心要点：**
1. **手势触发**：左/右边缘水平滑动触发返回操作
2. **事件处理**：通过手势事件回调链处理导航请求
3. **应用关闭**：调用应用的 `back()` 方法，然后通过 `notifyCoreClosed()` 通知系统
4. **屏幕切换**：使用 LVGL 的屏幕加载机制切换到 App Launcher
5. **资源清理**：自动清理应用资源和状态，确保系统稳定性

整个流程设计合理，层次分明，确保了用户操作的流畅性和系统运行的稳定性。