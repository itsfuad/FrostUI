#pragma once

#include "frost/core/types.hpp"

namespace frost {

/// Key codes following USB HID usage tables (cross-platform compatible)
enum class KeyCode : u32 {
    Unknown = 0,

    // Letters (A-Z)
    A = 4, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    // Numbers (top row)
    Num1 = 30, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,

    // Control keys
    Return = 40,
    Escape = 41,
    Backspace = 42,
    Tab = 43,
    Space = 44,

    // Punctuation
    Minus = 45,
    Equals = 46,
    LeftBracket = 47,
    RightBracket = 48,
    Backslash = 49,
    Semicolon = 51,
    Apostrophe = 52,
    Grave = 53,
    Comma = 54,
    Period = 55,
    Slash = 56,

    CapsLock = 57,

    // Function keys
    F1 = 58, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

    // Navigation/editing keys
    PrintScreen = 70,
    ScrollLock = 71,
    Pause = 72,
    Insert = 73,
    Home = 74,
    PageUp = 75,
    Delete = 76,
    End = 77,
    PageDown = 78,

    // Arrow keys
    Right = 79,
    Left = 80,
    Down = 81,
    Up = 82,

    NumLock = 83,

    // Numpad
    NumpadDivide = 84,
    NumpadMultiply = 85,
    NumpadMinus = 86,
    NumpadPlus = 87,
    NumpadEnter = 88,
    Numpad1 = 89, Numpad2, Numpad3, Numpad4, Numpad5,
    Numpad6, Numpad7, Numpad8, Numpad9, Numpad0,
    NumpadDecimal = 99,

    // Modifier keys
    LeftControl = 224,
    LeftShift = 225,
    LeftAlt = 226,
    LeftSuper = 227,
    RightControl = 228,
    RightShift = 229,
    RightAlt = 230,
    RightSuper = 231,
};

/// Key action type
enum class KeyAction : u8 {
    Release = 0,
    Press = 1,
    Repeat = 2
};

/// Mouse button identifiers
enum class MouseButton : u8 {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
    Button6 = 5,
    Button7 = 6,
    Button8 = 7,
};

/// Keyboard modifier flags
enum class ModifierFlags : u8 {
    None = 0,
    Shift = 1 << 0,
    Control = 1 << 1,
    Alt = 1 << 2,
    Super = 1 << 3,
    CapsLock = 1 << 4,
    NumLock = 1 << 5,
};

/// Bitwise OR for ModifierFlags
inline constexpr ModifierFlags operator|(ModifierFlags a, ModifierFlags b) {
    return static_cast<ModifierFlags>(static_cast<u8>(a) | static_cast<u8>(b));
}

/// Bitwise AND for ModifierFlags
inline constexpr ModifierFlags operator&(ModifierFlags a, ModifierFlags b) {
    return static_cast<ModifierFlags>(static_cast<u8>(a) & static_cast<u8>(b));
}

/// Bitwise OR assignment for ModifierFlags
inline constexpr ModifierFlags& operator|=(ModifierFlags& a, ModifierFlags b) {
    return a = a | b;
}

/// Check if modifier flags contain a specific modifier
[[nodiscard]] inline constexpr bool has_modifier(ModifierFlags flags, ModifierFlags test) {
    return (flags & test) == test;
}

/// Cursor/pointer type for the mouse
enum class CursorType : u8 {
    Arrow = 0,
    IBeam,
    Crosshair,
    Hand,
    ResizeH,
    ResizeV,
    ResizeNESW,
    ResizeNWSE,
    ResizeAll,
    NotAllowed,
    Hidden,
};

/// Tracks current input state for polling-based input queries
struct InputState {
    bool keys[256]{};
    bool mouse_buttons[8]{};
    f64 mouse_x{0.0};
    f64 mouse_y{0.0};
    ModifierFlags modifiers{ModifierFlags::None};

    /// Check if a key is currently pressed
    [[nodiscard]] constexpr bool is_key_down(KeyCode key) const {
        return keys[static_cast<u32>(key) & 0xFF];
    }

    /// Check if a mouse button is currently pressed
    [[nodiscard]] constexpr bool is_mouse_button_down(MouseButton button) const {
        return mouse_buttons[static_cast<u8>(button)];
    }

    /// Get current mouse position
    [[nodiscard]] constexpr Point2D mouse_position() const {
        return Point2D{static_cast<f32>(mouse_x), static_cast<f32>(mouse_y)};
    }
};

} // namespace frost
