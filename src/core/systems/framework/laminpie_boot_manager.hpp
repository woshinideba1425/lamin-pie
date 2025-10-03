#pragma once
#include "laminpie_core_framework.hpp"
#include "laminpie_system_event_type.hpp"
#include <laminpie_result.h>
#include "UTemplate/Name.hpp"

namespace laminpie::system::framework {
using namespace laminpie::system::event;

struct Laminpie_Boot_ManagerData_t {
	Laminpie_CoreHome &core_display;
	app::Laminpie_App_Manager &core_manager;
	event::LaminPie_EventDispatcher &core_event;
	device::DeviceManager &device_manager;
	lv_display_t *display_device;

	gui::LockCallback lv_lock_cb = nullptr;
	gui::UnlockCallback lv_unlock_cb = nullptr;
    Boot_EventData_t boot_event_data;
};


// 启动阶段专用错误类型
class BootPhaseError : public laminate::Error {
public:
    BootPhaseError(Laminpie_Boot_Event_Type phase, 
                   std::string function_name,
                   std::string error_message, 
                   int error_code = -1)
        : phase_(phase), function_name_(std::move(function_name)), 
          error_message_(std::move(error_message)), error_code_(error_code) {}
    
    std::string message() const override { 
        return "Boot phase [" + GetPhaseName(phase_) + "] failed in function [" + 
               function_name_ + "]: " + error_message_; 
    }
    std::string type_name() const override { return "BootPhaseError"; }
    int code() const override { return error_code_; }
    std::string context() const override { 
        return "Phase: " + GetPhaseName(phase_) + ", Function: " + function_name_; 
    }
    
    Laminpie_Boot_Event_Type phase() const { return phase_; }
    const std::string& function_name() const { return function_name_; }
    
private:
    Laminpie_Boot_Event_Type phase_;
    std::string function_name_;
    std::string error_message_;
    int error_code_;
    
    std::string GetPhaseName(Laminpie_Boot_Event_Type phase) const {
        switch (phase) {
            case Laminpie_Boot_Event_Type::kBoot_Stage_HardWareInit: return "HardwareInit";
            case Laminpie_Boot_Event_Type::kBoot_Stage_BSPInit: return "BSPInit";
            case Laminpie_Boot_Event_Type::kBoot_Stage_DriverInit: return "DriverInit";
            case Laminpie_Boot_Event_Type::kBoot_Stage_MiddlewareInit: return "MiddlewareInit";
            case Laminpie_Boot_Event_Type::kBoot_Stage_SytemServiceInit: return "SystemServiceInit";
            case Laminpie_Boot_Event_Type::kBoot_Stage_Resourceload: return "ResourceLoad";
            case Laminpie_Boot_Event_Type::kBoot_Stage_AppInit: return "AppInit";
            case Laminpie_Boot_Event_Type::kBoot_Stage_Complete: return "Complete";
            case Laminpie_Boot_Event_Type::kBoot_Stage_Failed: return "Failed";
            default: return "Unknown";
        }
    }
};

// BootResult模板类的前向声明
template<typename T>
class BootResult;

// 辅助 trait：检测是否为 BootResult 类型
template<typename T>
struct is_boot_result : std::false_type {};

template<typename T>
struct is_boot_result<BootResult<T>> : std::true_type {};

template<typename T>
inline constexpr bool is_boot_result_v = is_boot_result<T>::value;

// 辅助 trait：提取 BootResult 的 value_type
template<typename T>
struct boot_result_value_type {
    using type = T;
};

template<typename T>
struct boot_result_value_type<BootResult<T>> {
    using type = T;
};

template<typename T>
using boot_result_value_type_t = typename boot_result_value_type<T>::type;

// 辅助 trait：获取 map() 的返回类型
template<typename Func, typename ArgType>
struct map_return_type {
    using type = boot_result_value_type_t<std::invoke_result_t<Func, ArgType>>;
};

// void类型的特化版本
template<typename Func>
struct map_return_type<Func, void> {
    using type = boot_result_value_type_t<std::invoke_result_t<Func>>;
};

template<typename Func, typename ArgType>
using map_return_type_t = typename map_return_type<Func, ArgType>::type;

// 静态函数：从类型名提取模块名
template<typename U>
std::string ExtractModuleNameForType() {
    std::string type_name_str(Ubpa::type_name<U>().View());
    
    // 查找最后一个 "::" 分隔符
    size_t last_separator = type_name_str.find_last_of("::");
    if (last_separator != std::string::npos) {
        std::string namespace_part = type_name_str.substr(0, last_separator);
        
        // 检查是否包含函数调用 "()"
        if (namespace_part.find("()") != std::string::npos) {
            // 如果有函数调用，需要找到函数调用之前的 "::"
            // 先找到 "()" 的位置
            size_t paren_pos = namespace_part.find("()");
            if (paren_pos != std::string::npos) {
                // 在 "()" 之前查找最后一个 "::"
                std::string before_paren = namespace_part.substr(0, paren_pos);
                size_t second_last_separator = before_paren.find_last_of("::");
                if (second_last_separator != std::string::npos) {
                    if (second_last_separator > 0) {
                        std::string result = before_paren.substr(0, second_last_separator - 1);
                        return result;
                    }
                }
            }
        }
        
        // 没有函数调用，直接返回命名空间部分
        return namespace_part;
    }
    
    return "Unknown";
}

// void类型的特化版本
template<>
class BootResult<void> : public laminate::Result<void> {
public:
    // 成功构造函数 - void类型不需要value参数
    BootResult(Laminpie_Boot_Event_Type phase) 
        : laminate::Result<void>(), current_phase_(phase) {}
    
    // 默认构造函数
    BootResult(void) : laminate::Result<void>(), current_phase_(Laminpie_Boot_Event_Type::kBoot_Event_Type_Max) {}
    
    // 错误构造函数
    BootResult(std::shared_ptr<laminate::Error> error) : laminate::Result<void>(error), current_phase_(Laminpie_Boot_Event_Type::kBoot_Event_Type_Max) {}
    
    // 从具体错误类型构造
    template <typename E, typename = std::enable_if_t<std::is_base_of_v<laminate::Error, E>>>
    BootResult(E error) : laminate::Result<void>(std::move(error)), current_phase_(Laminpie_Boot_Event_Type::kBoot_Event_Type_Max) {}
    
    // 设置当前启动阶段
    BootResult<void>& SetPhase(Laminpie_Boot_Event_Type phase) {
        current_phase_ = phase;
        return *this;
    }
    
    // 添加依赖模块
    template<typename U>
    BootResult<void>& AddDependency(const U& dependency) {
        std::string module_name = ExtractModuleNameForType<U>();
        _related_dependencies.push_back(module_name);
        return *this;
    }
    
    // 获取当前启动阶段
    Laminpie_Boot_Event_Type GetCurrentPhase() const { return current_phase_; }
    
    // 获取相关依赖
    const std::vector<std::string>& GetDependencies() const { return _related_dependencies; }
    
    // 重写map方法，返回BootResult类型而不是Result类型
    template <typename Func>
    auto map(Func&& func) const -> BootResult<map_return_type_t<Func, void>> {
        using ReturnType = std::invoke_result_t<Func>;
        using FinalReturnType = map_return_type_t<Func, void>;
        if (this->is_ok()) {
            auto result = func();
            // 如果func返回的是BootResult类型，需要解包
            if constexpr (is_boot_result_v<ReturnType>) {
                // 解包嵌套的BootResult
                if (result.is_ok()) {
                    return BootResult<FinalReturnType>(result.unwrap(), result.GetCurrentPhase());
                } else {
                    return BootResult<FinalReturnType>(result.error());
                }
            } else {
                // 否则包装成BootResult
                return BootResult<FinalReturnType>(std::move(result), current_phase_);
            }
        }
        return BootResult<FinalReturnType>(this->error());
    }

private:
    Laminpie_Boot_Event_Type current_phase_;
    std::vector<std::string> _related_dependencies;
};

// 非void类型的通用版本
template<typename T>
class BootResult : public laminate::Result<T> {
public:
    // 成功构造函数
    BootResult(T value, Laminpie_Boot_Event_Type phase) 
        : laminate::Result<T>(std::move(value)), current_phase_(phase) {}
    
    // 默认构造函数
    BootResult(void) : laminate::Result<T>(), current_phase_(Laminpie_Boot_Event_Type::kBoot_Event_Type_Max) {}
    
    // 错误构造函数
    BootResult(std::shared_ptr<laminate::Error> error) : laminate::Result<T>(error), current_phase_(Laminpie_Boot_Event_Type::kBoot_Event_Type_Max) {}
    
    // 从具体错误类型构造
    template <typename E, typename = std::enable_if_t<std::is_base_of_v<laminate::Error, E>>>
    BootResult(E error) : laminate::Result<T>(std::move(error)), current_phase_(Laminpie_Boot_Event_Type::kBoot_Event_Type_Max) {}
    
    // 设置当前启动阶段
    BootResult<T>& SetPhase(Laminpie_Boot_Event_Type phase) {
        current_phase_ = phase;
        return *this;
    }
    
    // 添加依赖模块
    template<typename U>
    BootResult<T>& AddDependency(const U& dependency) {
        std::string module_name = ExtractModuleNameForType<U>();
        _related_dependencies.push_back(module_name);
        return *this;
    }
    
    // 添加多个依赖模块
    template<typename U, typename... Args>
    BootResult<T>& AddDependencies(const U& dependency, const Args&... args) {
        AddDependency(dependency);
        if constexpr (sizeof...(args) > 0) {
            AddDependencies(args...);
        }
        return *this;
    }
    
    // 创建启动阶段错误
    static BootResult<T> CreatePhaseError(Laminpie_Boot_Event_Type phase,
                                        const std::string& function_name,
                                        const std::string& error_message,
                                        int error_code = -1) {
        return BootResult<T>(BootPhaseError(phase, function_name, error_message, error_code))
               .SetPhase(phase);
    }
    
    // 获取当前启动阶段
    Laminpie_Boot_Event_Type GetCurrentPhase() const { return current_phase_; }
    
    // 获取依赖模块列表
    const std::vector<std::string>& GetDependencies() const { return _related_dependencies; }
    
    // 检查是否为启动阶段错误
    bool IsBootPhaseError() const {
        if (this->is_err()) {
            return this->error()->template is<BootPhaseError>();
        }
        return false;
    }
    
    // 获取启动阶段错误信息
    std::shared_ptr<BootPhaseError> GetBootPhaseError() const {
        if (IsBootPhaseError()) {
            return std::dynamic_pointer_cast<BootPhaseError>(this->error());
        }
        return nullptr;
    }
    
    // 重写map方法，返回BootResult类型而不是Result类型
    template <typename Func>
    auto map(Func&& func) const -> BootResult<map_return_type_t<Func, T>> {
        using ReturnType = std::invoke_result_t<Func, T>;
        using FinalReturnType = map_return_type_t<Func, T>;
        if (this->is_ok()) {
            auto result = func(this->unwrap());
            // 如果func返回的是BootResult类型，需要解包
            if constexpr (is_boot_result_v<ReturnType>) {
                // 解包嵌套的BootResult
                if (result.is_ok()) {
                    return BootResult<FinalReturnType>(result.unwrap(), result.GetCurrentPhase());
                } else {
                    return BootResult<FinalReturnType>(result.error());
                }
            } else {
                // 否则包装成BootResult
                return BootResult<FinalReturnType>(std::move(result), current_phase_);
            }
        }
        return BootResult<FinalReturnType>(this->error());
    }

private:
    Laminpie_Boot_Event_Type current_phase_;
    std::vector<std::string> _related_dependencies;
    
};

// BootResult便捷函数
template<typename T>
BootResult<T> BootOk(T value, Laminpie_Boot_Event_Type phase = Laminpie_Boot_Event_Type::kBoot_Event_Type_Max) {
    return BootResult<T>(std::move(value), phase);
}

inline BootResult<void> BootOk(Laminpie_Boot_Event_Type phase = Laminpie_Boot_Event_Type::kBoot_Event_Type_Max) {
    return BootResult<void>(phase);
}

template<typename T>
BootResult<T> BootErr(Laminpie_Boot_Event_Type phase,
                     const std::string& function_name,
                     const std::string& error_message,
                     int error_code = -1) {
    return BootResult<T>::CreatePhaseError(phase, function_name, error_message, error_code);
}

class Laminpie_Boot_Manager{
public:
    Laminpie_Boot_Manager(Laminpie_Boot_ManagerData_t &data);
    ~Laminpie_Boot_Manager(void);

    bool ConsignToBoot(Laminpie_Core_Framework &core_framework);

protected:
	virtual bool InitializeHardware() = 0;
	virtual bool InitializeBsp() = 0;
	virtual bool InitializeDrivers() = 0;
	virtual bool InitializeMiddleware() = 0;
	virtual bool InitializeSystemServices() = 0;
	virtual bool LoadResources() = 0;
	virtual bool InitializeApplication() = 0;
	virtual bool PostSelfTest(){return true;};

private:
    Laminpie_Boot_Manager(const Laminpie_Boot_Manager &) = delete;
    Laminpie_Boot_Manager &operator=(const Laminpie_Boot_Manager &) = delete;

	void NotifyPhase(Laminpie_Boot_Event_Type phase);
	void NotifyError(Laminpie_Boot_Event_Type phase, int err, const std::string &msg);
	void NotifyComplete(Laminpie_Boot_Event_Type phase);

    bool RunPhaseWithTimeout(std::function<bool()> phase_fn, Laminpie_Boot_Event_Type phase);
    bool RunPhase(std::function<bool()> phase_fn, Laminpie_Boot_Event_Type phase);

    // helper methods
    uint8_t CalculateProgress(Laminpie_Boot_Event_Type phase) const;
    uint16_t GetPhaseIndex(Laminpie_Boot_Event_Type phase) const;
    const char* GetPhaseName(Laminpie_Boot_Event_Type phase) const;

    Laminpie_Boot_ManagerData_t _boot_manager_data;
    Laminpie_Boot_Event_Type _boot_status;
    Laminpie_Boot_Event_Type _previous_phase {Laminpie_Boot_Event_Type::kBoot_Event_Type_Max};
    std::string _current_request_id;
};
}