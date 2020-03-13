#include "frost/ui/layout.hpp"
#include "frost/graphics/draw_list.hpp"
#include <algorithm>

namespace frost {

Unique<Container> Container::create() {
    return Unique<Container>(new Container());
}

Unique<Container> Container::create(const ContainerStyle& style) {
    return Unique<Container>(new Container(style));
}

Container::Container(const ContainerStyle& style)
    : style_(style) {}

void Container::set_style(const ContainerStyle& style) {
    style_ = style;
    mark_dirty();
}

void Container::set_direction(FlexDirection direction) {
    if (style_.direction != direction) {
        style_.direction = direction;
        mark_dirty();
    }
}

void Container::set_justify(FlexJustify justify) {
    if (style_.justify != justify) {
        style_.justify = justify;
        mark_dirty();
    }
}

void Container::set_align_items(FlexAlign align) {
    if (style_.align_items != align) {
        style_.align_items = align;
        mark_dirty();
    }
}

void Container::set_gap(f32 gap) {
    if (style_.gap != gap) {
        style_.gap = gap;
        mark_dirty();
    }
}

void Container::set_child_flex(usize index, const FlexItem& item) {
    if (index < flex_items_.size()) {
        flex_items_[index] = item;
        mark_dirty();
    }
}

const FlexItem& Container::get_child_flex(usize index) const {
    static const FlexItem default_item;
    if (index < flex_items_.size()) {
        return flex_items_[index];
    }
    return default_item;
}

void Container::add_child(Unique<Widget> child) {
    if (child) {
        flex_items_.push_back(FlexItem{});
        Widget::add_child(std::move(child));
    }
}

Unique<Widget> Container::remove_child(Widget* child) {
    // Find the index of the child
    for (usize i = 0; i < children_.size(); ++i) {
        if (children_[i].get() == child) {
            flex_items_.erase(flex_items_.begin() + static_cast<ptrdiff_t>(i));
            break;
        }
    }
    return Widget::remove_child(child);
}

void Container::render(DrawList& draw_list) {
    // Container itself is transparent by default - just render children
    Widget::render(draw_list);
}

bool Container::is_horizontal() const {
    return style_.direction == FlexDirection::Row ||
           style_.direction == FlexDirection::RowReverse;
}

bool Container::is_reversed() const {
    return style_.direction == FlexDirection::RowReverse ||
           style_.direction == FlexDirection::ColumnReverse;
}

f32 Container::main_size(const Size2D& size) const {
    return is_horizontal() ? size.width : size.height;
}

f32 Container::cross_size(const Size2D& size) const {
    return is_horizontal() ? size.height : size.width;
}

void Container::calculate_spacing(f32 available_space, f32 total_child_main,
                                  usize child_count, f32& start_offset,
                                  f32& between_spacing) const {
    f32 free_space = available_space - total_child_main;
    if (free_space < 0) free_space = 0;

    usize gaps = child_count > 1 ? child_count - 1 : 0;

    switch (style_.justify) {
        case FlexJustify::Start:
            start_offset = 0;
            between_spacing = style_.gap;
            break;

        case FlexJustify::End:
            start_offset = free_space - gaps * style_.gap;
            between_spacing = style_.gap;
            break;

        case FlexJustify::Center:
            start_offset = (free_space - gaps * style_.gap) / 2.0f;
            between_spacing = style_.gap;
            break;

        case FlexJustify::SpaceBetween:
            start_offset = 0;
            between_spacing = gaps > 0 ? free_space / static_cast<f32>(gaps) : 0;
            break;

        case FlexJustify::SpaceAround:
            if (child_count > 0) {
                f32 space_per_item = free_space / static_cast<f32>(child_count);
                start_offset = space_per_item / 2.0f;
                between_spacing = space_per_item;
            } else {
                start_offset = 0;
                between_spacing = 0;
            }
            break;

        case FlexJustify::SpaceEvenly:
            if (child_count > 0) {
                f32 total_gaps = static_cast<f32>(child_count + 1);
                f32 even_space = free_space / total_gaps;
                start_offset = even_space;
                between_spacing = even_space;
            } else {
                start_offset = 0;
                between_spacing = 0;
            }
            break;
    }
}

Size2D Container::measure(const LayoutConstraints& constraints) {
    // Count visible children
    Vector<Widget*> visible_children;
    for (auto& child : children_) {
        if (child && child->is_visible()) {
            visible_children.push_back(child.get());
        }
    }

    if (visible_children.empty()) {
        return Size2D{
            padding_.left + padding_.right,
            padding_.top + padding_.bottom
        };
    }

    f32 total_main = 0;
    f32 max_cross = 0;
    usize count = 0;

    // Measure each child
    for (auto* child : visible_children) {
        Size2D child_size = child->measure(constraints);

        total_main += main_size(child_size);
        max_cross = std::max(max_cross, cross_size(child_size));
        ++count;
    }

    // Add gaps
    if (count > 1) {
        total_main += style_.gap * static_cast<f32>(count - 1);
    }

    // Add padding
    f32 padding_main = is_horizontal()
        ? padding_.left + padding_.right
        : padding_.top + padding_.bottom;
    f32 padding_cross = is_horizontal()
        ? padding_.top + padding_.bottom
        : padding_.left + padding_.right;

    total_main += padding_main;
    max_cross += padding_cross;

    // Return size based on direction
    if (is_horizontal()) {
        return Size2D{total_main, max_cross};
    } else {
        return Size2D{max_cross, total_main};
    }
}

void Container::layout(const Rect& bounds) {
    bounds_ = bounds;
    dirty_ = false;

    // Content area (bounds minus padding)
    Rect content{
        bounds_.x + padding_.left,
        bounds_.y + padding_.top,
        bounds_.width - padding_.left - padding_.right,
        bounds_.height - padding_.top - padding_.bottom
    };

    // Collect visible children with their flex items
    struct ChildInfo {
        Widget* widget;
        FlexItem flex;
        Size2D measured_size;
        f32 final_main;
        f32 final_cross;
    };

    Vector<ChildInfo> visible_children;
    for (usize i = 0; i < children_.size(); ++i) {
        if (children_[i] && children_[i]->is_visible()) {
            ChildInfo info;
            info.widget = children_[i].get();
            info.flex = i < flex_items_.size() ? flex_items_[i] : FlexItem{};

            // Measure child
            LayoutConstraints child_constraints;
            child_constraints.max_size = Size2D{content.width, content.height};
            info.measured_size = info.widget->measure(child_constraints);

            // Determine base main size
            if (info.flex.basis >= 0) {
                info.final_main = info.flex.basis;
            } else {
                info.final_main = main_size(info.measured_size);
            }

            info.final_cross = cross_size(info.measured_size);
            visible_children.push_back(info);
        }
    }

    if (visible_children.empty()) {
        return;
    }

    // Calculate total main size and grow/shrink factors
    f32 total_main = 0;
    f32 total_grow = 0;
    f32 total_shrink = 0;

    for (auto& info : visible_children) {
        total_main += info.final_main;
        total_grow += info.flex.grow;
        total_shrink += info.flex.shrink;
    }

    // Add gaps
    f32 gaps_size = style_.gap * static_cast<f32>(visible_children.size() - 1);
    total_main += gaps_size;

    // Available main axis space
    f32 available_main = is_horizontal() ? content.width : content.height;
    f32 available_cross = is_horizontal() ? content.height : content.width;
    f32 free_space = available_main - total_main;

    // Distribute free space (grow) or reclaim space (shrink)
    if (free_space > 0 && total_grow > 0) {
        // Grow items
        for (auto& info : visible_children) {
            if (info.flex.grow > 0) {
                f32 extra = free_space * (info.flex.grow / total_grow);
                info.final_main += extra;
            }
        }
    } else if (free_space < 0 && total_shrink > 0) {
        // Shrink items
        f32 deficit = -free_space;
        for (auto& info : visible_children) {
            if (info.flex.shrink > 0) {
                f32 reduction = deficit * (info.flex.shrink / total_shrink);
                info.final_main = std::max(0.0f, info.final_main - reduction);
            }
        }
    }

    // Recalculate total for spacing
    f32 new_total_main = gaps_size;
    for (auto& info : visible_children) {
        new_total_main += info.final_main;
    }

    // Calculate starting position and spacing
    f32 start_offset, between_spacing;
    calculate_spacing(available_main, new_total_main - gaps_size,
                     visible_children.size(), start_offset, between_spacing);

    // Position children
    f32 main_pos = start_offset;

    // Handle reverse direction
    Vector<usize> order;
    order.reserve(visible_children.size());
    for (usize i = 0; i < visible_children.size(); ++i) {
        order.push_back(i);
    }
    if (is_reversed()) {
        std::reverse(order.begin(), order.end());
    }

    for (usize idx : order) {
        auto& info = visible_children[idx];

        // Determine cross axis size and position based on alignment
        FlexAlign align = info.flex.align_self;
        if (align == FlexAlign::Stretch && style_.align_items != FlexAlign::Stretch) {
            // align_self defaults to container's align_items if not explicitly set
            // Since FlexItem defaults align_self to Stretch, check parent
        }
        align = (info.flex.align_self == FlexAlign::Stretch)
            ? style_.align_items
            : info.flex.align_self;

        f32 cross_pos = 0;
        f32 final_cross = info.final_cross;

        switch (align) {
            case FlexAlign::Start:
                cross_pos = 0;
                break;
            case FlexAlign::End:
                cross_pos = available_cross - final_cross;
                break;
            case FlexAlign::Center:
                cross_pos = (available_cross - final_cross) / 2.0f;
                break;
            case FlexAlign::Stretch:
                cross_pos = 0;
                final_cross = available_cross;
                break;
        }

        // Build child bounds
        Rect child_bounds;
        if (is_horizontal()) {
            child_bounds = Rect{
                content.x + main_pos,
                content.y + cross_pos,
                info.final_main,
                final_cross
            };
        } else {
            child_bounds = Rect{
                content.x + cross_pos,
                content.y + main_pos,
                final_cross,
                info.final_main
            };
        }

        // Layout child
        info.widget->layout(child_bounds);

        // Advance position
        main_pos += info.final_main + between_spacing;
    }
}

} // namespace frost
