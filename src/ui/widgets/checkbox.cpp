#include "frost/ui/widgets/checkbox.hpp"
#include "frost/graphics/draw_list.hpp"
#include <algorithm>

namespace frost {

Unique<Checkbox> Checkbox::create(StringView text) {
    return Unique<Checkbox>(new Checkbox(text, CheckboxStyle{}));
}

Unique<Checkbox> Checkbox::create(StringView text, const CheckboxStyle& style) {
    return Unique<Checkbox>(new Checkbox(text, style));
}

Checkbox::Checkbox(StringView text, const CheckboxStyle& style)
    : text_(text)
    , style_(style) {
    padding_ = style_.padding;
}

void Checkbox::set_checked(bool checked) {
    if (checked_ != checked) {
        checked_ = checked;
        on_toggled.emit(checked_);
        mark_dirty();
    }
}

void Checkbox::toggle() {
    set_checked(!checked_);
}

void Checkbox::set_text(StringView text) {
    if (text_ != text) {
        text_ = String(text);
        mark_dirty();
    }
}

void Checkbox::set_style(const CheckboxStyle& style) {
    style_ = style;
    padding_ = style_.padding;
    mark_dirty();
}

f32 Checkbox::calculate_text_width() const {
    return static_cast<f32>(text_.size()) * style_.font_size * 0.6f;
}

f32 Checkbox::calculate_text_height() const {
    return style_.font_size;
}

Size2D Checkbox::measure(const LayoutConstraints& constraints) {
    const f32 text_width = calculate_text_width();
    const f32 text_height = calculate_text_height();

    f32 width = padding_.left + style_.box_size + padding_.right;
    if (!text_.empty()) {
        width += style_.spacing + text_width;
    }

    f32 height = std::max(style_.box_size, text_height) + padding_.top + padding_.bottom;

    width = std::clamp(width, constraints.min_size.width, constraints.max_size.width);
    height = std::clamp(height, constraints.min_size.height, constraints.max_size.height);
    return Size2D{width, height};
}

void Checkbox::render(DrawList& draw_list) {
    if (!is_visible()) {
        return;
    }

    const f32 content_x = bounds_.x + padding_.left;
    const f32 content_y = bounds_.y + padding_.top;
    const f32 content_h = bounds_.height - padding_.top - padding_.bottom;
    const f32 box_y = content_y + (content_h - style_.box_size) * 0.5f;

    Color border = is_enabled() ? style_.box_border : style_.disabled_color;
    Color box_bg = is_enabled() ? style_.box_background : Color{0.1f, 0.1f, 0.1f, 1.0f};

    draw_list.add_rect_filled(Rect{content_x, box_y, style_.box_size, style_.box_size}, box_bg, style_.corner_radius);
    draw_list.add_rect_outline(Rect{content_x, box_y, style_.box_size, style_.box_size}, border, style_.border_width, style_.corner_radius);

    if (checked_) {
        Color check_color = is_enabled() ? style_.check_color : style_.disabled_color;
        const f32 x0 = content_x + style_.box_size * 0.22f;
        const f32 y0 = box_y + style_.box_size * 0.52f;
        const f32 x1 = content_x + style_.box_size * 0.43f;
        const f32 y1 = box_y + style_.box_size * 0.75f;
        const f32 x2 = content_x + style_.box_size * 0.8f;
        const f32 y2 = box_y + style_.box_size * 0.28f;

        draw_list.add_line(Point2D{x0, y0}, Point2D{x1, y1}, check_color, 2.0f);
        draw_list.add_line(Point2D{x1, y1}, Point2D{x2, y2}, check_color, 2.0f);
    }

    if (!text_.empty()) {
        const f32 text_h = calculate_text_height();
        const f32 text_x = content_x + style_.box_size + style_.spacing;
        const f32 text_y = content_y + (content_h - text_h) * 0.5f;
        draw_list.add_text(
            Point2D{text_x, text_y},
            text_,
            is_enabled() ? style_.text_color : style_.disabled_color,
            style_.font_size
        );
    }
}

bool Checkbox::on_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods) {
    Widget::on_mouse_button(button, action, mods);

    if (!is_enabled()) {
        return false;
    }

    if (button == MouseButton::Left && action == KeyAction::Release && is_hovered()) {
        toggle();
        return true;
    }

    return false;
}

bool Checkbox::on_key(KeyCode key, KeyAction action, ModifierFlags mods) {
    if (!is_enabled() || action != KeyAction::Press) {
        return false;
    }

    if (key == KeyCode::Space || key == KeyCode::Return) {
        toggle();
        return true;
    }

    return false;
}

} // namespace frost
