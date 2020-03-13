#include "frost/ui/widgets/text_input.hpp"
#include "frost/graphics/draw_list.hpp"
#include <algorithm>

namespace frost {

Unique<TextInput> TextInput::create() {
    return Unique<TextInput>(new TextInput("", TextInputStyle{}));
}

Unique<TextInput> TextInput::create(StringView placeholder) {
    return Unique<TextInput>(new TextInput(placeholder, TextInputStyle{}));
}

Unique<TextInput> TextInput::create(StringView placeholder, const TextInputStyle& style) {
    return Unique<TextInput>(new TextInput(placeholder, style));
}

TextInput::TextInput(StringView placeholder, const TextInputStyle& style)
    : placeholder_(placeholder)
    , style_(style) {
    padding_ = style_.padding;
}

void TextInput::set_text(StringView text) {
    if (text_ != text) {
        text_ = String(text);
        cursor_pos_ = text_.size();
        selection_start_ = cursor_pos_;
        selection_end_ = cursor_pos_;
        on_text_changed.emit(text_);
        mark_dirty();
    }
}

void TextInput::set_placeholder(StringView placeholder) {
    if (placeholder_ != placeholder) {
        placeholder_ = String(placeholder);
        mark_dirty();
    }
}

void TextInput::set_cursor_position(usize pos) {
    cursor_pos_ = std::min(pos, text_.size());
    selection_start_ = cursor_pos_;
    selection_end_ = cursor_pos_;
    mark_dirty();
}

void TextInput::select_all() {
    selection_start_ = 0;
    selection_end_ = text_.size();
    cursor_pos_ = text_.size();
    mark_dirty();
}

void TextInput::clear_selection() {
    selection_start_ = cursor_pos_;
    selection_end_ = cursor_pos_;
}

void TextInput::set_style(const TextInputStyle& style) {
    style_ = style;
    padding_ = style_.padding;
    mark_dirty();
}

void TextInput::insert_text(StringView text) {
    // Delete selection first
    if (has_selection()) {
        delete_selection();
    }

    // Insert text at cursor
    text_.insert(cursor_pos_, text.data(), text.size());
    cursor_pos_ += text.size();
    selection_start_ = cursor_pos_;
    selection_end_ = cursor_pos_;

    on_text_changed.emit(text_);
    mark_dirty();
}

void TextInput::delete_selection() {
    if (!has_selection()) return;

    usize start = std::min(selection_start_, selection_end_);
    usize end = std::max(selection_start_, selection_end_);

    text_.erase(start, end - start);
    cursor_pos_ = start;
    selection_start_ = cursor_pos_;
    selection_end_ = cursor_pos_;

    on_text_changed.emit(text_);
    mark_dirty();
}

void TextInput::delete_backward() {
    if (has_selection()) {
        delete_selection();
    } else if (cursor_pos_ > 0) {
        text_.erase(cursor_pos_ - 1, 1);
        cursor_pos_--;
        selection_start_ = cursor_pos_;
        selection_end_ = cursor_pos_;
        on_text_changed.emit(text_);
        mark_dirty();
    }
}

void TextInput::delete_forward() {
    if (has_selection()) {
        delete_selection();
    } else if (cursor_pos_ < text_.size()) {
        text_.erase(cursor_pos_, 1);
        on_text_changed.emit(text_);
        mark_dirty();
    }
}

void TextInput::move_cursor(i32 delta, bool extend_selection) {
    i32 new_pos = static_cast<i32>(cursor_pos_) + delta;
    new_pos = std::clamp(new_pos, 0, static_cast<i32>(text_.size()));
    move_cursor_to(static_cast<usize>(new_pos), extend_selection);
}

void TextInput::move_cursor_to(usize pos, bool extend_selection) {
    pos = std::min(pos, text_.size());

    if (extend_selection) {
        // Extend selection from anchor
        if (!has_selection()) {
            selection_start_ = cursor_pos_;
        }
        cursor_pos_ = pos;
        selection_end_ = pos;
    } else {
        cursor_pos_ = pos;
        selection_start_ = pos;
        selection_end_ = pos;
    }

    mark_dirty();
}

Color TextInput::current_background_color() const {
    if (is_focused()) {
        return style_.focus_color;
    }
    return style_.background_color;
}

Color TextInput::current_border_color() const {
    if (is_focused()) {
        return style_.focus_border_color;
    }
    return style_.border_color;
}

f32 TextInput::calculate_text_width(StringView text) const {
    // Placeholder: approximate width based on character count
    return static_cast<f32>(text.size()) * style_.font_size * 0.6f;
}

f32 TextInput::calculate_text_height() const {
    return style_.font_size;
}

usize TextInput::get_char_at_x(f32 x) const {
    f32 content_x = bounds_.x + padding_.left - scroll_offset_;
    f32 char_width = style_.font_size * 0.6f;

    if (x <= content_x) return 0;

    usize pos = static_cast<usize>((x - content_x) / char_width);
    return std::min(pos, text_.size());
}

Size2D TextInput::measure(const LayoutConstraints& constraints) {
    f32 text_height = calculate_text_height();

    // Default width for text input
    f32 width = 200.0f;
    f32 height = text_height + padding_.top + padding_.bottom;

    // Use preferred width if specified
    if (constraints.preferred_width.has_value()) {
        width = constraints.preferred_width.value();
    }

    // Clamp to constraints
    width = std::clamp(width, constraints.min_size.width, constraints.max_size.width);
    height = std::clamp(height, constraints.min_size.height, constraints.max_size.height);

    return Size2D{width, height};
}

void TextInput::update(f64 delta_time) {
    Widget::update(delta_time);

    // Update cursor blink
    if (is_focused()) {
        cursor_blink_time_ += static_cast<f32>(delta_time);
        if (cursor_blink_time_ >= kCursorBlinkRate * 2.0f) {
            cursor_blink_time_ = 0.0f;
        }
    }
}

void TextInput::render(DrawList& draw_list) {
    if (!is_visible()) {
        return;
    }

    // Draw background
    draw_list.add_rect_filled(bounds_, current_background_color(), style_.corner_radius);

    // Draw border
    draw_list.add_rect_outline(bounds_, current_border_color(), style_.border_width, style_.corner_radius);

    // Content area
    f32 content_x = bounds_.x + padding_.left;
    f32 content_y = bounds_.y + padding_.top;
    f32 content_width = bounds_.width - padding_.left - padding_.right;
    f32 content_height = bounds_.height - padding_.top - padding_.bottom;
    f32 text_height = calculate_text_height();
    f32 text_y = content_y + (content_height - text_height) / 2.0f;

    // Clip to content area
    draw_list.push_clip_rect(Rect{content_x, content_y, content_width, content_height});

    // Draw text or placeholder
    if (text_.empty() && !placeholder_.empty() && !is_focused()) {
        draw_list.add_text(Point2D{content_x, text_y}, placeholder_,
                          style_.placeholder_color, style_.font_size);
    } else if (!text_.empty()) {
        // Draw selection background
        if (has_selection() && is_focused()) {
            usize sel_start = std::min(selection_start_, selection_end_);
            usize sel_end = std::max(selection_start_, selection_end_);

            f32 char_width = style_.font_size * 0.6f;
            f32 sel_x = content_x + static_cast<f32>(sel_start) * char_width - scroll_offset_;
            f32 sel_width = static_cast<f32>(sel_end - sel_start) * char_width;

            draw_list.add_rect_filled(
                Rect{sel_x, text_y, sel_width, text_height},
                style_.selection_color
            );
        }

        // Draw text
        draw_list.add_text(Point2D{content_x - scroll_offset_, text_y}, text_,
                          style_.text_color, style_.font_size);
    }

    // Draw cursor
    if (is_focused() && cursor_blink_time_ < kCursorBlinkRate) {
        f32 char_width = style_.font_size * 0.6f;
        f32 cursor_x = content_x + static_cast<f32>(cursor_pos_) * char_width - scroll_offset_;

        draw_list.add_rect_filled(
            Rect{cursor_x, text_y, 2.0f, text_height},
            style_.cursor_color
        );
    }

    draw_list.pop_clip_rect();
}

bool TextInput::on_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods) {
    Widget::on_mouse_button(button, action, mods);

    if (!is_enabled()) {
        return false;
    }

    if (button == MouseButton::Left && action == KeyAction::Press) {
        // TODO: Get actual mouse position from event
        // For now, just focus the input
        cursor_blink_time_ = 0.0f;
        return true;
    }

    return false;
}

bool TextInput::on_key(KeyCode key, KeyAction action, ModifierFlags mods) {
    if (!is_enabled() || action == KeyAction::Release) {
        return false;
    }

    bool shift = has_modifier(mods, ModifierFlags::Shift);
    bool ctrl = has_modifier(mods, ModifierFlags::Control);

    switch (key) {
        case KeyCode::Left:
            if (ctrl) {
                // TODO: Move by word
                move_cursor(-1, shift);
            } else {
                move_cursor(-1, shift);
            }
            cursor_blink_time_ = 0.0f;
            return true;

        case KeyCode::Right:
            if (ctrl) {
                // TODO: Move by word
                move_cursor(1, shift);
            } else {
                move_cursor(1, shift);
            }
            cursor_blink_time_ = 0.0f;
            return true;

        case KeyCode::Home:
            move_cursor_to(0, shift);
            cursor_blink_time_ = 0.0f;
            return true;

        case KeyCode::End:
            move_cursor_to(text_.size(), shift);
            cursor_blink_time_ = 0.0f;
            return true;

        case KeyCode::Backspace:
            delete_backward();
            cursor_blink_time_ = 0.0f;
            return true;

        case KeyCode::Delete:
            delete_forward();
            cursor_blink_time_ = 0.0f;
            return true;

        case KeyCode::Return:
            on_submit.emit(text_);
            return true;

        case KeyCode::A:  // Ctrl+A - Select All
            if (ctrl) {
                select_all();
                return true;
            }
            break;

        case KeyCode::C:  // Ctrl+C - Copy (TODO: clipboard)
            if (ctrl && has_selection()) {
                // TODO: Copy to clipboard
                return true;
            }
            break;

        case KeyCode::V:  // Ctrl+V - Paste (TODO: clipboard)
            if (ctrl) {
                // TODO: Paste from clipboard
                return true;
            }
            break;

        case KeyCode::X:  // Ctrl+X - Cut (TODO: clipboard)
            if (ctrl && has_selection()) {
                // TODO: Cut to clipboard
                delete_selection();
                return true;
            }
            break;

        default:
            break;
    }

    return false;
}

bool TextInput::on_char(u32 codepoint) {
    if (!is_enabled()) {
        return false;
    }

    // Ignore control characters
    if (codepoint < 32) {
        return false;
    }

    // Convert codepoint to UTF-8 (simplified - only handles ASCII for now)
    if (codepoint < 128) {
        char ch = static_cast<char>(codepoint);
        insert_text(StringView(&ch, 1));
        cursor_blink_time_ = 0.0f;
        return true;
    }

    // TODO: Proper UTF-8 encoding for non-ASCII
    return false;
}

bool TextInput::on_focus_gained() {
    Widget::on_focus_gained();
    cursor_blink_time_ = 0.0f;
    return true;
}

bool TextInput::on_focus_lost() {
    Widget::on_focus_lost();
    clear_selection();
    return true;
}

} // namespace frost
