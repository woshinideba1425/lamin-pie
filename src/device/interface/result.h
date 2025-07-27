// result.h - 类似Rust的Result类型
#pragma once
#include "error.h"
#include <variant>
#include <utility>
#include <optional>
#include <memory>
#include <string>
#include "error.h"

namespace laminate {

template <typename T>
class Result {
public:
    // 成功构造
    Result(T value) : variant_(std::move(value)) {}
    
    // 错误构造
    Result(std::shared_ptr<Error> error) : variant_(std::move(error)) {}
    
    // 从具体错误类型构造
    template <typename E, typename = std::enable_if_t<std::is_base_of_v<Error, E>>>
    Result(E error) : variant_(std::make_shared<E>(std::move(error))) {}
    
    // 检查是否成功
    bool is_ok() const { return std::holds_alternative<T>(variant_); }
    bool is_err() const { return !is_ok(); }
    
    // 获取值(不安全，调用前应检查is_ok())
    const T& unwrap() const { return std::get<T>(variant_); }
    T& unwrap() { return std::get<T>(variant_); }
    
    // 带默认值的解包
    T unwrap_or(T default_value) const {
        if (is_ok()) return unwrap();
        return default_value;
    }
    
    // 获取错误(不安全，调用前应检查is_err())
    std::shared_ptr<Error> error() const { return std::get<std::shared_ptr<Error>>(variant_); }
    
    // 映射操作 - 类似Rust的map
    template <typename Func>
    auto map(Func&& func) const -> Result<std::invoke_result_t<Func, T>> {
        using ReturnType = std::invoke_result_t<Func, T>;
        if (is_ok()) {
            return Result<ReturnType>(func(unwrap()));
        }
        return Result<ReturnType>(error());
    }
    
    // 错误映射 - 类似Rust的map_err
    template <typename Func>
    Result<T> map_err(Func&& func) const {
        if (is_err()) {
            return Result<T>(func(error()));
        }
        return *this;
    }
    
    // 添加上下文 - 类似Rust的context
    Result<T> with_context(std::string context) const {
        if (is_err()) {
            // 创建带上下文的新错误
            auto err = error();
            class ContextualError : public Error {
            public:
                ContextualError(std::shared_ptr<Error> err, std::string ctx) 
                    : wrapped_error_(std::move(err)), context_(std::move(ctx)) {}
                
                std::string message() const override { return wrapped_error_->message(); }
                std::string type_name() const override { return wrapped_error_->type_name(); }
                int code() const override { return wrapped_error_->code(); }
                std::string context() const override { return context_; }
                
            private:
                std::shared_ptr<Error> wrapped_error_;
                std::string context_;
            };
            
            return Result<T>(std::make_shared<ContextualError>(err, std::move(context)));
        }
        return *this;
    }

private:
    std::variant<T, std::shared_ptr<Error>> variant_;
};

// 特化void返回类型
template <>
class Result<void> {
public:
    Result() : error_(std::nullopt) {}
    Result(std::shared_ptr<Error> error) : error_(std::move(error)) {}
    
    template <typename E, typename = std::enable_if_t<std::is_base_of_v<Error, E>>>
    Result(E error) : error_(std::make_shared<E>(std::move(error))) {}
    
    bool is_ok() const { return !error_.has_value(); }
    bool is_err() const { return error_.has_value(); }
    
    std::shared_ptr<Error> error() const { return *error_; }
    
    Result<void> with_context(std::string context) const {
        if (is_err()) {
            auto err = error();
            class ContextualError : public Error {
            public:
                ContextualError(std::shared_ptr<Error> err, std::string ctx) 
                    : wrapped_error_(std::move(err)), context_(std::move(ctx)) {}
                
                std::string message() const override { return wrapped_error_->message(); }
                std::string type_name() const override { return wrapped_error_->type_name(); }
                int code() const override { return wrapped_error_->code(); }
                std::string context() const override { return context_; }
                
            private:
                std::shared_ptr<Error> wrapped_error_;
                std::string context_;
            };

            return Result<void>(std::make_shared<ContextualError>(err, std::move(context)));
        }
        return *this;
    }

private:
    std::optional<std::shared_ptr<Error>> error_;
};

// 便捷函数
template <typename T>
Result<T> Ok(T value) {
    return Result<T>(std::move(value));
}

inline Result<void> Ok() {
    return Result<void>();
}

template <typename E, typename = std::enable_if_t<std::is_base_of_v<Error, E>>>
Result<void> Err(E error) {
    return Result<void>(std::move(error));
}

}  // namespace laminate