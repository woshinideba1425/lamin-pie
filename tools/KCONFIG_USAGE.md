# LaminPie Kconfig System Usage Guide

## 概述

LaminPie现在支持完整的Kconfig配置系统，可以在Linux环境下使用。这个系统允许你通过配置文件或图形界面来配置LaminPie的各种选项。

## 工具说明

### 1. Kconfig解析器 (`kconfig_parser.py`)
解析Kconfig文件并提取配置选项。

```bash
python3 tools/kconfig_parser.py Kconfig -v
```

### 2. 配置生成器 (`config_generator.py`)
从Kconfig文件和默认配置生成sdkconfig.h。

```bash
python3 tools/config_generator.py Kconfig -d tools/default_config.txt -o test/include/sdkconfig.h -v
```

### 3. Menuconfig包装器 (`menuconfig_wrapper.py`)
提供图形界面配置工具。

```bash
python3 tools/menuconfig_wrapper.py Kconfig -d tools/default_config.txt -o my_config.txt
```

## 使用方法

### 方法1：使用默认配置
直接使用预定义的默认配置：

```bash
cd /home/lubancat/lamin-test/components/lamin-pie
python3 tools/config_generator.py Kconfig -d tools/default_config.txt -o test/include/sdkconfig.h
```

### 方法2：使用图形界面配置
通过menuconfig图形界面配置：

```bash
cd /home/lubancat/lamin-test/components/lamin-pie
python3 tools/menuconfig_wrapper.py Kconfig -d tools/default_config.txt -o my_config.txt
```

### 方法3：集成到CMake构建系统
在CMakeLists.txt中使用：

```cmake
# Include Kconfig support
include(${LAMINPIE_ROOT_DIR}/env_support/cmake/kconfig.cmake)

# Generate configuration from Kconfig
create_kconfig_target(laminpie_config
    KCONFIG_FILE ${LAMINPIE_ROOT_DIR}/Kconfig
    DEFAULTS_FILE ${LAMINPIE_ROOT_DIR}/tools/default_config.txt
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/config
    VERBOSE
)

# Add Kconfig dependency to your target
add_kconfig_dependency(your_target laminpie_config)
```

## 配置文件格式

### 默认配置文件 (`default_config.txt`)
```
# Core Configuration
LAMINPIE_CONF_SKIP=y
LAMINPIE_ENABLE_LOG=y
LAMINPIE_LOG_LEVEL_DEBUG=y

# Core Systems
LAMINPIE_ENABLE_SERVICES=y
LAMINPIE_ENABLE_SYSTEMS=y
LAMINPIE_ENABLE_DEVICE=y
LAMINPIE_ENABLE_GUI=n
```

### 生成的sdkconfig.h
```c
#ifndef SDKCONFIG_H
#define SDKCONFIG_H

#define CONFIG_PLATFORM_GENERIC 1
#define CONFIG_LAMINPIE_ENABLE_LOG 1
#define CONFIG_LAMINPIE_ENABLE_SERVICES 1
// ... 更多配置

#endif // SDKCONFIG_H
```

## 配置选项说明

### 核心配置
- `LAMINPIE_CONF_SKIP`: 跳过laminpie_conf.h文件
- `LAMINPIE_ENABLE_LOG`: 启用日志系统
- `LAMINPIE_LOG_LEVEL_DEBUG`: 设置日志级别为DEBUG

### 系统模块
- `LAMINPIE_ENABLE_SERVICES`: 启用服务模块
- `LAMINPIE_ENABLE_SYSTEMS`: 启用系统模块
- `LAMINPIE_ENABLE_DEVICE`: 启用设备模块
- `LAMINPIE_ENABLE_GUI`: 启用GUI模块
- `LAMINPIE_ENABLE_AI_FRAMEWORK`: 启用AI框架

### 线程配置
- `LAMINPIE_USE_OS_STD_THREAD`: 使用标准C++线程
- `LAMINPIE_THREAD_STACK_SIZE_DEFAULT`: 默认线程栈大小
- `LAMINPIE_THREAD_PRIORITY_DEFAULT`: 默认线程优先级

### 调试配置
- `LAMINPIE_SYSTEM_ENABLE_DEBUG_LOG`: 启用系统调试日志
- `LAMINPIE_SYSTEM_APP_ENABLE_DEBUG_LOG`: 启用应用调试日志
- `LAMINPIE_SYSTEM_EVENT_ENABLE_DEBUG_LOG`: 启用事件调试日志

## 测试Kconfig系统

运行测试脚本验证Kconfig系统：

```bash
cd /home/lubancat/lamin-test/components/lamin-pie
./tools/test_kconfig.sh
```

## 故障排除

### 1. Python3未安装
```bash
sudo apt-get install python3
```

### 2. menuconfig未安装
```bash
sudo apt-get install kconfig-frontends
# 或者使用pip安装
pip install kconfiglib
```

### 3. 配置文件格式错误
确保配置文件使用正确的格式：
- 布尔值：`y` 或 `n`
- 整数值：数字
- 字符串值：用引号包围

### 4. 权限问题
确保脚本有执行权限：
```bash
chmod +x tools/*.py
chmod +x tools/test_kconfig.sh
```

## 扩展Kconfig

要添加新的配置选项，编辑相应的Kconfig文件：

```kconfig
config MY_NEW_OPTION
    bool "Enable my new feature"
    default y
    help
        This enables my new feature.
```

然后在默认配置文件中添加：
```
MY_NEW_OPTION=y
```

## 总结

LaminPie的Kconfig系统现在完全支持Linux环境，提供了：

1. **灵活的配置管理**：通过文件或图形界面配置
2. **自动化构建集成**：与CMake构建系统无缝集成
3. **跨平台兼容**：支持ESP-IDF和通用Linux平台
4. **易于扩展**：可以轻松添加新的配置选项

这个系统让LaminPie的配置管理更加专业和灵活，符合现代嵌入式开发的最佳实践。
