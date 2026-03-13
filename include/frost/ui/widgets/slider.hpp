#pragma once

#include "frost/ui/widget.hpp"
#include "frost/graphics/color.hpp"
#include "frost/core/signals.hpp"

namespace frost {

struct SliderStyle {
    Color track_color{0.22f, 0.22f, 0.28f, 1.0f};
    Color fill_color{0.35f, 0.65f, 0.95f, 1.0f};
    Color thumb_color{0.9f, 0.9f, 0.95f, 1.0f};
    Color disabled_color{0.35f, 0.35f, 0.35f, 1.0f};
    f32 track_height{6.0f};
    f32 thumb_radius{9.0f};
    Edges padding{8.0f, 8.0f, 8.0f, 8.0f};
};

class Slider : public Widget {
public:
    static Unique<Slider> create();
    static Unique<Slider> create(f32 min_value, f32 max_value, f32 initial_value = 0.0f);
    static Unique<Slider> create(f32 min_value, f32 max_value, f32 initial_value, const SliderStyle& style);

    ~Slider() override = default;

    Signal<f32> on_value_changed;

    [[nodiscard]] f32 value() const { return value_; }
    void set_value(f32 value);

    [[nodiscard]] f32 min_value() const { return min_value_; }
    [[nodiscard]] f32 max_value() const { return max_value_; }
    void set_range(f32 min_value, f32 max_value);

    [[nodiscard]] f32 step() const { return step_; }
    void set_step(f32 step);

    [[nodiscard]] const SliderStyle& style() const { return style_; }
    void set_style(const SliderStyle& style);

    void render(DrawList& draw_list) override;
    [[nodiscard]] Size2D measure(const LayoutConstraints& constraints) override;
    bool on_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods) override;
    bool on_mouse_move(f64 x, f64 y) override;
    bool on_key(KeyCode key, KeyAction action, ModifierFlags mods) override;

protected:
    Slider() = default;
    Slider(f32 min_value, f32 max_value, f32 initial_value, const SliderStyle& style);

private:
    [[nodiscard]] f32 normalized_value() const;
    [[nodiscard]] f32 value_from_x(f32 x) const;
    [[nodiscard]] f32 quantize_value(f32 value) const;

    void set_value_internal(f32 value, bool emit_signal);

    f32 min_value_{0.0f};
    f32 max_value_{1.0f};
    f32 value_{0.0f};
    f32 step_{0.0f};
    bool dragging_{false};
    f32 last_mouse_x_{0.0f};

    SliderStyle style_;
};

} // namespace frost
