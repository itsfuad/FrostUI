#pragma once

#include "frost/ui/widget.hpp"
#include "frost/graphics/color.hpp"
#include "frost/core/signals.hpp"

namespace frost {

struct RadioButtonStyle {
    Color outer_color{0.35f, 0.35f, 0.45f, 1.0f};
    Color inner_color{0.45f, 0.8f, 0.45f, 1.0f};
    Color text_color{Color::white()};
    Color disabled_color{0.35f, 0.35f, 0.35f, 1.0f};
    f32 font_size{16.0f};
    f32 radius{10.0f};
    f32 border_width{1.5f};
    f32 spacing{8.0f};
    Edges padding{4.0f, 4.0f, 4.0f, 4.0f};
};

class RadioButton : public Widget {
public:
    static Unique<RadioButton> create(StringView text = "", StringView group = "default");
    static Unique<RadioButton> create(StringView text, StringView group, const RadioButtonStyle& style);

    ~RadioButton() override;

    Signal<bool> on_selected;

    [[nodiscard]] bool selected() const { return selected_; }
    void set_selected(bool selected);

    [[nodiscard]] const String& text() const { return text_; }
    void set_text(StringView text);

    [[nodiscard]] const String& group() const { return group_; }
    void set_group(StringView group);

    [[nodiscard]] const RadioButtonStyle& style() const { return style_; }
    void set_style(const RadioButtonStyle& style);

    void render(DrawList& draw_list) override;
    [[nodiscard]] Size2D measure(const LayoutConstraints& constraints) override;
    bool on_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods) override;
    bool on_key(KeyCode key, KeyAction action, ModifierFlags mods) override;

protected:
    RadioButton() = default;
    explicit RadioButton(StringView text, StringView group, const RadioButtonStyle& style);

private:
    [[nodiscard]] f32 calculate_text_width() const;
    [[nodiscard]] f32 calculate_text_height() const;

    void register_in_group();
    void unregister_from_group();
    void select_exclusive();

    bool selected_{false};
    String text_;
    String group_;
    RadioButtonStyle style_;
};

} // namespace frost
