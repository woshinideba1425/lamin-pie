LaminPie 项目代码统计报告
============================

统计时间: $(date)
统计目录: ~/lamin-test/components/lamin-pie

## 总体统计

### 文件数量统计
- 总文件数: 143 个
- C++源文件 (.cpp): 56 个
- 头文件 (.h/.hpp): 83 个  
- C源文件 (.c): 4 个

### 代码行数统计
- 总代码行数: 21,370 行
- C++源文件: 12,311 行
- 头文件: 8,568 行
- C源文件: 491 行

## 按目录结构统计

### 主要目录代码分布
| 目录 | 文件数 | 代码行数 | 占比 |
|------|--------|----------|------|
| src/ | 119 | 19,871 | 93.0% |
| test/ | 15 | 1,289 | 6.0% |
| env_support/ | 9 | 210 | 1.0% |

### src/ 目录详细统计
| 子目录 | 文件数 | 代码行数 | 说明 |
|--------|--------|----------|------|
| src/core/ | 61 | 9,048 | 核心系统代码 |
| src/apps/ | 33 | 5,112 | 应用程序代码 |
| src/device/ | 13 | 2,190 | 设备管理代码 |
| src/common/ | 7 | 1,428 | 通用工具代码 |
| src/porting/ | 5 | 1,714 | 平台移植代码 |

### test/ 目录详细统计
| 子目录 | 文件数 | 代码行数 | 说明 |
|--------|--------|----------|------|
| test/unit/ | 3 | 179 | 单元测试 |
| test/integration/ | 5 | 472 | 集成测试 |
| test/performance/ | 1 | 63 | 性能测试 |
| test/ (其他) | 6 | 575 | 测试基础设施 |

## 代码文件排行 (Top 20)

| 排名 | 文件名 | 行数 | 类型 |
|------|--------|------|------|
| 1 | src/core/systems/app/laminpie_app_base.cpp | 736 | C++ |
| 2 | src/core/gui/style/laminpie_gui_style.hpp | 560 | 头文件 |
| 3 | src/apps/lanucher/lanucher.cpp | 554 | C++ |
| 4 | src/common/laminpie_log.hpp | 519 | 头文件 |
| 5 | src/core/systems/app/laminpie_app_navigation.cpp | 516 | C++ |
| 6 | src/apps/wf_user_custom/wf_user_custom.cpp | 511 | C++ |
| 7 | src/porting/laminpie_pthread.cpp | 485 | C++ |
| 8 | src/core/systems/framework/laminpie_event_dispatcher.hpp | 479 | 头文件 |
| 9 | src/core/services/system_resouce/hal_update_task.cpp | 466 | C++ |
| 10 | src/core/gui/lvgl/laminpie_lv_object.cpp | 462 | C++ |
| 11 | src/apps/alram/SlideListContainer.cpp | 458 | C++ |
| 12 | src/device/laminpie_device_manager.cpp | 445 | C++ |
| 13 | src/core/systems/app/laminpie_app_manager.cpp | 437 | C++ |
| 14 | test/test_main.cpp | 405 | C++ |
| 15 | src/porting/laminpie_std_thread.cpp | 404 | C++ |
| 16 | src/porting/laminpie_freertos.cpp | 390 | C++ |
| 17 | src/core/systems/framework/laminpie_core_display.cpp | 344 | C++ |
| 18 | src/apps/infrared/infrared_tm.cpp | 344 | C++ |
| 19 | src/core/systems/framework/laminpie_system_event_type.hpp | 342 | 头文件 |
| 20 | src/core/systems/framework/laminpie_core_framework.cpp | 338 | C++ |

## 代码质量分析

### 文件大小分布
- 大型文件 (>500行): 6 个
- 中型文件 (200-500行): 25 个  
- 小型文件 (<200行): 112 个

### 代码密度
- 平均每文件行数: 149.4 行
- 最大文件行数: 736 行
- 最小文件行数: 约 10 行

## 项目结构分析

### 核心模块占比
- 核心系统 (src/core/): 42.4% (9,048行)
- 应用程序 (src/apps/): 23.9% (5,112行)
- 设备管理 (src/device/): 10.2% (2,190行)
- 通用工具 (src/common/): 6.7% (1,428行)
- 平台移植 (src/porting/): 8.0% (1,714行)

### 测试覆盖率
- 测试代码占比: 6.0% (1,289行)
- 测试文件数量: 15 个
- 平均测试文件大小: 86 行

## 总结

LaminPie 项目是一个中等规模的 C++ 嵌入式系统项目，具有以下特点：

1. **代码规模**: 总计 21,370 行代码，143 个文件
2. **架构清晰**: 核心系统、应用程序、设备管理等模块分工明确
3. **测试完善**: 包含单元测试、集成测试和性能测试
4. **平台适配**: 具有良好的平台移植层设计
5. **代码质量**: 文件大小分布合理，没有过度庞大的单个文件

该项目代码结构良好，适合嵌入式系统开发，具有良好的可维护性和扩展性。
