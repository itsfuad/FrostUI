#include "frost/ui/widgets/button.hpp"
#include "frost/graphics/draw_list.hpp"

namespace frost {

Unique<Button> Button::create(StringView text) {
    return Unique<Button>(new Button(text, ButtonStyle{}));
}

Unique<Button> Button::create(StringView text, const ButtonStyle& style) {
    return Unique<Button>(new Button(text, style));
}

Button::Button(StringView text, const ButtonStyle& style)
    : text_(text)
    , style_(style) {
    // Apply style padding to widget
    padding_ = style_.padding;
}

void Button::set_text(StringView text) {
    if (text_ != text) {
        text_ = String(text);
        mark_dirty();
    }
}

void Button::set_style(const ButtonStyle& style) {
    style_ = style;
    padding_ = style_.padding;
    mark_dirty();
}

Color Button::current_background_color() const {
    if (!is_enabled()) {
        return style_.disabled_color;
    }
    if (is_pressed()) {
        return style_.pressed_color;
    }
    if (is_hovered()) {
        return style_.hover_color;
    }
    return style_.background_color;
}

Color Button::current_text_color() const {
    if (!is_enabled()) {
        return style_.disabled_text_color;
    }
    return style_.text_color;
}

f32 Button::calculate_text_width() const {
    // Placeholder: approximate width based on character count
    return static_cast<f32>(text_.size()) * style_.font_size * 0.6f;
}

f32 Button::calculate_text_height() const {
    return style_.font_size;
}

Size2D Button::measure(const LayoutConstraints& constraints) {
    f32 text_width = calculate_text_width();
    f32 text_height = calculate_text_height();

    f32 width = text_width + padding_.left + padding_.right;
    f32 height = text_height + padding_.top + padding_.bottom;

    // Ensure minimum size for usability
    width = std::max(width, 60.0f);
    height = std::max(height, 32.0f);

    // Clamp to constraints
    width = std::clamp(width, constraints.min_size.width, constraints.max_size.width);
    height = std::clamp(height, constraints.min_size.height, constraints.max_size.height);

    return Size2D{width, height};
}

void Button::render(DrawList& draw_list) {
    if (!is_visible()) {
        return;
    }

    // Draw background
    draw_list.add_rect_filled(bounds_, current_background_color(), style_.corner_radius);

    // Draw text centered
    if (!text_.empty()) {
        f32 text_width = calculate_text_width();
        f32 text_height = calculate_text_height();

        f32 x = bounds_.x + (bounds_.width - text_width) / 2.0f;
        f32 y = bounds_.y + (bounds_.height - text_height) / 2.0f;

        draw_list.add_text(Point2D{x, y}, text_, current_text_color(), style_.font_size);
    }
}

bool Button::on_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods) {
    // Call base to update pressed state
    Widget::on_mouse_button(button, action, mods);

    if (!is_enabled()) {
        return false;
    }

    // Emit click on release if still hovered
    if (button == MouseButton::Left && action == KeyAction::Release && is_hovered()) {
        on_click.emit();
        return true;
    }

    return false;
}

} // namespace frost
