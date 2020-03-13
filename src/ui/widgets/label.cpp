#include "frost/ui/widgets/label.hpp"
#include "frost/graphics/draw_list.hpp"

namespace frost {

Unique<Label> Label::create(StringView text) {
    return Unique<Label>(new Label(text, LabelStyle{}));
}

Unique<Label> Label::create(StringView text, const LabelStyle& style) {
    return Unique<Label>(new Label(text, style));
}

Label::Label(StringView text, const LabelStyle& style)
    : text_(text)
    , style_(style) {}

void Label::set_text(StringView text) {
    if (text_ != text) {
        text_ = String(text);
        mark_dirty();
    }
}

void Label::set_style(const LabelStyle& style) {
    style_ = style;
    mark_dirty();
}

void Label::set_text_color(Color color) {
    if (style_.text_color != color) {
        style_.text_color = color;
        mark_dirty();
    }
}

void Label::set_font_size(f32 size) {
    if (style_.font_size != size) {
        style_.font_size = size;
        mark_dirty();
    }
}

void Label::set_text_align(TextAlign align) {
    if (style_.text_align != align) {
        style_.text_align = align;
        mark_dirty();
    }
}

void Label::set_vertical_align(VerticalAlign align) {
    if (style_.vertical_align != align) {
        style_.vertical_align = align;
        mark_dirty();
    }
}

f32 Label::calculate_text_width() const {
    // Placeholder: approximate width based on character count
    // Real implementation would use font metrics
    return static_cast<f32>(text_.size()) * style_.font_size * 0.6f;
}

f32 Label::calculate_text_height() const {
    return style_.font_size;
}

Size2D Label::measure(const LayoutConstraints& constraints) {
    f32 text_width = calculate_text_width();
    f32 text_height = calculate_text_height();

    f32 width = text_width + padding_.left + padding_.right;
    f32 height = text_height + padding_.top + padding_.bottom;

    // Clamp to constraints
    width = std::clamp(width, constraints.min_size.width, constraints.max_size.width);
    height = std::clamp(height, constraints.min_size.height, constraints.max_size.height);

    return Size2D{width, height};
}

void Label::render(DrawList& draw_list) {
    if (!is_visible() || text_.empty()) {
        return;
    }

    // Calculate text position
    f32 text_width = calculate_text_width();
    f32 text_height = calculate_text_height();

    // Content area
    f32 content_x = bounds_.x + padding_.left;
    f32 content_y = bounds_.y + padding_.top;
    f32 content_width = bounds_.width - padding_.left - padding_.right;
    f32 content_height = bounds_.height - padding_.top - padding_.bottom;

    // Horizontal alignment
    f32 x = content_x;
    switch (style_.text_align) {
        case TextAlign::Left:
            x = content_x;
            break;
        case TextAlign::Center:
            x = content_x + (content_width - text_width) / 2.0f;
            break;
        case TextAlign::Right:
            x = content_x + content_width - text_width;
            break;
    }

    // Vertical alignment
    f32 y = content_y;
    switch (style_.vertical_align) {
        case VerticalAlign::Top:
            y = content_y;
            break;
        case VerticalAlign::Middle:
            y = content_y + (content_height - text_height) / 2.0f;
            break;
        case VerticalAlign::Bottom:
            y = content_y + content_height - text_height;
            break;
    }

    // Draw text (placeholder - draws a rectangle representing text)
    draw_list.add_text(Point2D{x, y}, text_, style_.text_color, style_.font_size);
}

} // namespace frost
