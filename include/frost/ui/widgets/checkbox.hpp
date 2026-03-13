#pragma once

#include "frost/ui/widget.hpp"
#include "frost/graphics/color.hpp"
#include "frost/core/signals.hpp"

namespace frost {

struct CheckboxStyle {
    Color box_background{0.12f, 0.12f, 0.16f, 1.0f};
    Color box_border{0.35f, 0.35f, 0.45f, 1.0f};
    Color check_color{0.45f, 0.8f, 0.45f, 1.0f};
    Color text_color{Color::white()};
    Color disabled_color{0.35f, 0.35f, 0.35f, 1.0f};
    f32 font_size{16.0f};
    f32 box_size{20.0f};
    f32 border_width{1.5f};
    f32 corner_radius{3.0f};
    f32 spacing{8.0f};
    Edges padding{4.0f, 4.0f, 4.0f, 4.0f};
};

class Checkbox : public Widget {
public:
    static Unique<Checkbox> create(StringView text = "");
    static Unique<Checkbox> create(StringView text, const CheckboxStyle& style);

    ~Checkbox() override = default;

    Signal<bool> on_toggled;

    [[nodiscard]] bool checked() const { return checked_; }
    void set_checked(bool checked);
    void toggle();

    [[nodiscard]] const String& text() const { return text_; }
    void set_text(StringView text);

    [[nodiscard]] const CheckboxStyle& style() const { return style_; }
    void set_style(const CheckboxStyle& style);

    void render(DrawList& draw_list) override;
    [[nodiscard]] Size2D measure(const LayoutConstraints& constraints) override;
    bool on_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods) override;
    bool on_key(KeyCode key, KeyAction action, ModifierFlags mods) override;

protected:
    Checkbox() = default;
    explicit Checkbox(StringView text, const CheckboxStyle& style);

private:
    [[nodiscard]] f32 calculate_text_width() const;
    [[nodiscard]] f32 calculate_text_height() const;

    bool checked_{false};
    String text_;
    CheckboxStyle style_;
};

} // namespace frost
