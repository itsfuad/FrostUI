#pragma once

#include "frost/core/types.hpp"
#include "frost/core/math.hpp"
#include "frost/core/signals.hpp"
#include "frost/platform/input.hpp"

namespace frost {

// Forward declarations
class DrawList;
class Widget;

/// Widget state flags
enum class WidgetState : u8 {
    Normal = 0,
    Hovered = 1 << 0,
    Pressed = 1 << 1,
    Focused = 1 << 2,
    Disabled = 1 << 3,
};

inline constexpr WidgetState operator|(WidgetState a, WidgetState b) {
    return static_cast<WidgetState>(static_cast<u8>(a) | static_cast<u8>(b));
}

inline constexpr WidgetState operator&(WidgetState a, WidgetState b) {
    return static_cast<WidgetState>(static_cast<u8>(a) & static_cast<u8>(b));
}

inline constexpr WidgetState& operator|=(WidgetState& a, WidgetState b) {
    return a = a | b;
}

inline constexpr WidgetState& operator&=(WidgetState& a, WidgetState b) {
    return a = a & b;
}

inline constexpr WidgetState operator~(WidgetState a) {
    return static_cast<WidgetState>(~static_cast<u8>(a));
}

[[nodiscard]] inline constexpr bool has_state(WidgetState states, WidgetState test) {
    return (static_cast<u8>(states) & static_cast<u8>(test)) != 0;
}

/// Layout constraints for measuring widgets
struct LayoutConstraints {
    Size2D min_size{0, 0};
    Size2D max_size{std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max()};
    Option<f32> preferred_width;
    Option<f32> preferred_height;
};

/// Base class for all UI widgets
class Widget : public NonCopyable {
public:
    virtual ~Widget() = default;

    // ─────────────────────────────────────────────────────────────────────────
    // Lifecycle
    // ─────────────────────────────────────────────────────────────────────────

    /// Update widget state (called each frame)
    virtual void update(f64 delta_time);

    /// Render the widget to a draw list
    virtual void render(DrawList& draw_list);

    /// Measure the widget's preferred size given constraints
    [[nodiscard]] virtual Size2D measure(const LayoutConstraints& constraints);

    /// Layout the widget within the given bounds
    virtual void layout(const Rect& bounds);

    // ─────────────────────────────────────────────────────────────────────────
    // Event Handling
    // ─────────────────────────────────────────────────────────────────────────

    /// Called when mouse enters the widget
    virtual bool on_mouse_enter();

    /// Called when mouse leaves the widget
    virtual bool on_mouse_leave();

    /// Called when mouse moves over the widget
    virtual bool on_mouse_move(f64 x, f64 y);

    /// Called on mouse button press/release
    virtual bool on_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods);

    /// Called on key press/release
    virtual bool on_key(KeyCode key, KeyAction action, ModifierFlags mods);

    /// Called for character input
    virtual bool on_char(u32 codepoint);

    /// Called when widget gains focus
    virtual bool on_focus_gained();

    /// Called when widget loses focus
    virtual bool on_focus_lost();

    // ─────────────────────────────────────────────────────────────────────────
    // Properties
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] const Rect& bounds() const { return bounds_; }
    [[nodiscard]] WidgetState state() const { return state_; }
    [[nodiscard]] bool is_visible() const { return visible_; }
    [[nodiscard]] bool is_enabled() const { return enabled_; }
    [[nodiscard]] bool is_hovered() const { return has_state(state_, WidgetState::Hovered); }
    [[nodiscard]] bool is_pressed() const { return has_state(state_, WidgetState::Pressed); }
    [[nodiscard]] bool is_focused() const { return has_state(state_, WidgetState::Focused); }
    [[nodiscard]] Widget* parent() const { return parent_; }

    void set_visible(bool visible);
    void set_enabled(bool enabled);

    [[nodiscard]] const Edges& margin() const { return margin_; }
    [[nodiscard]] const Edges& padding() const { return padding_; }
    void set_margin(const Edges& margin);
    void set_padding(const Edges& padding);

    // ─────────────────────────────────────────────────────────────────────────
    // Hierarchy
    // ─────────────────────────────────────────────────────────────────────────

    /// Add a child widget
    virtual void add_child(Unique<Widget> child);

    /// Remove a child widget
    virtual Unique<Widget> remove_child(Widget* child);

    /// Get child widgets
    [[nodiscard]] const Vector<Unique<Widget>>& children() const { return children_; }

    /// Get mutable child widgets
    [[nodiscard]] Vector<Unique<Widget>>& children() { return children_; }

    // ─────────────────────────────────────────────────────────────────────────
    // Hit Testing
    // ─────────────────────────────────────────────────────────────────────────

    /// Test if a point is within the widget bounds
    [[nodiscard]] virtual bool hit_test(Point2D point) const;

    /// Find the deepest child widget at the given point
    [[nodiscard]] Widget* find_widget_at(Point2D point);

    // ─────────────────────────────────────────────────────────────────────────
    // Dirty Tracking
    // ─────────────────────────────────────────────────────────────────────────

    /// Mark the widget as needing re-layout
    void mark_dirty();

    /// Check if widget needs re-layout
    [[nodiscard]] bool is_dirty() const { return dirty_; }

    /// Clear the dirty flag
    void clear_dirty() { dirty_ = false; }

protected:
    Widget() = default;

    void set_state(WidgetState state) { state_ = state; }
    void add_state_flag(WidgetState flag) { state_ |= flag; }
    void remove_state_flag(WidgetState flag) { state_ = state_ & ~flag; }

    Rect bounds_;
    Edges margin_;
    Edges padding_;
    WidgetState state_{WidgetState::Normal};
    bool visible_{true};
    bool enabled_{true};
    bool dirty_{true};

    Widget* parent_{nullptr};
    Vector<Unique<Widget>> children_;
};

} // namespace frost
