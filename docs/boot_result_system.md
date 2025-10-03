# BootResult系统设计文档

## 概述

BootResult是基于Rust Result系统设计的C++启动过程错误处理机制，专门用于处理系统启动过程中的错误和依赖关系跟踪。

## 核心特性

### 1. 启动阶段跟踪
- **当前启动阶段记录**：每个BootResult都记录当前处于哪个启动阶段
- **阶段特定错误**：错误信息包含具体的启动阶段信息
- **阶段转换支持**：支持在不同启动阶段之间传递结果

### 2. 依赖模块跟踪
- **自动模块名提取**：基于`Name.hpp`的类型名提取功能自动获取模块名
- **依赖关系记录**：记录启动过程中依赖的各个模块
- **命名空间解析**：从完整类型名中提取命名空间作为模块标识

### 3. 具体函数错误记录
- **函数名记录**：错误发生时记录具体的函数名
- **调用栈信息**：提供详细的错误上下文信息
- **错误代码映射**：每个错误都有唯一的错误代码

## 系统架构

### 错误类型层次

```cpp
laminate::Error (基类)
    └── BootPhaseError (启动阶段专用错误)
```

### BootResult类设计

```cpp
template<typename T>
class BootResult : public laminate::Result<T> {
    // 启动阶段信息
    Laminpie_Boot_Event_Type current_phase_;
    
    // 依赖模块列表
    std::vector<std::string> _related_dependencies;
    
    // 核心方法
    BootResult<T>& SetPhase(Laminpie_Boot_Event_Type phase);
    BootResult<T>& AddDependency(const U& dependency);
    static BootResult<T> CreatePhaseError(...);
    bool IsBootPhaseError() const;
    std::shared_ptr<BootPhaseError> GetBootPhaseError() const;
};
```

## 启动阶段定义

系统定义了以下启动阶段：

```cpp
enum class Laminpie_Boot_Event_Type {
    kBoot_Stage_HardWareInit = 0,      // 硬件初始化
    kBoot_Stage_SytemServiceInit,      // 系统服务初始化
    kBoot_Stage_BSPInit,               // BSP初始化
    kBoot_Stage_DriverInit,            // 驱动程序初始化
    kBoot_Stage_MiddlewareInit,        // 中间件初始化
    kBoot_Stage_Resourceload,          // 资源加载
    kBoot_Stage_AppInit,               // 应用程序初始化
    kBoot_Stage_Complete,              // 启动完成
    kBoot_Stage_Failed,                // 启动失败
    kBoot_Event_Type_Max,
};
```

## 使用方法

### 1. 基本使用

```cpp
// 成功结果
BootResult<bool> result = BootOk<bool>(true, Laminpie_Boot_Event_Type::kBoot_Stage_HardWareInit);

// 错误结果
BootResult<bool> error_result = BootErr<bool>(
    Laminpie_Boot_Event_Type::kBoot_Stage_DriverInit,
    "InitializeI2CDriver",
    "I2C bus initialization failed",
    -3001
);
```

### 2. 依赖关系跟踪

```cpp
BootResult<void> InitializeBSP() {
    auto result = BootOk<void>(Laminpie_Boot_Event_Type::kBoot_Stage_BSPInit);
    
    // 添加依赖模块
    result.AddDependencies(hardware_manager, gpio_manager, timer_manager);
    
    return result;
}
```

### 3. 错误处理

```cpp
void HandleBootResult(const BootResult<bool>& result) {
    if (result.is_ok()) {
        // 处理成功情况
        bool value = result.unwrap();
        auto phase = result.GetCurrentPhase();
        const auto& dependencies = result.GetDependencies();
        
    } else {
        // 处理错误情况
        auto error = result.error();
        
        // 推荐的安全做法：先检查错误类型
        if (result.IsBootPhaseError()) {
            auto boot_error = result.GetBootPhaseError();
            if (boot_error) {
                // 获取具体的启动阶段和函数信息
                auto phase = boot_error->phase();
                auto function_name = boot_error->function_name();
                LOGE("Boot phase error: %s", error->message().c_str());
                LOGE("Phase: %s, Function: %s", 
                     GetPhaseName(phase), function_name.c_str());
            }
        } else {
            // 处理其他类型的错误
            LOGE("General error: %s", error->message().c_str());
        }
    }
}
```

### 4. 链式操作

```cpp
BootResult<int> ChainOperations() {
    // 第一步
    auto hardware_result = InitializeHardware();
    if (hardware_result.is_err()) {
        return BootErr<int>(
            hardware_result.GetCurrentPhase(),
            "ChainOperations",
            "Hardware initialization failed: " + hardware_result.error()->message(),
            hardware_result.error()->code()
        );
    }
    
    // 第二步
    auto bsp_result = InitializeBSP();
    if (bsp_result.is_err()) {
        return BootErr<int>(
            bsp_result.GetCurrentPhase(),
            "ChainOperations", 
            "BSP initialization failed: " + bsp_result.error()->message(),
            bsp_result.error()->code()
        );
    }
    
    return BootOk<int>(2, Laminpie_Boot_Event_Type::kBoot_Stage_BSPInit);
}
```

## 模块名提取机制

系统使用`Name.hpp`的类型名提取功能来自动获取模块名：

```cpp
template<typename U>
std::string ExtractModuleName() const {
    std::string type_name_str = Ubpa::type_name<U>();
    
    // 查找最后一个 "::" 分隔符来提取命名空间
    size_t last_separator = type_name_str.find_last_of("::");
    if (last_separator != std::string::npos) {
        std::string namespace_part = type_name_str.substr(0, last_separator);
        
        // 提取最后一个命名空间部分作为模块名
        size_t module_start = namespace_part.find_last_of("::");
        if (module_start != std::string::npos) {
            return namespace_part.substr(module_start + 2); // +2 跳过 "::"
        } else {
            return namespace_part;
        }
    }
    
    return type_name_str;
}
```

### 示例

```cpp
// 对于类型 laminpie::device::I2CDriver
// 提取的模块名为 "device"

// 对于类型 laminpie::system::framework::BootManager  
// 提取的模块名为 "framework"
```

## 错误代码规范

系统使用以下错误代码规范：

- **-1000 ~ -1999**: 硬件初始化错误
- **-2000 ~ -2999**: BSP初始化错误  
- **-3000 ~ -3999**: 驱动程序初始化错误
- **-4000 ~ -4999**: 中间件初始化错误
- **-5000 ~ -5999**: 系统服务初始化错误
- **-6000 ~ -6999**: 资源加载错误
- **-7000 ~ -7999**: 应用程序初始化错误

## 最佳实践

### 1. 错误处理
- 总是检查`is_ok()`或`is_err()`状态
- 使用`IsBootPhaseError()`检查启动阶段特定错误
- 使用`GetBootPhaseError()`安全地获取BootPhaseError对象
- **避免使用`std::static_pointer_cast`**，优先使用`dynamic_pointer_cast`或类型检查
- 记录详细的错误上下文信息

### 2. 依赖关系管理
- 在初始化函数中及时添加依赖模块
- 使用`AddDependencies()`批量添加多个依赖
- 保持依赖关系的准确性

### 3. 日志记录
- 使用模块专用日志宏记录关键信息
- 错误日志必须包含函数名和启动阶段信息
- 成功日志记录依赖模块信息
- 错误信息格式：`"Boot phase [PhaseName] failed in function [FunctionName]: ErrorMessage"`

### 4. 性能考虑
- BootResult使用移动语义避免不必要的拷贝
- 错误信息使用智能指针管理内存
- 依赖列表使用`std::vector`提供高效的访问

### 5. 类型安全
- 优先使用`IsBootPhaseError()`进行类型检查
- 使用`GetBootPhaseError()`获取类型安全的错误对象
- 避免不安全的类型转换操作

## 与现有系统的集成

BootResult系统与以下组件集成：

1. **事件系统**: 通过`Boot_EventData_t`发送启动事件
2. **日志系统**: 使用`LP_LOG_*`宏记录启动信息
3. **错误系统**: 继承`laminate::Error`基类
4. **类型系统**: 使用`Name.hpp`进行类型名提取

## 错误处理示例对比

### ❌ 不安全的做法

```cpp
// 危险：使用static_pointer_cast可能导致未定义行为
void UnsafeErrorHandling(const BootResult<bool>& result) {
    if (result.is_err()) {
        auto error = result.error();
        auto boot_error = std::static_pointer_cast<BootPhaseError>(error);
        if (boot_error) {  // 这个检查可能无效
            LOGE("Error: %s", boot_error->message().c_str());
        }
    }
}
```

### ✅ 安全的做法

```cpp
// 安全：使用类型检查和dynamic_pointer_cast
void SafeErrorHandling(const BootResult<bool>& result) {
    if (result.is_err()) {
        auto error = result.error();
        
        // 方法1：使用BootResult提供的类型检查
        if (result.IsBootPhaseError()) {
            auto boot_error = result.GetBootPhaseError();
            if (boot_error) {
                LOGE("Boot phase error: %s", boot_error->message().c_str());
                LOGE("Phase: %s, Function: %s", 
                     GetPhaseName(boot_error->phase()), 
                     boot_error->function_name().c_str());
            }
        }
        
        // 方法2：使用dynamic_pointer_cast
        auto boot_error = std::dynamic_pointer_cast<BootPhaseError>(error);
        if (boot_error) {
            LOGE("Boot phase error: %s", boot_error->message().c_str());
        } else {
            LOGE("General error: %s", error->message().c_str());
        }
    }
}
```

## 扩展性

系统设计支持以下扩展：

1. **新的启动阶段**: 在枚举中添加新的阶段类型
2. **自定义错误类型**: 继承`BootPhaseError`创建特定错误
3. **依赖关系分析**: 基于依赖信息进行启动顺序优化
4. **错误恢复机制**: 基于错误信息实现自动恢复

## 总结

BootResult系统提供了一个完整的启动过程错误处理解决方案，具有以下优势：

- **类型安全**: 编译时确保错误被正确处理，运行时提供安全的类型检查机制
- **信息丰富**: 提供详细的错误上下文、启动阶段和依赖信息
- **易于使用**: 提供简洁的API和便捷函数，支持链式操作
- **高度集成**: 与现有系统组件无缝集成
- **可扩展性**: 支持未来功能扩展和定制
- **错误追踪**: 自动记录函数名和启动阶段，便于问题定位

### 关键设计原则

1. **安全优先**: 优先使用类型安全的错误处理方法
2. **信息完整**: 错误信息包含完整的上下文信息
3. **易于调试**: 提供清晰的错误格式和日志输出
4. **性能优化**: 使用移动语义和智能指针优化内存使用

通过使用BootResult系统，开发者可以更好地管理启动过程中的错误，提高系统的可靠性和可维护性。记住始终使用安全的错误处理模式，避免不安全的类型转换操作。
