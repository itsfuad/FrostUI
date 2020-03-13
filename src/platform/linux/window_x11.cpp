#include "frost/platform/window.hpp"
#include "frost/platform/input.hpp"

#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <cstring>
#include <cstdlib>
#include <sys/select.h>
#include <unistd.h>

// X11 defines None, Button4, Button5 as macros which conflict with our enums
#ifdef None
#undef None
#endif
#ifdef Button4
#undef Button4
#endif
#ifdef Button5
#undef Button5
#endif

namespace frost {

// Key translation table from X11 keysym to our KeyCode
static KeyCode keysym_to_keycode(KeySym keysym) {
    switch (keysym) {
        // Letters
        case XK_a: case XK_A: return KeyCode::A;
        case XK_b: case XK_B: return KeyCode::B;
        case XK_c: case XK_C: return KeyCode::C;
        case XK_d: case XK_D: return KeyCode::D;
        case XK_e: case XK_E: return KeyCode::E;
        case XK_f: case XK_F: return KeyCode::F;
        case XK_g: case XK_G: return KeyCode::G;
        case XK_h: case XK_H: return KeyCode::H;
        case XK_i: case XK_I: return KeyCode::I;
        case XK_j: case XK_J: return KeyCode::J;
        case XK_k: case XK_K: return KeyCode::K;
        case XK_l: case XK_L: return KeyCode::L;
        case XK_m: case XK_M: return KeyCode::M;
        case XK_n: case XK_N: return KeyCode::N;
        case XK_o: case XK_O: return KeyCode::O;
        case XK_p: case XK_P: return KeyCode::P;
        case XK_q: case XK_Q: return KeyCode::Q;
        case XK_r: case XK_R: return KeyCode::R;
        case XK_s: case XK_S: return KeyCode::S;
        case XK_t: case XK_T: return KeyCode::T;
        case XK_u: case XK_U: return KeyCode::U;
        case XK_v: case XK_V: return KeyCode::V;
        case XK_w: case XK_W: return KeyCode::W;
        case XK_x: case XK_X: return KeyCode::X;
        case XK_y: case XK_Y: return KeyCode::Y;
        case XK_z: case XK_Z: return KeyCode::Z;

        // Numbers
        case XK_1: return KeyCode::Num1;
        case XK_2: return KeyCode::Num2;
        case XK_3: return KeyCode::Num3;
        case XK_4: return KeyCode::Num4;
        case XK_5: return KeyCode::Num5;
        case XK_6: return KeyCode::Num6;
        case XK_7: return KeyCode::Num7;
        case XK_8: return KeyCode::Num8;
        case XK_9: return KeyCode::Num9;
        case XK_0: return KeyCode::Num0;

        // Function keys
        case XK_F1: return KeyCode::F1;
        case XK_F2: return KeyCode::F2;
        case XK_F3: return KeyCode::F3;
        case XK_F4: return KeyCode::F4;
        case XK_F5: return KeyCode::F5;
        case XK_F6: return KeyCode::F6;
        case XK_F7: return KeyCode::F7;
        case XK_F8: return KeyCode::F8;
        case XK_F9: return KeyCode::F9;
        case XK_F10: return KeyCode::F10;
        case XK_F11: return KeyCode::F11;
        case XK_F12: return KeyCode::F12;

        // Control keys
        case XK_Return: return KeyCode::Return;
        case XK_Escape: return KeyCode::Escape;
        case XK_BackSpace: return KeyCode::Backspace;
        case XK_Tab: return KeyCode::Tab;
        case XK_space: return KeyCode::Space;

        // Punctuation
        case XK_minus: return KeyCode::Minus;
        case XK_equal: return KeyCode::Equals;
        case XK_bracketleft: return KeyCode::LeftBracket;
        case XK_bracketright: return KeyCode::RightBracket;
        case XK_backslash: return KeyCode::Backslash;
        case XK_semicolon: return KeyCode::Semicolon;
        case XK_apostrophe: return KeyCode::Apostrophe;
        case XK_grave: return KeyCode::Grave;
        case XK_comma: return KeyCode::Comma;
        case XK_period: return KeyCode::Period;
        case XK_slash: return KeyCode::Slash;

        // Navigation
        case XK_Insert: return KeyCode::Insert;
        case XK_Delete: return KeyCode::Delete;
        case XK_Home: return KeyCode::Home;
        case XK_End: return KeyCode::End;
        case XK_Page_Up: return KeyCode::PageUp;
        case XK_Page_Down: return KeyCode::PageDown;

        // Arrow keys
        case XK_Left: return KeyCode::Left;
        case XK_Right: return KeyCode::Right;
        case XK_Up: return KeyCode::Up;
        case XK_Down: return KeyCode::Down;

        // Lock keys
        case XK_Caps_Lock: return KeyCode::CapsLock;
        case XK_Scroll_Lock: return KeyCode::ScrollLock;
        case XK_Num_Lock: return KeyCode::NumLock;

        // Modifiers
        case XK_Shift_L: return KeyCode::LeftShift;
        case XK_Shift_R: return KeyCode::RightShift;
        case XK_Control_L: return KeyCode::LeftControl;
        case XK_Control_R: return KeyCode::RightControl;
        case XK_Alt_L: return KeyCode::LeftAlt;
        case XK_Alt_R: return KeyCode::RightAlt;
        case XK_Super_L: return KeyCode::LeftSuper;
        case XK_Super_R: return KeyCode::RightSuper;

        // Numpad
        case XK_KP_Divide: return KeyCode::NumpadDivide;
        case XK_KP_Multiply: return KeyCode::NumpadMultiply;
        case XK_KP_Subtract: return KeyCode::NumpadMinus;
        case XK_KP_Add: return KeyCode::NumpadPlus;
        case XK_KP_Enter: return KeyCode::NumpadEnter;
        case XK_KP_1: case XK_KP_End: return KeyCode::Numpad1;
        case XK_KP_2: case XK_KP_Down: return KeyCode::Numpad2;
        case XK_KP_3: case XK_KP_Page_Down: return KeyCode::Numpad3;
        case XK_KP_4: case XK_KP_Left: return KeyCode::Numpad4;
        case XK_KP_5: case XK_KP_Begin: return KeyCode::Numpad5;
        case XK_KP_6: case XK_KP_Right: return KeyCode::Numpad6;
        case XK_KP_7: case XK_KP_Home: return KeyCode::Numpad7;
        case XK_KP_8: case XK_KP_Up: return KeyCode::Numpad8;
        case XK_KP_9: case XK_KP_Page_Up: return KeyCode::Numpad9;
        case XK_KP_0: case XK_KP_Insert: return KeyCode::Numpad0;
        case XK_KP_Decimal: case XK_KP_Delete: return KeyCode::NumpadDecimal;

        // Other
        case XK_Print: return KeyCode::PrintScreen;
        case XK_Pause: return KeyCode::Pause;

        default: return KeyCode::Unknown;
    }
}

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
    void create_cursors();
    void destroy_cursors();
    f32 calculate_dpi_scale();

    Display* display_{nullptr};
    ::Window window_{0};
    XIM xim_{nullptr};
    XIC xic_{nullptr};

    Atom wm_delete_window_{0};
    Atom wm_state_{0};
    Atom wm_state_maximized_vert_{0};
    Atom wm_state_maximized_horz_{0};
    Atom wm_state_fullscreen_{0};
    Atom net_wm_state_{0};
    Atom motif_wm_hints_{0};

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
    bool visible_{false};
    WindowState state_{WindowState::Normal};

    InputState input_state_;
    ::Cursor cursors_[11]{};
    CursorType current_cursor_{CursorType::Arrow};
    bool cursor_visible_{true};
};

X11Window::X11Window(const WindowConfig& config)
    : width_(config.width)
    , height_(config.height)
    , min_width_(config.min_width)
    , min_height_(config.min_height)
    , max_width_(config.max_width)
    , max_height_(config.max_height)
    , resizable_(config.resizable) {

    // Open display connection
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        throw std::runtime_error("Failed to open X11 display");
    }

    int screen = DefaultScreen(display_);
    ::Window root = RootWindow(display_, screen);

    // Calculate DPI scale
    dpi_scale_ = config.dpi_scale > 0.0f ? config.dpi_scale : calculate_dpi_scale();

    // Create window
    XSetWindowAttributes attrs{};
    attrs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
                       ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                       StructureNotifyMask | FocusChangeMask | EnterWindowMask |
                       LeaveWindowMask | PropertyChangeMask;
    attrs.background_pixel = BlackPixel(display_, screen);

    window_ = XCreateWindow(
        display_, root,
        0, 0, width_, height_,
        0,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        CWEventMask | CWBackPixel,
        &attrs
    );

    if (!window_) {
        XCloseDisplay(display_);
        throw std::runtime_error("Failed to create X11 window");
    }

    // Setup WM protocols
    wm_delete_window_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    wm_state_ = XInternAtom(display_, "WM_STATE", False);
    net_wm_state_ = XInternAtom(display_, "_NET_WM_STATE", False);
    wm_state_maximized_vert_ = XInternAtom(display_, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    wm_state_maximized_horz_ = XInternAtom(display_, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    wm_state_fullscreen_ = XInternAtom(display_, "_NET_WM_STATE_FULLSCREEN", False);
    motif_wm_hints_ = XInternAtom(display_, "_MOTIF_WM_HINTS", False);

    XSetWMProtocols(display_, window_, &wm_delete_window_, 1);

    // Set window title
    set_title(config.title);

    // Set size hints
    update_size_hints();

    // Set up input method for text input
    xim_ = XOpenIM(display_, nullptr, nullptr, nullptr);
    if (xim_) {
        xic_ = XCreateIC(xim_,
            XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
            XNClientWindow, window_,
            XNFocusWindow, window_,
            nullptr
        );
    }

    // Create cursor shapes
    create_cursors();

    // Apply decorations setting
    if (!config.decorated) {
        set_decorated(false);
    }

    // Show window if configured
    if (config.visible) {
        set_visible(true);
    }

    // Maximize if configured
    if (config.maximized) {
        maximize();
    }

    XFlush(display_);
}

X11Window::~X11Window() {
    destroy_cursors();

    if (xic_) {
        XDestroyIC(xic_);
    }
    if (xim_) {
        XCloseIM(xim_);
    }
    if (window_ && display_) {
        XDestroyWindow(display_, window_);
    }
    if (display_) {
        XCloseDisplay(display_);
    }
}

NativeWindowHandle X11Window::native_handle() const {
    NativeWindowHandle handle{};
    handle.x11.display = display_;
    handle.x11.window = static_cast<unsigned long>(window_);
    return handle;
}

f32 X11Window::calculate_dpi_scale() {
    // Try to get DPI from Xft.dpi resource
    char* resource = XResourceManagerString(display_);
    if (resource) {
        char* dpi_str = strstr(resource, "Xft.dpi:");
        if (dpi_str) {
            dpi_str += 8; // Skip "Xft.dpi:"
            while (*dpi_str == ' ' || *dpi_str == '\t') dpi_str++;
            f32 dpi = static_cast<f32>(atof(dpi_str));
            if (dpi > 0) {
                return dpi / 96.0f;  // 96 DPI is standard
            }
        }
    }
    return 1.0f;
}

void X11Window::create_cursors() {
    cursors_[static_cast<int>(CursorType::Arrow)] = XCreateFontCursor(display_, XC_left_ptr);
    cursors_[static_cast<int>(CursorType::IBeam)] = XCreateFontCursor(display_, XC_xterm);
    cursors_[static_cast<int>(CursorType::Crosshair)] = XCreateFontCursor(display_, XC_crosshair);
    cursors_[static_cast<int>(CursorType::Hand)] = XCreateFontCursor(display_, XC_hand2);
    cursors_[static_cast<int>(CursorType::ResizeH)] = XCreateFontCursor(display_, XC_sb_h_double_arrow);
    cursors_[static_cast<int>(CursorType::ResizeV)] = XCreateFontCursor(display_, XC_sb_v_double_arrow);
    cursors_[static_cast<int>(CursorType::ResizeNESW)] = XCreateFontCursor(display_, XC_sizing);
    cursors_[static_cast<int>(CursorType::ResizeNWSE)] = XCreateFontCursor(display_, XC_sizing);
    cursors_[static_cast<int>(CursorType::ResizeAll)] = XCreateFontCursor(display_, XC_fleur);
    cursors_[static_cast<int>(CursorType::NotAllowed)] = XCreateFontCursor(display_, XC_X_cursor);
    // CursorType::Hidden uses no cursor
}

void X11Window::destroy_cursors() {
    for (int i = 0; i < 10; ++i) {
        if (cursors_[i]) {
            XFreeCursor(display_, cursors_[i]);
        }
    }
}

void X11Window::set_title(StringView title) {
    XStoreName(display_, window_, String(title).c_str());
    XFlush(display_);
}

void X11Window::set_size(i32 width, i32 height) {
    XResizeWindow(display_, window_, width, height);
    XFlush(display_);
}

void X11Window::set_position(i32 x, i32 y) {
    XMoveWindow(display_, window_, x, y);
    XFlush(display_);
}

void X11Window::set_min_size(i32 width, i32 height) {
    min_width_ = width;
    min_height_ = height;
    update_size_hints();
}

void X11Window::set_max_size(i32 width, i32 height) {
    max_width_ = width;
    max_height_ = height;
    update_size_hints();
}

void X11Window::update_size_hints() {
    XSizeHints* hints = XAllocSizeHints();
    if (hints) {
        hints->flags = PMinSize | PMaxSize;
        hints->min_width = min_width_;
        hints->min_height = min_height_;

        if (max_width_ > 0 && max_height_ > 0) {
            hints->max_width = max_width_;
            hints->max_height = max_height_;
        } else {
            hints->max_width = 32767;
            hints->max_height = 32767;
        }

        if (!resizable_) {
            hints->flags |= PMinSize | PMaxSize;
            hints->min_width = width_;
            hints->min_height = height_;
            hints->max_width = width_;
            hints->max_height = height_;
        }

        XSetWMNormalHints(display_, window_, hints);
        XFree(hints);
        XFlush(display_);
    }
}

void X11Window::set_visible(bool visible) {
    if (visible) {
        XMapWindow(display_, window_);
        state_ = WindowState::Normal;
    } else {
        XUnmapWindow(display_, window_);
        state_ = WindowState::Hidden;
    }
    visible_ = visible;
    XFlush(display_);
}

void X11Window::set_resizable(bool resizable) {
    resizable_ = resizable;
    update_size_hints();
}

void X11Window::set_decorated(bool decorated) {
    struct {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long input_mode;
        unsigned long status;
    } hints{};

    hints.flags = 2;  // MWM_HINTS_DECORATIONS
    hints.decorations = decorated ? 1 : 0;

    XChangeProperty(display_, window_, motif_wm_hints_, motif_wm_hints_, 32,
                    PropModeReplace, reinterpret_cast<unsigned char*>(&hints), 5);
    XFlush(display_);
}

void X11Window::minimize() {
    XIconifyWindow(display_, window_, DefaultScreen(display_));
    state_ = WindowState::Minimized;
    XFlush(display_);
}

void X11Window::maximize() {
    XEvent event{};
    event.type = ClientMessage;
    event.xclient.window = window_;
    event.xclient.message_type = net_wm_state_;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 1;  // _NET_WM_STATE_ADD
    event.xclient.data.l[1] = wm_state_maximized_vert_;
    event.xclient.data.l[2] = wm_state_maximized_horz_;
    event.xclient.data.l[3] = 0;
    event.xclient.data.l[4] = 0;

    XSendEvent(display_, DefaultRootWindow(display_), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &event);
    state_ = WindowState::Maximized;
    XFlush(display_);
}

void X11Window::restore() {
    XEvent event{};
    event.type = ClientMessage;
    event.xclient.window = window_;
    event.xclient.message_type = net_wm_state_;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 0;  // _NET_WM_STATE_REMOVE
    event.xclient.data.l[1] = wm_state_maximized_vert_;
    event.xclient.data.l[2] = wm_state_maximized_horz_;
    event.xclient.data.l[3] = 0;
    event.xclient.data.l[4] = 0;

    XSendEvent(display_, DefaultRootWindow(display_), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &event);

    if (state_ == WindowState::Minimized) {
        XMapWindow(display_, window_);
    }

    state_ = WindowState::Normal;
    XFlush(display_);
}

void X11Window::focus() {
    XRaiseWindow(display_, window_);
    XSetInputFocus(display_, window_, RevertToPointerRoot, CurrentTime);
    XFlush(display_);
}

void X11Window::request_close() {
    should_close_ = true;
    on_close_requested.emit();
}

void X11Window::poll_events() {
    while (XPending(display_)) {
        XEvent event;
        XNextEvent(display_, &event);

        // Filter for input method events
        if (xic_ && XFilterEvent(&event, window_)) {
            continue;
        }

        process_event(event);
    }
}

void X11Window::wait_events() {
    XEvent event;
    XNextEvent(display_, &event);

    if (!xic_ || !XFilterEvent(&event, window_)) {
        process_event(event);
    }

    // Process any additional pending events
    poll_events();
}

void X11Window::wait_events_timeout(f64 timeout_seconds) {
    int fd = ConnectionNumber(display_);

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    struct timeval tv;
    tv.tv_sec = static_cast<long>(timeout_seconds);
    tv.tv_usec = static_cast<long>((timeout_seconds - tv.tv_sec) * 1000000);

    if (select(fd + 1, &fds, nullptr, nullptr, &tv) > 0) {
        poll_events();
    }
}

void X11Window::process_event(XEvent& event) {
    switch (event.type) {
        case KeyPress:
        case KeyRelease: {
            KeySym keysym = XkbKeycodeToKeysym(display_, event.xkey.keycode, 0,
                                               event.xkey.state & ShiftMask ? 1 : 0);
            KeyCode key = keysym_to_keycode(keysym);
            KeyAction action = (event.type == KeyPress) ? KeyAction::Press : KeyAction::Release;
            ModifierFlags mods = translate_modifiers(event.xkey.state);

            // Update input state
            if (key != KeyCode::Unknown) {
                input_state_.keys[static_cast<u32>(key) & 0xFF] = (action == KeyAction::Press);
            }
            input_state_.modifiers = mods;

            on_key.emit(key, action, mods);

            // Handle text input for KeyPress
            if (event.type == KeyPress && xic_) {
                char buf[32];
                Status status;
                int len = Xutf8LookupString(xic_, &event.xkey, buf, sizeof(buf) - 1, &keysym, &status);
                if (len > 0 && (status == XLookupChars || status == XLookupBoth)) {
                    buf[len] = '\0';
                    // Simple ASCII character handling for now
                    // TODO: Full UTF-8 support
                    if (len == 1 && buf[0] >= 32) {
                        on_char.emit(static_cast<u32>(buf[0]));
                    }
                }
            }
            break;
        }

        case ButtonPress:
        case ButtonRelease: {
            // Scroll wheel events
            if (event.xbutton.button == 4 || event.xbutton.button == 5) {
                if (event.type == ButtonPress) {
                    f64 delta = (event.xbutton.button == 4) ? 1.0 : -1.0;
                    on_scroll.emit(0.0, delta);
                }
                break;
            }
            if (event.xbutton.button == 6 || event.xbutton.button == 7) {
                if (event.type == ButtonPress) {
                    f64 delta = (event.xbutton.button == 6) ? -1.0 : 1.0;
                    on_scroll.emit(delta, 0.0);
                }
                break;
            }

            MouseButton button = translate_mouse_button(event.xbutton.button);
            KeyAction action = (event.type == ButtonPress) ? KeyAction::Press : KeyAction::Release;
            ModifierFlags mods = translate_modifiers(event.xbutton.state);

            input_state_.mouse_buttons[static_cast<u8>(button)] = (action == KeyAction::Press);
            on_mouse_button.emit(button, action, mods);
            break;
        }

        case MotionNotify: {
            input_state_.mouse_x = event.xmotion.x;
            input_state_.mouse_y = event.xmotion.y;
            on_mouse_move.emit(static_cast<f64>(event.xmotion.x), static_cast<f64>(event.xmotion.y));
            break;
        }

        case ConfigureNotify: {
            bool size_changed = (event.xconfigure.width != width_ || event.xconfigure.height != height_);
            bool pos_changed = (event.xconfigure.x != x_ || event.xconfigure.y != y_);

            if (size_changed) {
                width_ = event.xconfigure.width;
                height_ = event.xconfigure.height;
                on_resize.emit(width_, height_);
                on_framebuffer_resize.emit(width_, height_);
            }
            if (pos_changed) {
                x_ = event.xconfigure.x;
                y_ = event.xconfigure.y;
                on_move.emit(x_, y_);
            }
            break;
        }

        case FocusIn: {
            focused_ = true;
            on_focus_changed.emit(true);
            if (xic_) {
                XSetICFocus(xic_);
            }
            break;
        }

        case FocusOut: {
            focused_ = false;
            on_focus_changed.emit(false);
            if (xic_) {
                XUnsetICFocus(xic_);
            }
            break;
        }

        case ClientMessage: {
            if (static_cast<Atom>(event.xclient.data.l[0]) == wm_delete_window_) {
                request_close();
            }
            break;
        }

        case MapNotify: {
            visible_ = true;
            break;
        }

        case UnmapNotify: {
            visible_ = false;
            break;
        }

        default:
            break;
    }
}

KeyCode X11Window::translate_keycode(unsigned int keycode, KeySym keysym) {
    return keysym_to_keycode(keysym);
}

ModifierFlags X11Window::translate_modifiers(unsigned int state) {
    ModifierFlags mods = ModifierFlags::None;

    if (state & ShiftMask) mods = mods | ModifierFlags::Shift;
    if (state & ControlMask) mods = mods | ModifierFlags::Control;
    if (state & Mod1Mask) mods = mods | ModifierFlags::Alt;
    if (state & Mod4Mask) mods = mods | ModifierFlags::Super;
    if (state & LockMask) mods = mods | ModifierFlags::CapsLock;
    if (state & Mod2Mask) mods = mods | ModifierFlags::NumLock;

    return mods;
}

MouseButton X11Window::translate_mouse_button(unsigned int button) {
    switch (button) {
        case 1: return MouseButton::Left;
        case 2: return MouseButton::Middle;
        case 3: return MouseButton::Right;
        case 8: return MouseButton::Button4;
        case 9: return MouseButton::Button5;
        default: return MouseButton::Left;
    }
}

void X11Window::set_cursor(CursorType cursor) {
    current_cursor_ = cursor;
    if (!cursor_visible_ || cursor == CursorType::Hidden) {
        // Create and set an invisible cursor
        Pixmap pixmap = XCreatePixmap(display_, window_, 1, 1, 1);
        XColor color{};
        ::Cursor invisible = XCreatePixmapCursor(display_, pixmap, pixmap, &color, &color, 0, 0);
        XDefineCursor(display_, window_, invisible);
        XFreeCursor(display_, invisible);
        XFreePixmap(display_, pixmap);
    } else {
        XDefineCursor(display_, window_, cursors_[static_cast<int>(cursor)]);
    }
    XFlush(display_);
}

void X11Window::set_cursor_visible(bool visible) {
    cursor_visible_ = visible;
    set_cursor(current_cursor_);
}

Point2D X11Window::cursor_position() const {
    return input_state_.mouse_position();
}

Result<Unique<PlatformWindow>> PlatformWindow::create(const WindowConfig& config) {
    try {
        Unique<PlatformWindow> window = make_unique<X11Window>(config);
        return window;
    } catch (const std::exception& e) {
        return Error{ErrorCode::PlatformError, e.what()};
    }
}

} // namespace frost

#endif // __linux__
