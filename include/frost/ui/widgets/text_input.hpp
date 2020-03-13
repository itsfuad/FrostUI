#pragma once

#include "frost/ui/widget.hpp"
#include "frost/graphics/color.hpp"
#include "frost/core/signals.hpp"

namespace frost {

/// Text input style configuration
struct TextInputStyle {
    Color background_color{0.1f, 0.1f, 0.15f, 1.0f};
    Color focus_color{0.15f, 0.15f, 0.2f, 1.0f};
    Color border_color{0.3f, 0.3f, 0.4f, 1.0f};
    Color focus_border_color{0.4f, 0.5f, 0.8f, 1.0f};
    Color text_color{Color::white()};
    Color placeholder_color{0.5f, 0.5f, 0.5f, 1.0f};
    Color cursor_color{Color::white()};
    Color selection_color{0.3f, 0.4f, 0.6f, 0.5f};
    f32 font_size{16.0f};
    f32 corner_radius{4.0f};
    f32 border_width{1.0f};
    Edges padding{8.0f, 12.0f, 8.0f, 12.0f};  // top, right, bottom, left
};

/// A single-line text input widget
class TextInput : public Widget {
public:
    /// Create a text input
    static Unique<TextInput> create();

    /// Create a text input with placeholder
    static Unique<TextInput> create(StringView placeholder);

    /// Create a text input with placeholder and style
    static Unique<TextInput> create(StringView placeholder, const TextInputStyle& style);

    ~TextInput() override = default;

    // ─────────────────────────────────────────────────────────────────────────
    // Signals
    // ─────────────────────────────────────────────────────────────────────────

    /// Emitted when text changes
    Signal<const String&> on_text_changed;

    /// Emitted when Enter is pressed
    Signal<const String&> on_submit;

    // ─────────────────────────────────────────────────────────────────────────
    // Text
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] const String& text() const { return text_; }
    void set_text(StringView text);

    [[nodiscard]] const String& placeholder() const { return placeholder_; }
    void set_placeholder(StringView placeholder);

    // ─────────────────────────────────────────────────────────────────────────
    // Selection
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] usize cursor_position() const { return cursor_pos_; }
    void set_cursor_position(usize pos);

    [[nodiscard]] bool has_selection() const { return selection_start_ != selection_end_; }
    void select_all();
    void clear_selection();

    // ─────────────────────────────────────────────────────────────────────────
    // Style
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] const TextInputStyle& style() const { return style_; }
    void set_style(const TextInputStyle& style);

    // ─────────────────────────────────────────────────────────────────────────
    // Widget Overrides
    // ─────────────────────────────────────────────────────────────────────────

    void update(f64 delta_time) override;
    void render(DrawList& draw_list) override;
    [[nodiscard]] Size2D measure(const LayoutConstraints& constraints) override;

    bool on_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods) override;
    bool on_key(KeyCode key, KeyAction action, ModifierFlags mods) override;
    bool on_char(u32 codepoint) override;
    bool on_focus_gained() override;
    bool on_focus_lost() override;

protected:
    TextInput() = default;
    explicit TextInput(StringView placeholder, const TextInputStyle& style);

private:
    /// Insert text at cursor position
    void insert_text(StringView text);

    /// Delete selected text or character
    void delete_selection();

    /// Delete character before cursor
    void delete_backward();

    /// Delete character after cursor
    void delete_forward();

    /// Move cursor
    void move_cursor(i32 delta, bool extend_selection);

    /// Move cursor to position
    void move_cursor_to(usize pos, bool extend_selection);

    /// Get current background color based on state
    [[nodiscard]] Color current_background_color() const;

    /// Get current border color based on state
    [[nodiscard]] Color current_border_color() const;

    /// Calculate text width (placeholder - needs font metrics)
    [[nodiscard]] f32 calculate_text_width(StringView text) const;

    /// Calculate text height
    [[nodiscard]] f32 calculate_text_height() const;

    /// Get character position from x coordinate
    [[nodiscard]] usize get_char_at_x(f32 x) const;

    String text_;
    String placeholder_;
    TextInputStyle style_;

    usize cursor_pos_{0};
    usize selection_start_{0};
    usize selection_end_{0};

    f32 cursor_blink_time_{0.0f};
    static constexpr f32 kCursorBlinkRate = 0.5f;

    f32 scroll_offset_{0.0f};
};

} // namespace frost
