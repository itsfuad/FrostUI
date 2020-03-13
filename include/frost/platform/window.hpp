#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// FrostUI Platform Window
// ─────────────────────────────────────────────────────────────────────────────

#include "frost/core/types.hpp"
#include "frost/core/result.hpp"
#include "frost/core/math.hpp"
#include "frost/core/signals.hpp"
#include "frost/platform/input.hpp"

#ifdef _WIN32
    struct HWND__;
    using HWND = HWND__*;
    struct HINSTANCE__;
    using HINSTANCE = HINSTANCE__*;
#endif

namespace frost {

// Forward declarations
class VulkanContext;

// ─────────────────────────────────────────────────────────────────────────────
// Window Configuration
// ─────────────────────────────────────────────────────────────────────────────

struct WindowConfig {
    String title{"FrostUI Window"};
    i32 width{1280};
    i32 height{720};
    i32 min_width{200};
    i32 min_height{150};
    i32 max_width{0};    // 0 = no max
    i32 max_height{0};   // 0 = no max
    bool resizable{true};
    bool decorated{true};
    bool transparent{false};
    bool always_on_top{false};
    bool maximized{false};
    bool visible{true};
    f32 dpi_scale{1.0f}; // 0 = auto-detect
};

// ─────────────────────────────────────────────────────────────────────────────
// Window State
// ─────────────────────────────────────────────────────────────────────────────

enum class WindowState {
    Normal,
    Minimized,
    Maximized,
    Fullscreen,
    Hidden
};

// ─────────────────────────────────────────────────────────────────────────────
// Native Window Handle
// ─────────────────────────────────────────────────────────────────────────────

struct NativeWindowHandle {
#ifdef _WIN32
    struct {
        HWND hwnd{nullptr};
        HINSTANCE hinstance{nullptr};
    } win32;
#else
    struct {
        void* display{nullptr};  // X11 Display*
        unsigned long window{0}; // X11 Window
    } x11;
    struct {
        void* display{nullptr};  // wl_display*
        void* surface{nullptr};  // wl_surface*
    } wayland;
#endif
};

// ─────────────────────────────────────────────────────────────────────────────
// Platform Window Interface
// ─────────────────────────────────────────────────────────────────────────────

class PlatformWindow : public NonCopyable {
public:
    virtual ~PlatformWindow() = default;

    // ─────────────────────────────────────────────────────────────────────────
    // Factory
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] static Result<Unique<PlatformWindow>> create(const WindowConfig& config);

    // ─────────────────────────────────────────────────────────────────────────
    // Properties
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] virtual Size2D size() const = 0;
    [[nodiscard]] virtual Size2D framebuffer_size() const = 0;
    [[nodiscard]] virtual Point2D position() const = 0;
    [[nodiscard]] virtual f32 dpi_scale() const = 0;
    [[nodiscard]] virtual WindowState state() const = 0;
    [[nodiscard]] virtual bool is_focused() const = 0;
    [[nodiscard]] virtual bool should_close() const = 0;
    [[nodiscard]] virtual NativeWindowHandle native_handle() const = 0;

    // ─────────────────────────────────────────────────────────────────────────
    // Modifiers
    // ─────────────────────────────────────────────────────────────────────────

    virtual void set_title(StringView title) = 0;
    virtual void set_size(i32 width, i32 height) = 0;
    virtual void set_position(i32 x, i32 y) = 0;
    virtual void set_min_size(i32 width, i32 height) = 0;
    virtual void set_max_size(i32 width, i32 height) = 0;
    virtual void set_visible(bool visible) = 0;
    virtual void set_resizable(bool resizable) = 0;
    virtual void set_decorated(bool decorated) = 0;

    virtual void minimize() = 0;
    virtual void maximize() = 0;
    virtual void restore() = 0;
    virtual void focus() = 0;
    virtual void request_close() = 0;

    // ─────────────────────────────────────────────────────────────────────────
    // Event Loop
    // ─────────────────────────────────────────────────────────────────────────

    virtual void poll_events() = 0;
    virtual void wait_events() = 0;
    virtual void wait_events_timeout(f64 timeout_seconds) = 0;

    // ─────────────────────────────────────────────────────────────────────────
    // Cursor
    // ─────────────────────────────────────────────────────────────────────────

    virtual void set_cursor(CursorType cursor) = 0;
    virtual void set_cursor_visible(bool visible) = 0;
    virtual Point2D cursor_position() const = 0;

    // ─────────────────────────────────────────────────────────────────────────
    // Rendering
    // ─────────────────────────────────────────────────────────────────────────

    /// Present pixels to the window (software rendering)
    /// @param pixels RGBA8 pixel data (width * height * 4 bytes)
    /// @param width Width of the pixel data
    /// @param height Height of the pixel data
    virtual void present_pixels(const u8* pixels, i32 width, i32 height) = 0;

    // ─────────────────────────────────────────────────────────────────────────
    // Signals
    // ─────────────────────────────────────────────────────────────────────────

    Signal<i32, i32> on_resize;          // width, height
    Signal<i32, i32> on_framebuffer_resize;  // width, height
    Signal<i32, i32> on_move;            // x, y
    Signal<> on_close_requested;
    Signal<bool> on_focus_changed;       // focused
    Signal<f32> on_dpi_changed;          // new scale

    Signal<KeyCode, KeyAction, ModifierFlags> on_key;
    Signal<u32> on_char;                 // Unicode codepoint
    Signal<f64, f64> on_mouse_move;      // x, y
    Signal<MouseButton, KeyAction, ModifierFlags> on_mouse_button;
    Signal<f64, f64> on_scroll;          // x, y offset
    Signal<Vector<String>> on_drop;      // file paths

protected:
    PlatformWindow() = default;
};

// ─────────────────────────────────────────────────────────────────────────────
// Platform Utilities
// ─────────────────────────────────────────────────────────────────────────────

namespace platform {

// Initialize the platform subsystem (called once at startup)
Result<void> initialize();

// Shutdown the platform subsystem
void shutdown();

// Get the primary monitor's DPI scale
f32 get_primary_monitor_dpi_scale();

// Get display refresh rate
f32 get_primary_monitor_refresh_rate();

// Get all monitor info
struct MonitorInfo {
    String name;
    Rect bounds;
    Rect work_area;
    f32 dpi_scale;
    f32 refresh_rate;
    bool is_primary;
};

Vector<MonitorInfo> get_monitors();

} // namespace platform

} // namespace frost
