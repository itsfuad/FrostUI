#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// FrostUI Result Type (Error Handling)
// ─────────────────────────────────────────────────────────────────────────────

#include "frost/core/types.hpp"
#include <variant>
#include <string>
#include <source_location>
#include <format>

namespace frost {

// ─────────────────────────────────────────────────────────────────────────────
// Error Type
// ─────────────────────────────────────────────────────────────────────────────

enum class ErrorCode : u32 {
    None = 0,

    // General
    Unknown,
    InvalidArgument,
    OutOfMemory,
    NotFound,
    AlreadyExists,
    NotSupported,
    Timeout,

    // Platform
    PlatformError,
    WindowCreationFailed,

    // Vulkan
    VulkanNotAvailable,
    VulkanInitFailed,
    VulkanDeviceNotSuitable,
    VulkanSurfaceCreationFailed,
    VulkanSwapchainCreationFailed,
    VulkanPipelineCreationFailed,
    VulkanShaderCompilationFailed,
    VulkanOutOfDeviceMemory,

    // Graphics
    ShaderLoadFailed,
    TextureLoadFailed,

    // IO
    FileNotFound,
    FileReadError,
    FileWriteError,

    // UI
    WidgetNotFound,
    InvalidLayout,
};

struct Error {
    ErrorCode code{ErrorCode::None};
    String message;
    String location;

    constexpr Error() = default;

    Error(ErrorCode c, StringView msg = {},
          std::source_location loc = std::source_location::current())
        : code(c)
        , message(msg)
        , location(std::format("{}:{}:{}", loc.file_name(), loc.line(), loc.column()))
    {}

    [[nodiscard]] bool is_ok() const { return code == ErrorCode::None; }
    [[nodiscard]] explicit operator bool() const { return !is_ok(); }

    [[nodiscard]] String to_string() const {
        if (message.empty()) {
            return std::format("[{}] Error code: {}", location, static_cast<u32>(code));
        }
        return std::format("[{}] {}", location, message);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Result Type
// ─────────────────────────────────────────────────────────────────────────────

template<typename T>
class Result {
public:
    using ValueType = T;

    // Success constructors
    Result(T value) : data_(std::move(value)) {}

    // Error constructor
    Result(Error error) : data_(std::move(error)) {}
    Result(ErrorCode code, StringView msg = {},
           std::source_location loc = std::source_location::current())
        : data_(Error{code, msg, loc}) {}

    // Status checks
    [[nodiscard]] bool is_ok() const { return std::holds_alternative<T>(data_); }
    [[nodiscard]] bool is_err() const { return std::holds_alternative<Error>(data_); }
    [[nodiscard]] explicit operator bool() const { return is_ok(); }

    // Value access
    [[nodiscard]] T& value() & {
        return std::get<T>(data_);
    }

    [[nodiscard]] const T& value() const& {
        return std::get<T>(data_);
    }

    [[nodiscard]] T&& value() && {
        return std::get<T>(std::move(data_));
    }

    [[nodiscard]] T value_or(T default_value) const& {
        if (is_ok()) return std::get<T>(data_);
        return default_value;
    }

    [[nodiscard]] T* operator->() { return &value(); }
    [[nodiscard]] const T* operator->() const { return &value(); }
    [[nodiscard]] T& operator*() & { return value(); }
    [[nodiscard]] const T& operator*() const& { return value(); }

    // Error access
    [[nodiscard]] const Error& error() const {
        return std::get<Error>(data_);
    }

    [[nodiscard]] ErrorCode error_code() const {
        return is_err() ? std::get<Error>(data_).code : ErrorCode::None;
    }

    // Transformations
    template<typename Fn>
    auto map(Fn&& fn) -> Result<decltype(fn(std::declval<T&>()))> {
        using U = decltype(fn(std::declval<T&>()));
        if (is_ok()) {
            return Result<U>(fn(value()));
        }
        return Result<U>(error());
    }

    template<typename Fn>
    auto and_then(Fn&& fn) -> decltype(fn(std::declval<T&>())) {
        using ReturnType = decltype(fn(std::declval<T&>()));
        if (is_ok()) {
            return fn(value());
        }
        return ReturnType(error());
    }

private:
    std::variant<T, Error> data_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Void Result Specialization
// ─────────────────────────────────────────────────────────────────────────────

template<>
class Result<void> {
public:
    Result() = default;

    Result(Error error) : error_(std::move(error)) {}
    Result(ErrorCode code, StringView msg = {},
           std::source_location loc = std::source_location::current())
        : error_(Error{code, msg, loc}) {}

    [[nodiscard]] bool is_ok() const { return !error_.has_value(); }
    [[nodiscard]] bool is_err() const { return error_.has_value(); }
    [[nodiscard]] explicit operator bool() const { return is_ok(); }

    [[nodiscard]] const Error& error() const { return *error_; }
    [[nodiscard]] ErrorCode error_code() const {
        return error_.has_value() ? error_->code : ErrorCode::None;
    }

private:
    Option<Error> error_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Helper Macros
// ─────────────────────────────────────────────────────────────────────────────

#define FROST_TRY(expr) \
    ({ \
        auto&& _result = (expr); \
        if (!_result) return _result.error(); \
        std::move(_result).value(); \
    })

#define FROST_CHECK(expr, code, msg) \
    do { \
        if (!(expr)) return ::frost::Error{code, msg}; \
    } while (0)

// ─────────────────────────────────────────────────────────────────────────────
// Common Result Aliases
// ─────────────────────────────────────────────────────────────────────────────

using VoidResult = Result<void>;

} // namespace frost
