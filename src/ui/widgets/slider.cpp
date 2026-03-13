#include "frost/ui/widgets/slider.hpp"
#include "frost/graphics/draw_list.hpp"
#include <algorithm>
#include <cmath>

namespace frost {

Unique<Slider> Slider::create() {
    return Unique<Slider>(new Slider(0.0f, 1.0f, 0.0f, SliderStyle{}));
}

Unique<Slider> Slider::create(f32 min_value, f32 max_value, f32 initial_value) {
    return Unique<Slider>(new Slider(min_value, max_value, initial_value, SliderStyle{}));
}

Unique<Slider> Slider::create(f32 min_value, f32 max_value, f32 initial_value, const SliderStyle& style) {
    return Unique<Slider>(new Slider(min_value, max_value, initial_value, style));
}

Slider::Slider(f32 min_value, f32 max_value, f32 initial_value, const SliderStyle& style)
    : style_(style) {
    padding_ = style_.padding;
    set_range(min_value, max_value);
    set_value_internal(initial_value, false);
}

void Slider::set_value(f32 value) {
    set_value_internal(value, true);
}

void Slider::set_value_internal(f32 value, bool emit_signal) {
    const f32 clamped = quantize_value(value);
    if (value_ != clamped) {
        value_ = clamped;
        if (emit_signal) {
            on_value_changed.emit(value_);
        }
        mark_dirty();
    }
}

void Slider::set_range(f32 min_value, f32 max_value) {
    if (max_value < min_value) {
        std::swap(min_value, max_value);
    }

    min_value_ = min_value;
    max_value_ = max_value;
    set_value_internal(value_, false);
    mark_dirty();
}

void Slider::set_step(f32 step) {
    step_ = std::max(0.0f, step);
    set_value_internal(value_, false);
}

void Slider::set_style(const SliderStyle& style) {
    style_ = style;
    padding_ = style_.padding;
    mark_dirty();
}

f32 Slider::normalized_value() const {
    const f32 range = max_value_ - min_value_;
    if (range <= 0.0f) {
        return 0.0f;
    }
    return (value_ - min_value_) / range;
}

f32 Slider::quantize_value(f32 value) const {
    const f32 clamped = std::clamp(value, min_value_, max_value_);
    if (step_ <= 0.0f) {
        return clamped;
    }

    const f32 steps = std::round((clamped - min_value_) / step_);
    return std::clamp(min_value_ + steps * step_, min_value_, max_value_);
}

f32 Slider::value_from_x(f32 x) const {
    const f32 left = bounds_.x + padding_.left + style_.thumb_radius;
    const f32 right = bounds_.x + bounds_.width - padding_.right - style_.thumb_radius;
    if (right <= left) {
        return min_value_;
    }

    const f32 t = std::clamp((x - left) / (right - left), 0.0f, 1.0f);
    return min_value_ + t * (max_value_ - min_value_);
}

Size2D Slider::measure(const LayoutConstraints& constraints) {
    f32 width = constraints.preferred_width.value_or(180.0f);
    f32 height = std::max(style_.thumb_radius * 2.0f, style_.track_height)
        + padding_.top + padding_.bottom;

    width = std::clamp(width, constraints.min_size.width, constraints.max_size.width);
    height = std::clamp(height, constraints.min_size.height, constraints.max_size.height);
    return Size2D{width, height};
}

void Slider::render(DrawList& draw_list) {
    if (!is_visible()) {
        return;
    }

    const f32 center_y = bounds_.y + bounds_.height * 0.5f;
    const f32 left = bounds_.x + padding_.left + style_.thumb_radius;
    const f32 right = bounds_.x + bounds_.width - padding_.right - style_.thumb_radius;
    const f32 track_width = std::max(0.0f, right - left);

    const Color track_color = is_enabled() ? style_.track_color : style_.disabled_color;
    const Color fill_color = is_enabled() ? style_.fill_color : style_.disabled_color;
    const Color thumb_color = is_enabled() ? style_.thumb_color : style_.disabled_color;

    const Rect track_rect{left, center_y - style_.track_height * 0.5f, track_width, style_.track_height};
    draw_list.add_rect_filled(track_rect, track_color, style_.track_height * 0.5f);

    const f32 fill_w = track_width * normalized_value();
    if (fill_w > 0.0f) {
        draw_list.add_rect_filled(Rect{left, track_rect.y, fill_w, track_rect.height}, fill_color, style_.track_height * 0.5f);
    }

    const f32 thumb_x = left + fill_w;
    draw_list.add_circle_filled(Point2D{thumb_x, center_y}, style_.thumb_radius, thumb_color);
}

bool Slider::on_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods) {
    Widget::on_mouse_button(button, action, mods);

    if (!is_enabled() || button != MouseButton::Left) {
        return false;
    }

    if (action == KeyAction::Press) {
        set_value(value_from_x(last_mouse_x_));
        dragging_ = true;
        return true;
    }

    if (action == KeyAction::Release) {
        dragging_ = false;
        return true;
    }

    return false;
}

bool Slider::on_mouse_move(f64 x, f64 y) {
    if (!is_enabled()) {
        return false;
    }

    last_mouse_x_ = static_cast<f32>(x);

    if (!dragging_) {
        return false;
    }

    set_value(value_from_x(static_cast<f32>(x)));
    return true;
}

bool Slider::on_key(KeyCode key, KeyAction action, ModifierFlags mods) {
    if (!is_enabled() || action != KeyAction::Press) {
        return false;
    }

    const f32 delta = step_ > 0.0f ? step_ : (max_value_ - min_value_) / 100.0f;
    if (delta <= 0.0f) {
        return false;
    }

    if (key == KeyCode::Left || key == KeyCode::Down) {
        set_value(value_ - delta);
        return true;
    }

    if (key == KeyCode::Right || key == KeyCode::Up) {
        set_value(value_ + delta);
        return true;
    }

    return false;
}

} // namespace frost
