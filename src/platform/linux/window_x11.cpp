#include "frost/platform/window.hpp"
#include "frost/platform/input.hpp"

#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <cstring>

// X11 defines None as a macro which conflicts with our enum
#ifdef None
#undef None
#endif

namespace frost {

class X11Window : public PlatformWindow {
public:
    explicit X11Window(const WindowConfig& config);
    ~X11Window() override;

    // Properties
    [[nodiscard]] Size2D size() const override { return Size2D{static_cast<f32>(width_), static_cast<f32>(height_)}; }
    [[nodiscard]] Size2D framebuffer_size() const override { return size(); }
    [[nodiscard]] Point2D position() const override { return Point2D{static_cast<f32>(x_), static_cast<f32>(y_)}; }
    [[nodiscard]] f32 dpi_scale() const override { return dpi_scale_; }
    [[nodiscard]] WindowState state() const override { return state_; }
    [[nodiscard]] bool is_focused() const override { return focused_; }
    [[nodiscard]] bool should_close() const override { return should_close_; }
    [[nodiscard]] NativeWindowHandle native_handle() const override;

    // Modifiers
    void set_title(StringView title) override;
    void set_size(i32 width, i32 height) override;
    void set_position(i32 x, i32 y) override;
    void set_min_size(i32 width, i32 height) override;
    void set_max_size(i32 width, i32 height) override;
    void set_visible(bool visible) override;
    void set_resizable(bool resizable) override;
    void set_decorated(bool decorated) override;

    void minimize() override;
    void maximize() override;
    void restore() override;
    void focus() override;
    void request_close() override;

    // Event loop
    void poll_events() override;
    void wait_events() override;
    void wait_events_timeout(f64 timeout_seconds) override;

    // Cursor
    void set_cursor(CursorType cursor) override;
    void set_cursor_visible(bool visible) override;
    Point2D cursor_position() const override;

private:
    void process_event(XEvent& event);
    KeyCode translate_keycode(unsigned int keycode, KeySym keysym);
    ModifierFlags translate_modifiers(unsigned int state);
    MouseButton translate_mouse_button(unsigned int button);
    void update_size_hints();

    Display* display_{nullptr};
    ::Window window_{0};
    Atom wm_delete_window_{0};
    Atom wm_state_{0};
    Atom wm_state_maximized_vert_{0};
    Atom wm_state_maximized_horz_{0};
    Atom wm_state_fullscreen_{0};

    i32 width_{0};
    i32 height_{0};
    i32 x_{0};
    i32 y_{0};
    i32 min_width_{0};
    i32 min_height_{0};
    i32 max_width_{0};
    i32 max_height_{0};
    f32 dpi_scale_{1.0f};
    bool focused_{false};
    bool should_close_{false};
    bool resizable_{true};
    WindowState state_{WindowState::Normal};

    InputState input_state_;
    Cursor cursors_[11]{};
    CursorType current_cursor_{CursorType::Arrow};
    bool cursor_visible_{true};
};

// Implementation placeholder - will be completed in Phase 2
X11Window::X11Window(const WindowConfig& config)
    : width_(config.width)
    , height_(config.height)
    , min_width_(config.min_width)
    , min_height_(config.min_height)
    , max_width_(config.max_width)
    , max_height_(config.max_height)
    , dpi_scale_(config.dpi_scale)
    , resizable_(config.resizable) {
    // Full implementation in Phase 2
}

X11Window::~X11Window() {
    // Full implementation in Phase 2
}

NativeWindowHandle X11Window::native_handle() const {
    NativeWindowHandle handle{};
    handle.x11.display = display_;
    handle.x11.window = static_cast<unsigned long>(window_);
    return handle;
}

void X11Window::set_title(StringView title) {}
void X11Window::set_size(i32 width, i32 height) {}
void X11Window::set_position(i32 x, i32 y) {}
void X11Window::set_min_size(i32 width, i32 height) {}
void X11Window::set_max_size(i32 width, i32 height) {}
void X11Window::set_visible(bool visible) {}
void X11Window::set_resizable(bool resizable) {}
void X11Window::set_decorated(bool decorated) {}
void X11Window::minimize() {}
void X11Window::maximize() {}
void X11Window::restore() {}
void X11Window::focus() {}
void X11Window::request_close() { should_close_ = true; }
void X11Window::poll_events() {}
void X11Window::wait_events() {}
void X11Window::wait_events_timeout(f64 timeout_seconds) {}
void X11Window::set_cursor(CursorType cursor) {}
void X11Window::set_cursor_visible(bool visible) {}
Point2D X11Window::cursor_position() const { return input_state_.mouse_position(); }
void X11Window::process_event(XEvent& event) {}
KeyCode X11Window::translate_keycode(unsigned int keycode, KeySym keysym) { return KeyCode::Unknown; }
ModifierFlags X11Window::translate_modifiers(unsigned int state) { return ModifierFlags::None; }
MouseButton X11Window::translate_mouse_button(unsigned int button) { return MouseButton::Left; }
void X11Window::update_size_hints() {}

Result<Unique<PlatformWindow>> PlatformWindow::create(const WindowConfig& config) {
    Unique<PlatformWindow> window = make_unique<X11Window>(config);
    return window;
}

} // namespace frost

#endif // __linux__
