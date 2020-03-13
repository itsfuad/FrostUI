#pragma once

#include "frost/ui/widget.hpp"

namespace frost {

/// Direction for flex layout
enum class FlexDirection : u8 {
    Row,            ///< Children arranged horizontally (left to right)
    RowReverse,     ///< Children arranged horizontally (right to left)
    Column,         ///< Children arranged vertically (top to bottom)
    ColumnReverse,  ///< Children arranged vertically (bottom to top)
};

/// Main axis alignment
enum class FlexJustify : u8 {
    Start,          ///< Pack items at start
    End,            ///< Pack items at end
    Center,         ///< Pack items around center
    SpaceBetween,   ///< Distribute items evenly, first at start, last at end
    SpaceAround,    ///< Distribute items evenly with equal space around
    SpaceEvenly,    ///< Distribute items evenly with equal space between
};

/// Cross axis alignment
enum class FlexAlign : u8 {
    Start,          ///< Align at cross start
    End,            ///< Align at cross end
    Center,         ///< Center on cross axis
    Stretch,        ///< Stretch to fill cross axis
};

/// Flex item properties
struct FlexItem {
    f32 grow{0.0f};         ///< How much this item should grow relative to others
    f32 shrink{1.0f};       ///< How much this item should shrink relative to others
    f32 basis{-1.0f};       ///< Base size (-1 = auto, use measure())
    FlexAlign align_self{FlexAlign::Stretch};  ///< Override container's align_items
};

/// Container style configuration
struct ContainerStyle {
    FlexDirection direction{FlexDirection::Column};
    FlexJustify justify{FlexJustify::Start};
    FlexAlign align_items{FlexAlign::Stretch};
    f32 gap{0.0f};          ///< Space between items
    bool wrap{false};       ///< Allow wrapping to next line (TODO: not yet implemented)
};

/// A container widget that arranges children using flexbox-like layout
class Container : public Widget {
public:
    /// Create with default style
    static Unique<Container> create();

    /// Create with specific style
    static Unique<Container> create(const ContainerStyle& style);

    ~Container() override = default;

    // ─────────────────────────────────────────────────────────────────────────
    // Style
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] const ContainerStyle& style() const { return style_; }
    void set_style(const ContainerStyle& style);

    void set_direction(FlexDirection direction);
    void set_justify(FlexJustify justify);
    void set_align_items(FlexAlign align);
    void set_gap(f32 gap);

    // ─────────────────────────────────────────────────────────────────────────
    // Flex Item Properties
    // ─────────────────────────────────────────────────────────────────────────

    /// Set flex properties for a child by index
    void set_child_flex(usize index, const FlexItem& item);

    /// Get flex properties for a child
    [[nodiscard]] const FlexItem& get_child_flex(usize index) const;

    // ─────────────────────────────────────────────────────────────────────────
    // Widget Overrides
    // ─────────────────────────────────────────────────────────────────────────

    void render(DrawList& draw_list) override;
    [[nodiscard]] Size2D measure(const LayoutConstraints& constraints) override;
    void layout(const Rect& bounds) override;

    void add_child(Unique<Widget> child) override;
    Unique<Widget> remove_child(Widget* child) override;

protected:
    Container() = default;
    explicit Container(const ContainerStyle& style);

private:
    /// Check if layout is horizontal (Row or RowReverse)
    [[nodiscard]] bool is_horizontal() const;

    /// Check if layout is reversed
    [[nodiscard]] bool is_reversed() const;

    /// Get main axis size from Size2D
    [[nodiscard]] f32 main_size(const Size2D& size) const;

    /// Get cross axis size from Size2D
    [[nodiscard]] f32 cross_size(const Size2D& size) const;

    /// Calculate spacing based on justify mode
    void calculate_spacing(f32 available_space, f32 total_child_main,
                          usize child_count, f32& start_offset,
                          f32& between_spacing) const;

    ContainerStyle style_;
    Vector<FlexItem> flex_items_;
};

/// Convenience function to create a horizontal container
inline Unique<Container> HBox(f32 gap = 0.0f) {
    ContainerStyle style;
    style.direction = FlexDirection::Row;
    style.gap = gap;
    return Container::create(style);
}

/// Convenience function to create a vertical container
inline Unique<Container> VBox(f32 gap = 0.0f) {
    ContainerStyle style;
    style.direction = FlexDirection::Column;
    style.gap = gap;
    return Container::create(style);
}

/// Convenience function to create a centered container
inline Unique<Container> Center() {
    ContainerStyle style;
    style.direction = FlexDirection::Column;
    style.justify = FlexJustify::Center;
    style.align_items = FlexAlign::Center;
    return Container::create(style);
}

} // namespace frost
