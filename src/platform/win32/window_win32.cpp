#include "frost/platform/window.hpp"
#include "frost/platform/input.hpp"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <Windows.h>
#include <windowsx.h>
#include <ShellScalingApi.h>
#include <map>
#include <string>

#pragma comment(lib, "Shcore.lib")

namespace frost {

// ─────────────────────────────────────────────────────────────────────────────
// Virtual Key to KeyCode Mapping
// ─────────────────────────────────────────────────────────────────────────────

static KeyCode vk_to_keycode(WPARAM vk, LPARAM lp) {
    // Handle extended keys
    bool extended = (lp & 0x01000000) != 0;
    UINT scancode = (lp >> 16) & 0xFF;

    switch (vk) {
        // Letters
        case 'A': return KeyCode::A;
        case 'B': return KeyCode::B;
        case 'C': return KeyCode::C;
        case 'D': return KeyCode::D;
        case 'E': return KeyCode::E;
        case 'F': return KeyCode::F;
        case 'G': return KeyCode::G;
        case 'H': return KeyCode::H;
        case 'I': return KeyCode::I;
        case 'J': return KeyCode::J;
        case 'K': return KeyCode::K;
        case 'L': return KeyCode::L;
        case 'M': return KeyCode::M;
        case 'N': return KeyCode::N;
        case 'O': return KeyCode::O;
        case 'P': return KeyCode::P;
        case 'Q': return KeyCode::Q;
        case 'R': return KeyCode::R;
        case 'S': return KeyCode::S;
        case 'T': return KeyCode::T;
        case 'U': return KeyCode::U;
        case 'V': return KeyCode::V;
        case 'W': return KeyCode::W;
        case 'X': return KeyCode::X;
        case 'Y': return KeyCode::Y;
        case 'Z': return KeyCode::Z;

        // Numbers (top row)
        case '0': return KeyCode::Num0;
        case '1': return KeyCode::Num1;
        case '2': return KeyCode::Num2;
        case '3': return KeyCode::Num3;
        case '4': return KeyCode::Num4;
        case '5': return KeyCode::Num5;
        case '6': return KeyCode::Num6;
        case '7': return KeyCode::Num7;
        case '8': return KeyCode::Num8;
        case '9': return KeyCode::Num9;

        // Function keys
        case VK_F1:  return KeyCode::F1;
        case VK_F2:  return KeyCode::F2;
        case VK_F3:  return KeyCode::F3;
        case VK_F4:  return KeyCode::F4;
        case VK_F5:  return KeyCode::F5;
        case VK_F6:  return KeyCode::F6;
        case VK_F7:  return KeyCode::F7;
        case VK_F8:  return KeyCode::F8;
        case VK_F9:  return KeyCode::F9;
        case VK_F10: return KeyCode::F10;
        case VK_F11: return KeyCode::F11;
        case VK_F12: return KeyCode::F12;

        // Control keys
        case VK_RETURN:    return extended ? KeyCode::NumpadEnter : KeyCode::Return;
        case VK_ESCAPE:    return KeyCode::Escape;
        case VK_BACK:      return KeyCode::Backspace;
        case VK_TAB:       return KeyCode::Tab;
        case VK_SPACE:     return KeyCode::Space;

        // Navigation keys
        case VK_INSERT:    return KeyCode::Insert;
        case VK_DELETE:    return KeyCode::Delete;
        case VK_HOME:      return KeyCode::Home;
        case VK_END:       return KeyCode::End;
        case VK_PRIOR:     return KeyCode::PageUp;
        case VK_NEXT:      return KeyCode::PageDown;

        // Arrow keys
        case VK_UP:        return KeyCode::Up;
        case VK_DOWN:      return KeyCode::Down;
        case VK_LEFT:      return KeyCode::Left;
        case VK_RIGHT:     return KeyCode::Right;

        // Modifier keys - distinguish left/right
        case VK_SHIFT:
            return (scancode == 0x36) ? KeyCode::RightShift : KeyCode::LeftShift;
        case VK_CONTROL:
            return extended ? KeyCode::RightControl : KeyCode::LeftControl;
        case VK_MENU:
            return extended ? KeyCode::RightAlt : KeyCode::LeftAlt;
        case VK_LWIN:      return KeyCode::LeftSuper;
        case VK_RWIN:      return KeyCode::RightSuper;

        // Lock keys
        case VK_CAPITAL:   return KeyCode::CapsLock;
        case VK_NUMLOCK:   return KeyCode::NumLock;
        case VK_SCROLL:    return KeyCode::ScrollLock;

        // Punctuation
        case VK_OEM_MINUS:   return KeyCode::Minus;
        case VK_OEM_PLUS:    return KeyCode::Equals;
        case VK_OEM_4:       return KeyCode::LeftBracket;
        case VK_OEM_6:       return KeyCode::RightBracket;
        case VK_OEM_5:       return KeyCode::Backslash;
        case VK_OEM_1:       return KeyCode::Semicolon;
        case VK_OEM_7:       return KeyCode::Apostrophe;
        case VK_OEM_3:       return KeyCode::Grave;
        case VK_OEM_COMMA:   return KeyCode::Comma;
        case VK_OEM_PERIOD:  return KeyCode::Period;
        case VK_OEM_2:       return KeyCode::Slash;

        // Numpad
        case VK_NUMPAD0:   return KeyCode::Numpad0;
        case VK_NUMPAD1:   return KeyCode::Numpad1;
        case VK_NUMPAD2:   return KeyCode::Numpad2;
        case VK_NUMPAD3:   return KeyCode::Numpad3;
        case VK_NUMPAD4:   return KeyCode::Numpad4;
        case VK_NUMPAD5:   return KeyCode::Numpad5;
        case VK_NUMPAD6:   return KeyCode::Numpad6;
        case VK_NUMPAD7:   return KeyCode::Numpad7;
        case VK_NUMPAD8:   return KeyCode::Numpad8;
        case VK_NUMPAD9:   return KeyCode::Numpad9;
        case VK_DECIMAL:   return KeyCode::NumpadDecimal;
        case VK_DIVIDE:    return KeyCode::NumpadDivide;
        case VK_MULTIPLY:  return KeyCode::NumpadMultiply;
        case VK_SUBTRACT:  return KeyCode::NumpadMinus;
        case VK_ADD:       return KeyCode::NumpadPlus;

        case VK_SNAPSHOT:  return KeyCode::PrintScreen;
        case VK_PAUSE:     return KeyCode::Pause;

        default:
            return KeyCode::Unknown;
    }
}

static ModifierFlags get_current_modifiers() {
    ModifierFlags mods = ModifierFlags::None;

    if (GetKeyState(VK_SHIFT) & 0x8000)
        mods = mods | ModifierFlags::Shift;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        mods = mods | ModifierFlags::Control;
    if (GetKeyState(VK_MENU) & 0x8000)
        mods = mods | ModifierFlags::Alt;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
        mods = mods | ModifierFlags::Super;
    if (GetKeyState(VK_CAPITAL) & 0x0001)
        mods = mods | ModifierFlags::CapsLock;
    if (GetKeyState(VK_NUMLOCK) & 0x0001)
        mods = mods | ModifierFlags::NumLock;

    return mods;
}

// ─────────────────────────────────────────────────────────────────────────────
// Win32 Window Implementation
// ─────────────────────────────────────────────────────────────────────────────

class Win32Window : public PlatformWindow {
public:
    explicit Win32Window(const WindowConfig& config);
    ~Win32Window() override;

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
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT handle_message(UINT msg, WPARAM wparam, LPARAM lparam);
    void update_window_style();
    void load_cursors();
    DWORD get_window_style() const;
    DWORD get_window_ex_style() const;

    HWND hwnd_{nullptr};
    HINSTANCE hinstance_{nullptr};

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
    bool decorated_{true};
    WindowState state_{WindowState::Normal};

    // For fullscreen restoration
    WINDOWPLACEMENT saved_placement_{sizeof(WINDOWPLACEMENT)};
    DWORD saved_style_{0};
    DWORD saved_ex_style_{0};

    InputState input_state_;
    HCURSOR cursors_[11]{};
    CursorType current_cursor_{CursorType::Arrow};
    bool cursor_visible_{true};

    static const wchar_t* CLASS_NAME;
    static bool class_registered_;
};

const wchar_t* Win32Window::CLASS_NAME = L"FrostUIWindow";
bool Win32Window::class_registered_ = false;

Win32Window::Win32Window(const WindowConfig& config)
    : width_(config.width)
    , height_(config.height)
    , min_width_(config.min_width)
    , min_height_(config.min_height)
    , max_width_(config.max_width)
    , max_height_(config.max_height)
    , resizable_(config.resizable)
    , decorated_(config.decorated) {

    hinstance_ = GetModuleHandle(nullptr);

    // Register window class once
    if (!class_registered_) {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = Win32Window::WindowProc;
        wc.hInstance = hinstance_;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr;
        wc.lpszClassName = CLASS_NAME;
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

        if (!RegisterClassExW(&wc)) {
            return;  // Error - window creation will fail
        }
        class_registered_ = true;
    }

    load_cursors();

    DWORD style = get_window_style();
    DWORD ex_style = get_window_ex_style();

    // Calculate window size including borders
    RECT rect = {0, 0, width_, height_};
    AdjustWindowRectEx(&rect, style, FALSE, ex_style);

    int window_width = rect.right - rect.left;
    int window_height = rect.bottom - rect.top;

    // Center on primary monitor
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    x_ = (screen_width - window_width) / 2;
    y_ = (screen_height - window_height) / 2;

    // Convert title to wide string
    int title_len = MultiByteToWideChar(CP_UTF8, 0, config.title.c_str(),
                                        static_cast<int>(config.title.size()), nullptr, 0);
    std::wstring title_wide(static_cast<size_t>(title_len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, config.title.c_str(),
                        static_cast<int>(config.title.size()), title_wide.data(), title_len);

    hwnd_ = CreateWindowExW(
        ex_style,
        CLASS_NAME,
        title_wide.c_str(),
        style,
        x_, y_,
        window_width, window_height,
        nullptr,
        nullptr,
        hinstance_,
        this
    );

    if (!hwnd_) {
        return;
    }

    // Get actual DPI
    UINT dpi = GetDpiForWindow(hwnd_);
    dpi_scale_ = (config.dpi_scale > 0.0f) ? config.dpi_scale : (static_cast<f32>(dpi) / 96.0f);

    // Handle initial visibility and state
    if (config.visible) {
        if (config.maximized) {
            ShowWindow(hwnd_, SW_SHOWMAXIMIZED);
            state_ = WindowState::Maximized;
        } else {
            ShowWindow(hwnd_, SW_SHOW);
            state_ = WindowState::Normal;
        }
    } else {
        state_ = WindowState::Hidden;
    }

    UpdateWindow(hwnd_);
    focused_ = (GetForegroundWindow() == hwnd_);
}

Win32Window::~Win32Window() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

NativeWindowHandle Win32Window::native_handle() const {
    NativeWindowHandle handle{};
    handle.win32.hwnd = hwnd_;
    handle.win32.hinstance = hinstance_;
    return handle;
}

DWORD Win32Window::get_window_style() const {
    DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (decorated_) {
        style |= WS_OVERLAPPEDWINDOW;
        if (!resizable_) {
            style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
        }
    } else {
        style |= WS_POPUP;
    }

    return style;
}

DWORD Win32Window::get_window_ex_style() const {
    DWORD ex_style = WS_EX_APPWINDOW;
    return ex_style;
}

void Win32Window::update_window_style() {
    DWORD style = get_window_style();
    DWORD ex_style = get_window_ex_style();

    SetWindowLong(hwnd_, GWL_STYLE, style);
    SetWindowLong(hwnd_, GWL_EXSTYLE, ex_style);

    // Recalculate window size
    RECT rect = {0, 0, width_, height_};
    AdjustWindowRectEx(&rect, style, FALSE, ex_style);

    SetWindowPos(hwnd_, nullptr, 0, 0,
                 rect.right - rect.left, rect.bottom - rect.top,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void Win32Window::load_cursors() {
    cursors_[static_cast<int>(CursorType::Arrow)]      = LoadCursor(nullptr, IDC_ARROW);
    cursors_[static_cast<int>(CursorType::IBeam)]      = LoadCursor(nullptr, IDC_IBEAM);
    cursors_[static_cast<int>(CursorType::Crosshair)]  = LoadCursor(nullptr, IDC_CROSS);
    cursors_[static_cast<int>(CursorType::Hand)]       = LoadCursor(nullptr, IDC_HAND);
    cursors_[static_cast<int>(CursorType::ResizeH)]    = LoadCursor(nullptr, IDC_SIZEWE);
    cursors_[static_cast<int>(CursorType::ResizeV)]    = LoadCursor(nullptr, IDC_SIZENS);
    cursors_[static_cast<int>(CursorType::ResizeNESW)] = LoadCursor(nullptr, IDC_SIZENESW);
    cursors_[static_cast<int>(CursorType::ResizeNWSE)] = LoadCursor(nullptr, IDC_SIZENWSE);
    cursors_[static_cast<int>(CursorType::ResizeAll)]  = LoadCursor(nullptr, IDC_SIZEALL);
    cursors_[static_cast<int>(CursorType::NotAllowed)] = LoadCursor(nullptr, IDC_NO);
    cursors_[static_cast<int>(CursorType::Hidden)]     = nullptr;
}

LRESULT CALLBACK Win32Window::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    Win32Window* window = nullptr;

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);
        window = static_cast<Win32Window*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        window->hwnd_ = hwnd;
    } else {
        window = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window) {
        return window->handle_message(msg, wparam, lparam);
    }

    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

LRESULT Win32Window::handle_message(UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_CLOSE:
            on_close_requested.emit();
            should_close_ = true;
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE: {
            int new_width = LOWORD(lparam);
            int new_height = HIWORD(lparam);

            if (wparam == SIZE_MINIMIZED) {
                state_ = WindowState::Minimized;
            } else if (wparam == SIZE_MAXIMIZED) {
                state_ = WindowState::Maximized;
            } else if (wparam == SIZE_RESTORED) {
                state_ = WindowState::Normal;
            }

            if (new_width != width_ || new_height != height_) {
                width_ = new_width;
                height_ = new_height;
                on_resize.emit(width_, height_);
                on_framebuffer_resize.emit(width_, height_);
            }
            return 0;
        }

        case WM_MOVE: {
            x_ = static_cast<short>(LOWORD(lparam));
            y_ = static_cast<short>(HIWORD(lparam));
            on_move.emit(x_, y_);
            return 0;
        }

        case WM_GETMINMAXINFO: {
            auto* mmi = reinterpret_cast<MINMAXINFO*>(lparam);

            DWORD style = get_window_style();
            DWORD ex_style = get_window_ex_style();

            if (min_width_ > 0 || min_height_ > 0) {
                RECT rect = {0, 0, min_width_, min_height_};
                AdjustWindowRectEx(&rect, style, FALSE, ex_style);
                mmi->ptMinTrackSize.x = rect.right - rect.left;
                mmi->ptMinTrackSize.y = rect.bottom - rect.top;
            }

            if (max_width_ > 0 || max_height_ > 0) {
                RECT rect = {0, 0,
                    max_width_ > 0 ? max_width_ : 32767,
                    max_height_ > 0 ? max_height_ : 32767};
                AdjustWindowRectEx(&rect, style, FALSE, ex_style);
                mmi->ptMaxTrackSize.x = rect.right - rect.left;
                mmi->ptMaxTrackSize.y = rect.bottom - rect.top;
            }
            return 0;
        }

        case WM_SETFOCUS:
            focused_ = true;
            on_focus_changed.emit(true);
            return 0;

        case WM_KILLFOCUS:
            focused_ = false;
            on_focus_changed.emit(false);
            return 0;

        case WM_MOUSEMOVE: {
            f64 x = static_cast<f64>(GET_X_LPARAM(lparam));
            f64 y = static_cast<f64>(GET_Y_LPARAM(lparam));
            input_state_.mouse_x = x;
            input_state_.mouse_y = y;
            on_mouse_move.emit(x, y);
            return 0;
        }

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP: {
            MouseButton button = MouseButton::Left;
            KeyAction action = (msg == WM_LBUTTONDOWN) ? KeyAction::Press : KeyAction::Release;
            ModifierFlags mods = get_current_modifiers();
            input_state_.mouse_buttons[0] = (action == KeyAction::Press);
            on_mouse_button.emit(button, action, mods);

            if (msg == WM_LBUTTONDOWN) {
                SetCapture(hwnd_);
            } else {
                ReleaseCapture();
            }
            return 0;
        }

        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP: {
            MouseButton button = MouseButton::Right;
            KeyAction action = (msg == WM_RBUTTONDOWN) ? KeyAction::Press : KeyAction::Release;
            ModifierFlags mods = get_current_modifiers();
            input_state_.mouse_buttons[1] = (action == KeyAction::Press);
            on_mouse_button.emit(button, action, mods);
            return 0;
        }

        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP: {
            MouseButton button = MouseButton::Middle;
            KeyAction action = (msg == WM_MBUTTONDOWN) ? KeyAction::Press : KeyAction::Release;
            ModifierFlags mods = get_current_modifiers();
            input_state_.mouse_buttons[2] = (action == KeyAction::Press);
            on_mouse_button.emit(button, action, mods);
            return 0;
        }

        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP: {
            int btn = GET_XBUTTON_WPARAM(wparam);
            MouseButton button = (btn == XBUTTON1) ? MouseButton::Button4 : MouseButton::Button5;
            KeyAction action = (msg == WM_XBUTTONDOWN) ? KeyAction::Press : KeyAction::Release;
            ModifierFlags mods = get_current_modifiers();
            int idx = (btn == XBUTTON1) ? 3 : 4;
            input_state_.mouse_buttons[idx] = (action == KeyAction::Press);
            on_mouse_button.emit(button, action, mods);
            return TRUE;
        }

        case WM_MOUSEWHEEL: {
            f64 delta = static_cast<f64>(GET_WHEEL_DELTA_WPARAM(wparam)) / WHEEL_DELTA;
            on_scroll.emit(0.0, delta);
            return 0;
        }

        case WM_MOUSEHWHEEL: {
            f64 delta = static_cast<f64>(GET_WHEEL_DELTA_WPARAM(wparam)) / WHEEL_DELTA;
            on_scroll.emit(delta, 0.0);
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            KeyCode key = vk_to_keycode(wparam, lparam);
            KeyAction action = (lparam & 0x40000000) ? KeyAction::Repeat : KeyAction::Press;
            ModifierFlags mods = get_current_modifiers();
            if (key != KeyCode::Unknown) {
                input_state_.keys[static_cast<u32>(key) & 0xFF] = true;
            }
            on_key.emit(key, action, mods);
            return 0;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP: {
            KeyCode key = vk_to_keycode(wparam, lparam);
            ModifierFlags mods = get_current_modifiers();
            if (key != KeyCode::Unknown) {
                input_state_.keys[static_cast<u32>(key) & 0xFF] = false;
            }
            on_key.emit(key, KeyAction::Release, mods);
            return 0;
        }

        case WM_CHAR:
        case WM_SYSCHAR: {
            u32 codepoint = static_cast<u32>(wparam);
            if (codepoint >= 32) {
                on_char.emit(codepoint);
            }
            return 0;
        }

        case WM_DPICHANGED: {
            UINT dpi = HIWORD(wparam);
            f32 new_scale = static_cast<f32>(dpi) / 96.0f;
            if (new_scale != dpi_scale_) {
                dpi_scale_ = new_scale;
                on_dpi_changed.emit(dpi_scale_);
            }

            auto* rect = reinterpret_cast<RECT*>(lparam);
            SetWindowPos(hwnd_, nullptr,
                rect->left, rect->top,
                rect->right - rect->left, rect->bottom - rect->top,
                SWP_NOZORDER | SWP_NOACTIVATE);
            return 0;
        }

        case WM_SETCURSOR: {
            if (LOWORD(lparam) == HTCLIENT) {
                if (cursor_visible_ && current_cursor_ != CursorType::Hidden) {
                    SetCursor(cursors_[static_cast<int>(current_cursor_)]);
                } else {
                    SetCursor(nullptr);
                }
                return TRUE;
            }
            break;
        }

        case WM_DROPFILES: {
            HDROP drop = reinterpret_cast<HDROP>(wparam);
            UINT count = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);

            Vector<String> paths;
            paths.reserve(count);

            for (UINT i = 0; i < count; ++i) {
                UINT len = DragQueryFileW(drop, i, nullptr, 0) + 1;
                std::wstring wpath(len, L'\0');
                DragQueryFileW(drop, i, wpath.data(), len);

                // Convert to UTF-8
                int utf8_len = WideCharToMultiByte(CP_UTF8, 0, wpath.c_str(), -1, nullptr, 0, nullptr, nullptr);
                String path(static_cast<size_t>(utf8_len - 1), '\0');
                WideCharToMultiByte(CP_UTF8, 0, wpath.c_str(), -1, path.data(), utf8_len, nullptr, nullptr);
                paths.push_back(std::move(path));
            }

            DragFinish(drop);
            on_drop.emit(std::move(paths));
            return 0;
        }

        default:
            break;
    }

    return DefWindowProcW(hwnd_, msg, wparam, lparam);
}

void Win32Window::poll_events() {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            should_close_ = true;
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void Win32Window::wait_events() {
    WaitMessage();
    poll_events();
}

void Win32Window::wait_events_timeout(f64 timeout_seconds) {
    MsgWaitForMultipleObjects(0, nullptr, FALSE,
        static_cast<DWORD>(timeout_seconds * 1000.0), QS_ALLINPUT);
    poll_events();
}

void Win32Window::set_title(StringView title) {
    int len = MultiByteToWideChar(CP_UTF8, 0, title.data(),
                                  static_cast<int>(title.size()), nullptr, 0);
    std::wstring wide(static_cast<size_t>(len), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, title.data(),
                        static_cast<int>(title.size()), wide.data(), len);
    SetWindowTextW(hwnd_, wide.c_str());
}

void Win32Window::set_size(i32 width, i32 height) {
    RECT rect = {0, 0, width, height};
    DWORD style = get_window_style();
    DWORD ex_style = get_window_ex_style();
    AdjustWindowRectEx(&rect, style, FALSE, ex_style);
    SetWindowPos(hwnd_, nullptr, 0, 0,
                 rect.right - rect.left, rect.bottom - rect.top,
                 SWP_NOMOVE | SWP_NOZORDER);
}

void Win32Window::set_position(i32 x, i32 y) {
    SetWindowPos(hwnd_, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void Win32Window::set_min_size(i32 width, i32 height) {
    min_width_ = width;
    min_height_ = height;
}

void Win32Window::set_max_size(i32 width, i32 height) {
    max_width_ = width;
    max_height_ = height;
}

void Win32Window::set_visible(bool visible) {
    ShowWindow(hwnd_, visible ? SW_SHOW : SW_HIDE);
    state_ = visible ? WindowState::Normal : WindowState::Hidden;
}

void Win32Window::set_resizable(bool resizable) {
    resizable_ = resizable;
    update_window_style();
}

void Win32Window::set_decorated(bool decorated) {
    decorated_ = decorated;
    update_window_style();
}

void Win32Window::minimize() {
    ShowWindow(hwnd_, SW_MINIMIZE);
}

void Win32Window::maximize() {
    ShowWindow(hwnd_, SW_MAXIMIZE);
}

void Win32Window::restore() {
    ShowWindow(hwnd_, SW_RESTORE);
}

void Win32Window::focus() {
    BringWindowToTop(hwnd_);
    SetForegroundWindow(hwnd_);
    SetFocus(hwnd_);
}

void Win32Window::request_close() {
    should_close_ = true;
}

void Win32Window::set_cursor(CursorType cursor) {
    current_cursor_ = cursor;
    if (cursor_visible_ && cursor != CursorType::Hidden) {
        SetCursor(cursors_[static_cast<int>(cursor)]);
    } else {
        SetCursor(nullptr);
    }
}

void Win32Window::set_cursor_visible(bool visible) {
    cursor_visible_ = visible;
    if (visible && current_cursor_ != CursorType::Hidden) {
        SetCursor(cursors_[static_cast<int>(current_cursor_)]);
    } else {
        SetCursor(nullptr);
    }
}

Point2D Win32Window::cursor_position() const {
    return input_state_.mouse_position();
}

// ─────────────────────────────────────────────────────────────────────────────
// Platform Factory Function
// ─────────────────────────────────────────────────────────────────────────────

Result<Unique<PlatformWindow>> PlatformWindow::create(const WindowConfig& config) {
    auto window = make_unique<Win32Window>(config);
    if (!window->native_handle().win32.hwnd) {
        return Error{"Failed to create Win32 window"};
    }
    Unique<PlatformWindow> base_window = std::move(window);
    return base_window;
}

// ─────────────────────────────────────────────────────────────────────────────
// Platform Utilities
// ─────────────────────────────────────────────────────────────────────────────

namespace platform {

Result<void> initialize() {
    // Enable per-monitor DPI awareness
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    return {};
}

void shutdown() {
    // Nothing to clean up
}

f32 get_primary_monitor_dpi_scale() {
    HDC hdc = GetDC(nullptr);
    f32 scale = static_cast<f32>(GetDeviceCaps(hdc, LOGPIXELSX)) / 96.0f;
    ReleaseDC(nullptr, hdc);
    return scale;
}

f32 get_primary_monitor_refresh_rate() {
    DEVMODE dm{};
    dm.dmSize = sizeof(dm);
    if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm)) {
        return static_cast<f32>(dm.dmDisplayFrequency);
    }
    return 60.0f;
}

Vector<MonitorInfo> get_monitors() {
    Vector<MonitorInfo> monitors;

    EnumDisplayMonitors(nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM data) -> BOOL {
            auto& monitors = *reinterpret_cast<Vector<MonitorInfo>*>(data);

            MONITORINFOEXW mi{};
            mi.cbSize = sizeof(mi);
            GetMonitorInfoW(monitor, &mi);

            MonitorInfo info;

            // Convert name to UTF-8
            int len = WideCharToMultiByte(CP_UTF8, 0, mi.szDevice, -1, nullptr, 0, nullptr, nullptr);
            info.name.resize(static_cast<size_t>(len - 1));
            WideCharToMultiByte(CP_UTF8, 0, mi.szDevice, -1, info.name.data(), len, nullptr, nullptr);

            info.bounds = Rect{
                static_cast<f32>(mi.rcMonitor.left),
                static_cast<f32>(mi.rcMonitor.top),
                static_cast<f32>(mi.rcMonitor.right - mi.rcMonitor.left),
                static_cast<f32>(mi.rcMonitor.bottom - mi.rcMonitor.top)
            };

            info.work_area = Rect{
                static_cast<f32>(mi.rcWork.left),
                static_cast<f32>(mi.rcWork.top),
                static_cast<f32>(mi.rcWork.right - mi.rcWork.left),
                static_cast<f32>(mi.rcWork.bottom - mi.rcWork.top)
            };

            info.is_primary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;

            // Get DPI
            UINT dpi_x, dpi_y;
            if (SUCCEEDED(GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi_x, &dpi_y))) {
                info.dpi_scale = static_cast<f32>(dpi_x) / 96.0f;
            } else {
                info.dpi_scale = 1.0f;
            }

            // Get refresh rate
            DEVMODE dm{};
            dm.dmSize = sizeof(dm);
            if (EnumDisplaySettingsW(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm)) {
                info.refresh_rate = static_cast<f32>(dm.dmDisplayFrequency);
            } else {
                info.refresh_rate = 60.0f;
            }

            monitors.push_back(std::move(info));
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&monitors));

    return monitors;
}

} // namespace platform

} // namespace frost

#endif // _WIN32
