#include "frost/ui/widgets/radio_button.hpp"
#include "frost/graphics/draw_list.hpp"
#include <algorithm>
#include <unordered_map>

namespace frost {

namespace {
using GroupMap = std::unordered_map<String, Vector<RadioButton*>>;

GroupMap& radio_groups() {
    static GroupMap groups;
    return groups;
}
} // namespace

Unique<RadioButton> RadioButton::create(StringView text, StringView group) {
    return Unique<RadioButton>(new RadioButton(text, group, RadioButtonStyle{}));
}

Unique<RadioButton> RadioButton::create(StringView text, StringView group, const RadioButtonStyle& style) {
    return Unique<RadioButton>(new RadioButton(text, group, style));
}

RadioButton::RadioButton(StringView text, StringView group, const RadioButtonStyle& style)
    : text_(text)
    , group_(group.empty() ? "default" : String(group))
    , style_(style) {
    padding_ = style_.padding;
    register_in_group();
}

RadioButton::~RadioButton() {
    unregister_from_group();
}

void RadioButton::register_in_group() {
    radio_groups()[group_].push_back(this);
}

void RadioButton::unregister_from_group() {
    auto it = radio_groups().find(group_);
    if (it == radio_groups().end()) {
        return;
    }

    auto& group_vec = it->second;
    group_vec.erase(std::remove(group_vec.begin(), group_vec.end(), this), group_vec.end());
    if (group_vec.empty()) {
        radio_groups().erase(it);
    }
}

void RadioButton::set_selected(bool selected) {
    if (selected) {
        select_exclusive();
        return;
    }

    if (selected_) {
        selected_ = false;
        on_selected.emit(false);
        mark_dirty();
    }
}

void RadioButton::select_exclusive() {
    auto it = radio_groups().find(group_);
    if (it != radio_groups().end()) {
        for (RadioButton* button : it->second) {
            if (!button) {
                continue;
            }
            if (button == this) {
                continue;
            }
            if (button->selected_) {
                button->selected_ = false;
                button->on_selected.emit(false);
                button->mark_dirty();
            }
        }
    }

    if (!selected_) {
        selected_ = true;
        on_selected.emit(true);
        mark_dirty();
    }
}

void RadioButton::set_text(StringView text) {
    if (text_ != text) {
        text_ = String(text);
        mark_dirty();
    }
}

void RadioButton::set_group(StringView group) {
    String next_group = group.empty() ? "default" : String(group);
    if (group_ == next_group) {
        return;
    }

    unregister_from_group();
    group_ = std::move(next_group);
    register_in_group();

    if (selected_) {
        select_exclusive();
    }

    mark_dirty();
}

void RadioButton::set_style(const RadioButtonStyle& style) {
    style_ = style;
    padding_ = style_.padding;
    mark_dirty();
}

f32 RadioButton::calculate_text_width() const {
    return static_cast<f32>(text_.size()) * style_.font_size * 0.6f;
}

f32 RadioButton::calculate_text_height() const {
    return style_.font_size;
}

Size2D RadioButton::measure(const LayoutConstraints& constraints) {
    const f32 diameter = style_.radius * 2.0f;
    const f32 text_width = calculate_text_width();
    const f32 text_height = calculate_text_height();

    f32 width = padding_.left + diameter + padding_.right;
    if (!text_.empty()) {
        width += style_.spacing + text_width;
    }
    f32 height = std::max(diameter, text_height) + padding_.top + padding_.bottom;

    width = std::clamp(width, constraints.min_size.width, constraints.max_size.width);
    height = std::clamp(height, constraints.min_size.height, constraints.max_size.height);
    return Size2D{width, height};
}

void RadioButton::render(DrawList& draw_list) {
    if (!is_visible()) {
        return;
    }

    const f32 content_x = bounds_.x + padding_.left;
    const f32 content_y = bounds_.y + padding_.top;
    const f32 content_h = bounds_.height - padding_.top - padding_.bottom;
    const f32 diameter = style_.radius * 2.0f;
    const Point2D center{content_x + style_.radius, content_y + content_h * 0.5f};

    Color outer = is_enabled() ? style_.outer_color : style_.disabled_color;
    draw_list.add_circle_outline(center, style_.radius, outer, style_.border_width);

    if (selected_) {
        const f32 inner_radius = style_.radius * 0.55f;
        draw_list.add_circle_filled(center, inner_radius, is_enabled() ? style_.inner_color : style_.disabled_color);
    }

    if (!text_.empty()) {
        const f32 text_h = calculate_text_height();
        const f32 text_x = content_x + diameter + style_.spacing;
        const f32 text_y = content_y + (content_h - text_h) * 0.5f;
        draw_list.add_text(
            Point2D{text_x, text_y},
            text_,
            is_enabled() ? style_.text_color : style_.disabled_color,
            style_.font_size
        );
    }
}

bool RadioButton::on_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods) {
    Widget::on_mouse_button(button, action, mods);

    if (!is_enabled()) {
        return false;
    }

    if (button == MouseButton::Left && action == KeyAction::Release && is_hovered()) {
        select_exclusive();
        return true;
    }

    return false;
}

bool RadioButton::on_key(KeyCode key, KeyAction action, ModifierFlags mods) {
    if (!is_enabled() || action != KeyAction::Press) {
        return false;
    }

    if (key == KeyCode::Space || key == KeyCode::Return) {
        select_exclusive();
        return true;
    }

    return false;
}

} // namespace frost
