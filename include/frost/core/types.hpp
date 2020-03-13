#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// FrostUI Core Types
// ─────────────────────────────────────────────────────────────────────────────

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <optional>
#include <variant>
#include <functional>

namespace frost {

// ─────────────────────────────────────────────────────────────────────────────
// Integer Types
// ─────────────────────────────────────────────────────────────────────────────

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;
using isize = std::ptrdiff_t;

// ─────────────────────────────────────────────────────────────────────────────
// Handle Types
// ─────────────────────────────────────────────────────────────────────────────

template<typename Tag>
struct Handle {
    u64 value{0};

    constexpr Handle() = default;
    constexpr explicit Handle(u64 v) : value(v) {}

    constexpr bool is_valid() const { return value != 0; }
    constexpr explicit operator bool() const { return is_valid(); }

    constexpr bool operator==(Handle other) const { return value == other.value; }
    constexpr bool operator!=(Handle other) const { return value != other.value; }
    constexpr bool operator<(Handle other) const { return value < other.value; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Common Aliases
// ─────────────────────────────────────────────────────────────────────────────

template<typename T>
using Unique = std::unique_ptr<T>;

template<typename T>
using Shared = std::shared_ptr<T>;

template<typename T>
using Weak = std::weak_ptr<T>;

template<typename T>
using Option = std::optional<T>;

template<typename T>
using Span = std::span<T>;

template<typename T>
using Vector = std::vector<T>;

using String = std::string;
using StringView = std::string_view;

// ─────────────────────────────────────────────────────────────────────────────
// Utility Functions
// ─────────────────────────────────────────────────────────────────────────────

template<typename T, typename... Args>
[[nodiscard]] constexpr Unique<T> make_unique(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
[[nodiscard]] constexpr Shared<T> make_shared(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// ─────────────────────────────────────────────────────────────────────────────
// Non-copyable/Non-movable Base
// ─────────────────────────────────────────────────────────────────────────────

struct NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

struct NonMovable {
    NonMovable() = default;
    NonMovable(const NonMovable&) = delete;
    NonMovable& operator=(const NonMovable&) = delete;
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
};

} // namespace frost
