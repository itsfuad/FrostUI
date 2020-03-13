#pragma once

#include "frost/ui/widget.hpp"
#include "frost/graphics/color.hpp"
#include "frost/core/signals.hpp"

namespace frost {

/// Button style configuration
struct ButtonStyle {
    Color background_color{0.2f, 0.2f, 0.3f, 1.0f};
    Color hover_color{0.3f, 0.3f, 0.4f, 1.0f};
    Color pressed_color{0.15f, 0.15f, 0.25f, 1.0f};
    Color disabled_color{0.15f, 0.15f, 0.15f, 1.0f};
    Color text_color{Color::white()};
    Color disabled_text_color{0.5f, 0.5f, 0.5f, 1.0f};
    f32 font_size{16.0f};
    f32 corner_radius{4.0f};
    Edges padding{8.0f, 16.0f, 8.0f, 16.0f};  // top, right, bottom, left
};

/// A clickable button widget
class Button : public Widget {
public:
    /// Create a button with text
    static Unique<Button> create(StringView text = "");

    /// Create a button with text and style
    static Unique<Button> create(StringView text, const ButtonStyle& style);

    ~Button() override = default;

    // ─────────────────────────────────────────────────────────────────────────
    // Signals
    // ─────────────────────────────────────────────────────────────────────────

    /// Emitted when the button is clicked
    Signal<> on_click;

    // ─────────────────────────────────────────────────────────────────────────
    // Text
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] const String& text() const { return text_; }
    void set_text(StringView text);

    // ─────────────────────────────────────────────────────────────────────────
    // Style
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] const ButtonStyle& style() const { return style_; }
    void set_style(const ButtonStyle& style);

    // ─────────────────────────────────────────────────────────────────────────
    // Widget Overrides
    // ─────────────────────────────────────────────────────────────────────────

    void render(DrawList& draw_list) override;
    [[nodiscard]] Size2D measure(const LayoutConstraints& constraints) override;

    bool on_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods) override;

protected:
    Button() = default;
    explicit Button(StringView text, const ButtonStyle& style);

private:
    /// Get current background color based on state
    [[nodiscard]] Color current_background_color() const;

    /// Get current text color based on state
    [[nodiscard]] Color current_text_color() const;

    /// Calculate text width (placeholder - needs font metrics)
    [[nodiscard]] f32 calculate_text_width() const;

    /// Calculate text height
    [[nodiscard]] f32 calculate_text_height() const;

    String text_;
    ButtonStyle style_;
};

} // namespace frost
