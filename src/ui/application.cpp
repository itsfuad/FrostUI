#include "frost/ui/application.hpp"
#include <chrono>

namespace frost {

Application::~Application() = default;

Result<Unique<Application>> Application::create(const ApplicationConfig& config) {
    auto app = Unique<Application>(new Application());

    // Create window
    auto window_result = PlatformWindow::create(config.window);
    if (!window_result) {
        return window_result.error();
    }
    app->window_ = std::move(window_result.value());
    app->target_fps_ = config.target_fps;

    // Connect window event handlers
    app->window_->on_mouse_move.connect([&app = *app](f64 x, f64 y) {
        app.handle_mouse_move(x, y);
    });

    app->window_->on_mouse_button.connect([&app = *app](MouseButton button, KeyAction action, ModifierFlags mods) {
        app.handle_mouse_button(button, action, mods);
    });

    app->window_->on_key.connect([&app = *app](KeyCode key, KeyAction action, ModifierFlags mods) {
        app.handle_key(key, action, mods);
    });

    app->window_->on_char.connect([&app = *app](u32 codepoint) {
        app.handle_char(codepoint);
    });

    app->window_->on_resize.connect([&app = *app](i32 width, i32 height) {
        app.handle_resize(width, height);
    });

    return app;
}

void Application::run() {
    running_ = true;

    auto get_time = []() {
        return std::chrono::duration<f64>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
        ).count();
    };

    last_frame_time_ = get_time();

    while (running_ && !window_->should_close()) {
        f64 current_time = get_time();
        delta_time_ = current_time - last_frame_time_;
        last_frame_time_ = current_time;

        process_events();
        update();
        render();

        // Simple frame rate limiting
        f64 frame_time = 1.0 / target_fps_;
        f64 elapsed = get_time() - current_time;
        if (elapsed < frame_time) {
            // Busy wait for remaining time (could use sleep for longer waits)
            while (get_time() - current_time < frame_time) {
                // Yield
            }
        }
    }

    running_ = false;
}

void Application::quit() {
    running_ = false;
}

void Application::set_root(Unique<Widget> root) {
    root_ = std::move(root);
    if (root_) {
        auto size = window_->size();
        root_->layout(Rect{0, 0, size.width, size.height});
    }
}

void Application::set_focused_widget(Widget* widget) {
    if (focused_widget_ != widget) {
        if (focused_widget_) {
            focused_widget_->on_focus_lost();
        }
        focused_widget_ = widget;
        if (focused_widget_) {
            focused_widget_->on_focus_gained();
        }
    }
}

void Application::process_events() {
    window_->poll_events();
}

void Application::update() {
    if (root_) {
        // Re-layout if dirty
        if (root_->is_dirty()) {
            auto size = window_->size();
            root_->layout(Rect{0, 0, size.width, size.height});
        }

        root_->update(delta_time_);
    }
}

void Application::render() {
    draw_list_.clear();

    if (root_) {
        root_->render(draw_list_);
    }

    // Render draw list to pixels and present to window
    auto size = window_->size();
    i32 width = static_cast<i32>(size.width);
    i32 height = static_cast<i32>(size.height);

    if (width > 0 && height > 0) {
        const u8* pixels = renderer_.render(draw_list_, width, height);
        window_->present_pixels(pixels, width, height);
    }
}

void Application::handle_mouse_move(f64 x, f64 y) {
    Point2D pos{static_cast<f32>(x), static_cast<f32>(y)};
    last_mouse_pos_ = pos;

    // Find widget under cursor
    Widget* new_hovered = root_ ? root_->find_widget_at(pos) : nullptr;

    if (new_hovered != hovered_widget_) {
        if (hovered_widget_) {
            hovered_widget_->on_mouse_leave();
        }
        hovered_widget_ = new_hovered;
        if (hovered_widget_) {
            hovered_widget_->on_mouse_enter();
        }
    }

    if (hovered_widget_) {
        hovered_widget_->on_mouse_move(x, y);
    }
}

void Application::handle_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods) {
    if (hovered_widget_) {
        hovered_widget_->on_mouse_button(button, action, mods);

        // Focus on click
        if (button == MouseButton::Left && action == KeyAction::Press) {
            set_focused_widget(hovered_widget_);
        }
    }
}

void Application::handle_key(KeyCode key, KeyAction action, ModifierFlags mods) {
    if (focused_widget_) {
        focused_widget_->on_key(key, action, mods);
    }
}

void Application::handle_char(u32 codepoint) {
    if (focused_widget_) {
        focused_widget_->on_char(codepoint);
    }
}

void Application::handle_resize(i32 width, i32 height) {
    if (root_) {
        root_->layout(Rect{0, 0, static_cast<f32>(width), static_cast<f32>(height)});
    }
}

} // namespace frost
