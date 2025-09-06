# LaminPie Result系统与错误处理文档

## 概述

LaminPie Result系统是一个类似Rust Result类型的C++实现，提供了类型安全、零开销的错误处理机制。该系统允许函数返回成功值或错误信息，强制调用者处理可能的错误情况。

## 核心特性

- **类型安全**: 编译时确保错误被处理
- **零开销抽象**: 成功路径几乎无性能损失
- **链式操作**: 支持函数式编程风格的错误处理
- **上下文传播**: 错误信息可以携带上下文信息
- **RAII兼容**: 与C++资源管理机制完美配合

## 基本概念

### Result类型

```cpp
template <typename T>
class Result {
    // 包含成功值T或错误信息
    std::variant<T, std::shared_ptr<Error>> variant_;
};
```

Result类型是一个联合类型，要么包含成功的结果值，要么包含错误信息。这确保了类型安全，防止未处理的错误。

### 错误类型层次

```cpp
class Error {
public:
    virtual ~Error() = default;
    virtual std::string message() const = 0;
    virtual std::string type_name() const = 0;
    virtual int code() const = 0;
    virtual std::string context() const { return ""; }
};
```

所有错误类型都继承自基类Error，提供统一的错误信息接口。

## 基本使用

### 1. 创建Result

```cpp
#include "result.h"

// 成功结果
Result<int> success_result = Ok(42);
Result<std::string> success_string = Ok(std::string("hello"));

// 错误结果
Result<int> error_result = Err(StandardError("Something went wrong", -1));

// 使用便捷函数
auto result1 = Ok(100);
auto result2 = Err(TimeoutError("Operation timed out"));
```

### 2. 检查结果状态

```cpp
Result<int> result = some_operation();

// 检查是否成功
if (result.is_ok()) {
    // 处理成功情况
    int value = result.unwrap();
    std::cout << "Value: " << value << std::endl;
}

// 检查是否出错
if (result.is_err()) {
    // 处理错误情况
    auto error = result.error();
    std::cerr << "Error: " << error->message() << std::endl;
}
```

### 3. 安全解包

```cpp
Result<int> result = get_number();

// 不安全解包（需要先检查is_ok()）
int value = result.unwrap();  // 如果出错会抛出异常

// 带默认值的解包
int value = result.unwrap_or(0);  // 如果出错返回0

// 带默认值的解包（使用函数）
int value = result.unwrap_or_else([]() { 
    return calculate_default_value(); 
});
```

## 链式操作

### map操作

map操作允许对成功值进行转换，如果结果是错误则保持不变：

```cpp
Result<int> result = get_number()
    .map([](int x) { return x * 2; })           // 将数字乘以2
    .map([](int x) { return std::to_string(x); }) // 转换为字符串
    .map([](const std::string& s) { return s.length(); }); // 获取长度

// 如果任何一步失败，后续步骤会被跳过
if (result.is_err()) {
    // 处理错误
}
```

### map_err操作

map_err操作允许转换错误类型：

```cpp
Result<int> result = get_number()
    .map_err([](std::shared_ptr<Error> err) {
        // 将错误转换为更具体的错误类型
        return std::make_shared<CustomError>(err->message());
    });
```

### with_context操作

with_context操作为错误添加上下文信息：

```cpp
Result<int> result = load_config()
    .with_context("Failed to load configuration file")
    .map([](const Config& cfg) { return parse_config(cfg); })
    .with_context("Failed to parse configuration");

if (result.is_err()) {
    auto error = result.error();
    std::cerr << "Context: " << error->context() << std::endl;
    std::cerr << "Error: " << error->message() << std::endl;
}
```

## 实际应用示例

### 1. 文件操作

```cpp
Result<std::string> read_file_content(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return Err(FileError("Cannot open file: " + filename, -1));
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    if (file.bad()) {
        return Err(FileError("Error reading file: " + filename, -2));
    }
    
    return Ok(content);
}

// 使用示例
auto result = read_file_content("config.json")
    .map([](const std::string& content) { return parse_json(content); })
    .with_context("Failed to load configuration");

if (result.is_err()) {
    LP_LOG_ERROR("CONFIG", "Error: %s", result.error()->message().c_str());
    return false;
}
```

### 2. 网络请求

```cpp
Result<HttpResponse> make_http_request(const std::string& url) {
    // 模拟HTTP请求
    if (url.empty()) {
        return Err(NetworkError("Empty URL", -1));
    }
    
    // 模拟网络延迟
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 模拟随机失败
    if (rand() % 10 == 0) {
        return Err(NetworkError("Network timeout", -2));
    }
    
    return Ok(HttpResponse{200, "OK", "Response data"});
}

// 使用示例
auto result = make_http_request("https://api.example.com/data")
    .map([](const HttpResponse& resp) { return resp.body; })
    .map([](const std::string& body) { return parse_json(body); })
    .with_context("Failed to fetch and parse API data");
```

### 3. 设备管理

```cpp
Result<Device> initialize_device(const std::string& device_id) {
    // 查找设备
    auto device_result = find_device(device_id);
    if (device_result.is_err()) {
        return Err(device_result.error());
    }
    
    // 加载驱动
    auto driver_result = load_driver(device_id);
    if (driver_result.is_err()) {
        return Err(driver_result.error());
    }
    
    // 初始化设备
    auto device = Device(device_result.unwrap(), driver_result.unwrap());
    if (!device.initialize()) {
        return Err(DeviceError("Device initialization failed", -1));
    }
    
    return Ok(device);
}

// 使用示例
auto result = initialize_device("sensor_001")
    .map([](const Device& device) { return device.get_capabilities(); })
    .with_context("Failed to initialize sensor device");

if (result.is_err()) {
    SYSTEM_DEVICE_LOG_ERROR("Device init failed: %s", 
                           result.error()->message().c_str());
    return false;
}
```

## 错误类型定义

### 标准错误类型

```cpp
class StandardError : public Error {
private:
    std::string message_;
    int code_;
    
public:
    StandardError(const std::string& message, int code) 
        : message_(message), code_(code) {}
    
    std::string message() const override { return message_; }
    std::string type_name() const override { return "StandardError"; }
    int code() const override { return code_; }
};

class FileError : public Error {
private:
    std::string message_;
    int code_;
    
public:
    FileError(const std::string& message, int code) 
        : message_(message), code_(code) {}
    
    std::string message() const override { return message_; }
    std::string type_name() const override { return "FileError"; }
    int code() const override { return code_; }
};

class NetworkError : public Error {
private:
    std::string message_;
    int code_;
    
public:
    NetworkError(const std::string& message, int code) 
        : message_(message), code_(code) {}
    
    std::string message() const override { return message_; }
    std::string type_name() const override { return "NetworkError"; }
    int code() const override { return code_; }
};
```

### 上下文错误

```cpp
class ContextualError : public Error {
private:
    std::shared_ptr<Error> wrapped_error_;
    std::string context_;
    
public:
    ContextualError(std::shared_ptr<Error> err, std::string ctx) 
        : wrapped_error_(std::move(err)), context_(std::move(ctx)) {}
    
    std::string message() const override { return wrapped_error_->message(); }
    std::string type_name() const override { return wrapped_error_->type_name(); }
    int code() const override { return wrapped_error_->code(); }
    std::string context() const override { return context_; }
};
```

## 性能考虑

### 内存开销

```cpp
// Result<T>的内存占用
sizeof(Result<int>) = sizeof(std::variant<int, std::shared_ptr<Error>>)
                    = max(sizeof(int), sizeof(std::shared_ptr<Error>)) + 1
                    = 16 + 1 = 17 bytes (64位系统)

// 对于大类型
sizeof(Result<std::string>) = max(sizeof(std::string), sizeof(std::shared_ptr<Error>)) + 1
                            = 32 + 1 = 33 bytes (64位系统)
```

### 性能特点

- **成功路径**: 几乎零开销，直接访问variant中的值
- **错误路径**: 有堆分配开销（shared_ptr）
- **链式操作**: 每次map操作创建新的Result对象
- **错误传播**: 错误在链式操作中自动传播，无额外开销

### 优化建议

1. **小错误使用枚举**: 对于简单的错误码，考虑使用枚举避免堆分配
2. **合并操作**: 将多个map操作合并为单个操作减少中间对象
3. **错误池**: 对于频繁的错误，使用对象池减少分配开销

## 最佳实践

### 1. 错误处理策略

```cpp
// 好的做法：明确处理错误
auto result = risky_operation();
if (result.is_err()) {
    LP_LOG_ERROR("MODULE", "Operation failed: %s", result.error()->message().c_str());
    return false;  // 或者返回适当的错误码
}
int value = result.unwrap();

// 避免：忽略错误
int value = risky_operation().unwrap();  // 危险！可能抛出异常
```

### 2. 错误信息设计

```cpp
// 好的错误信息
return Err(StandardError("Cannot connect to database: connection timeout after 30s", -1001));

// 避免：模糊的错误信息
return Err(StandardError("Error", -1));
```

### 3. 链式操作使用

```cpp
// 好的链式操作：逻辑清晰
auto result = load_config()
    .map([](const Config& cfg) { return validate_config(cfg); })
    .map([](const ValidConfig& cfg) { return apply_config(cfg); })
    .with_context("Configuration processing failed");

// 避免：过长的链式操作
auto result = step1().map(f1).map(f2).map(f3).map(f4).map(f5);  // 难以理解
```

### 4. 类型安全

```cpp
// 好的做法：利用类型系统
Result<int> get_number();
Result<std::string> get_string();

auto result = get_number()
    .map([](int x) { return std::to_string(x); })  // 类型匹配
    .map([](const std::string& s) { return s.length(); });

// 避免：类型不匹配
auto result = get_number()
    .map([](int x) { return x; })  // 返回int
    .map([](const std::string& s) { return s.length(); });  // 编译错误！
```

## 与现有系统集成

### 1. 与日志系统集成

```cpp
// 在错误处理中使用日志系统
auto result = some_operation();
if (result.is_err()) {
    auto error = result.error();
    LP_LOG_ERROR("MODULE_TAG", "Operation failed: %s (code: %d)", 
                error->message().c_str(), error->code());
    return false;
}
```

### 2. 与事件系统集成

```cpp
// 在设备管理中使用Result
Result<Device> DeviceManager::initializeDevice(const std::string& deviceId) {
    auto device = findDevice(deviceId);
    if (device.is_err()) {
        return Err(device.error());
    }
    
    // 发送设备初始化事件
    EventDispatcher::getInstance().post(DeviceInitEvent(deviceId));
    
    return Ok(device.unwrap());
}
```

### 3. 与现有错误码集成

```cpp
// 将现有错误码转换为Result
Result<void> convert_error_code(int error_code) {
    switch (error_code) {
        case 0:
            return Ok();
        case -1:
            return Err(StandardError("Invalid parameter", error_code));
        case -2:
            return Err(StandardError("Timeout", error_code));
        default:
            return Err(StandardError("Unknown error", error_code));
    }
}
```

## 总结

LaminPie Result系统提供了一个强大而优雅的错误处理机制，具有以下优势：

1. **类型安全**: 编译时确保错误被处理
2. **零开销抽象**: 成功路径几乎无性能损失
3. **链式操作**: 支持函数式编程风格
4. **上下文传播**: 错误信息可以携带丰富的上下文
5. **易于集成**: 与现有系统完美配合

通过合理使用Result系统，可以显著提高代码的健壮性和可维护性，同时保持优秀的性能表现。
