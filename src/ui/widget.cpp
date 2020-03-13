#include "frost/ui/widget.hpp"
#include "frost/graphics/draw_list.hpp"

namespace frost {

void Widget::update(f64 delta_time) {
    // Update children
    for (auto& child : children_) {
        if (child && child->is_visible()) {
            child->update(delta_time);
        }
    }
}

void Widget::render(DrawList& draw_list) {
    // Render children
    for (auto& child : children_) {
        if (child && child->is_visible()) {
            child->render(draw_list);
        }
    }
}

Size2D Widget::measure(const LayoutConstraints& constraints) {
    // Default: use preferred size or minimum size
    f32 width = constraints.preferred_width.value_or(constraints.min_size.width);
    f32 height = constraints.preferred_height.value_or(constraints.min_size.height);
    return Size2D{width, height};
}

void Widget::layout(const Rect& bounds) {
    bounds_ = bounds;
    dirty_ = false;

    // Default: children fill the bounds minus padding
    Rect content_bounds{
        bounds_.x + padding_.left,
        bounds_.y + padding_.top,
        bounds_.width - padding_.left - padding_.right,
        bounds_.height - padding_.top - padding_.bottom
    };

    for (auto& child : children_) {
        if (child) {
            child->layout(content_bounds);
        }
    }
}

bool Widget::on_mouse_enter() {
    add_state_flag(WidgetState::Hovered);
    return false;
}

bool Widget::on_mouse_leave() {
    remove_state_flag(WidgetState::Hovered);
    remove_state_flag(WidgetState::Pressed);
    return false;
}

bool Widget::on_mouse_move(f64 x, f64 y) {
    return false;
}

bool Widget::on_mouse_button(MouseButton button, KeyAction action, ModifierFlags mods) {
    if (button == MouseButton::Left) {
        if (action == KeyAction::Press) {
            add_state_flag(WidgetState::Pressed);
        } else if (action == KeyAction::Release) {
            remove_state_flag(WidgetState::Pressed);
        }
    }
    return false;
}

bool Widget::on_key(KeyCode key, KeyAction action, ModifierFlags mods) {
    return false;
}

bool Widget::on_char(u32 codepoint) {
    return false;
}

bool Widget::on_focus_gained() {
    add_state_flag(WidgetState::Focused);
    return false;
}

bool Widget::on_focus_lost() {
    remove_state_flag(WidgetState::Focused);
    return false;
}

void Widget::set_visible(bool visible) {
    visible_ = visible;
    mark_dirty();
}

void Widget::set_enabled(bool enabled) {
    enabled_ = enabled;
    if (!enabled_) {
        add_state_flag(WidgetState::Disabled);
    } else {
        remove_state_flag(WidgetState::Disabled);
    }
    mark_dirty();
}

void Widget::set_margin(const Edges& margin) {
    margin_ = margin;
    mark_dirty();
}

void Widget::set_padding(const Edges& padding) {
    padding_ = padding;
    mark_dirty();
}

void Widget::add_child(Unique<Widget> child) {
    if (child) {
        child->parent_ = this;
        children_.push_back(std::move(child));
        mark_dirty();
    }
}

Unique<Widget> Widget::remove_child(Widget* child) {
    for (auto it = children_.begin(); it != children_.end(); ++it) {
        if (it->get() == child) {
            Unique<Widget> removed = std::move(*it);
            removed->parent_ = nullptr;
            children_.erase(it);
            mark_dirty();
            return removed;
        }
    }
    return nullptr;
}

bool Widget::hit_test(Point2D point) const {
    return visible_ && enabled_ && bounds_.contains(point);
}

Widget* Widget::find_widget_at(Point2D point) {
    if (!hit_test(point)) {
        return nullptr;
    }

    // Check children in reverse order (top-most first)
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if (*it) {
            Widget* found = (*it)->find_widget_at(point);
            if (found) {
                return found;
            }
        }
    }

    return this;
}

void Widget::mark_dirty() {
    dirty_ = true;
    if (parent_) {
        parent_->mark_dirty();
    }
}

} // namespace frost
