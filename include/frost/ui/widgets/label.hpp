#pragma once

#include "frost/ui/widget.hpp"
#include "frost/graphics/color.hpp"

namespace frost {

/// Text alignment options
enum class TextAlign : u8 {
    Left,
    Center,
    Right,
};

/// Vertical alignment options
enum class VerticalAlign : u8 {
    Top,
    Middle,
    Bottom,
};

/// Label style configuration
struct LabelStyle {
    Color text_color{Color::white()};
    f32 font_size{16.0f};
    TextAlign text_align{TextAlign::Left};
    VerticalAlign vertical_align{VerticalAlign::Middle};
};

/// A simple text display widget
class Label : public Widget {
public:
    /// Create a label with text
    static Unique<Label> create(StringView text = "");

    /// Create a label with text and style
    static Unique<Label> create(StringView text, const LabelStyle& style);

    ~Label() override = default;

    // ─────────────────────────────────────────────────────────────────────────
    // Text
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] const String& text() const { return text_; }
    void set_text(StringView text);

    // ─────────────────────────────────────────────────────────────────────────
    // Style
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] const LabelStyle& style() const { return style_; }
    void set_style(const LabelStyle& style);

    void set_text_color(Color color);
    void set_font_size(f32 size);
    void set_text_align(TextAlign align);
    void set_vertical_align(VerticalAlign align);

    // ─────────────────────────────────────────────────────────────────────────
    // Widget Overrides
    // ─────────────────────────────────────────────────────────────────────────

    void render(DrawList& draw_list) override;
    [[nodiscard]] Size2D measure(const LayoutConstraints& constraints) override;

protected:
    Label() = default;
    explicit Label(StringView text, const LabelStyle& style);

private:
    /// Calculate text width (placeholder - needs font metrics)
    [[nodiscard]] f32 calculate_text_width() const;

    /// Calculate text height
    [[nodiscard]] f32 calculate_text_height() const;

    String text_;
    LabelStyle style_;
};

} // namespace frost
