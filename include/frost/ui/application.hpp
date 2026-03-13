#pragma once

#include "frost/core/types.hpp"
#include "frost/core/result.hpp"
#include "frost/platform/window.hpp"
#include "frost/graphics/draw_list.hpp"
#include "frost/graphics/software_renderer.hpp"
#include "frost/ui/widget.hpp"

namespace frost {

/// Application configuration
struct ApplicationConfig {
    WindowConfig window;
    bool vsync{true};
    f32 target_fps{60.0f};
};

/// Main application class that manages the window and UI
class Application : public NonCopyable {
public:
    /// Create a new application
    [[nodiscard]] static Result<Unique<Application>> create(const ApplicationConfig& config);
    ~Application();

    // ─────────────────────────────────────────────────────────────────────────
    // Main Loop
    // ─────────────────────────────────────────────────────────────────────────

    /// Run the main event loop
    void run();

    /// Quit the application
    void quit();

    /// Check if the application is running
    [[nodiscard]] bool is_running() const { return running_; }

    // ─────────────────────────────────────────────────────────────────────────
    // Root Widget
    // ─────────────────────────────────────────────────────────────────────────

    /// Set the root widget
    void set_root(Unique<Widget> root);

    /// Get the root widget
    [[nodiscard]] Widget* root() const { return root_.get(); }

    // ─────────────────────────────────────────────────────────────────────────
    // Focus Management
    // ─────────────────────────────────────────────────────────────────────────

    /// Set the focused widget
    void set_focused_widget(Widget* widget);

    /// Get the currently focused widget
    [[nodiscard]] Widget* focused_widget() const { return focused_widget_; }

    // ─────────────────────────────────────────────────────────────────────────
    // Accessors
    // ─────────────────────────────────────────────────────────────────────────

    /// Get the window
    [[nodiscard]] PlatformWindow& window() { return *window_; }

    /// Get draw list (for this frame)
    [[nodiscard]] DrawList& draw_list() { return draw_list_; }

    /// Get frame delta time
    [[nodiscard]] f64 delta_time() const { return delta_time_; }

    // ─────────────────────────────────────────────────────────────────────────
    // Fonts
    // ─────────────────────────────────────────────────────────────────────────

    /// Load a custom bitmap font (PSF1/PSF2) for software text rendering.
    [[nodiscard]] Result<void> load_font_from_file(StringView file_path);

    /// Revert software text rendering back to the built-in default font.
    void reset_font();

private:
    Application() = default;

    void process_events();
    void update();
    void render();

    void handle_mouse_move(f64 x, f64 y);
    void handle_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods);
    void handle_key(KeyCode key, KeyAction action, ModifierFlags mods);
    void handle_char(u32 codepoint);
    void handle_resize(i32 width, i32 height);

    Unique<PlatformWindow> window_;
    Unique<Widget> root_;
    DrawList draw_list_;
    SoftwareRenderer renderer_;

    Widget* focused_widget_{nullptr};
    Widget* hovered_widget_{nullptr};
    Point2D last_mouse_pos_;

    bool running_{false};
    f64 last_frame_time_{0.0};
    f64 delta_time_{0.0};
    f32 target_fps_{60.0f};
};

} // namespace frost
