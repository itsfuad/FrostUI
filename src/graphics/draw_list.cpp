#include "frost/graphics/draw_list.hpp"
#include <cmath>

namespace frost {

void DrawList::clear() {
    vertices_.clear();
    indices_.clear();
    commands_.clear();
    text_commands_.clear();
    clip_stack_.clear();
}

void DrawList::reserve(usize vertex_count, usize index_count) {
    vertices_.reserve(vertex_count);
    indices_.reserve(index_count);
}

void DrawList::add_vertex(const DrawVertex& v) {
    vertices_.push_back(v);
}

void DrawList::add_index(u32 index) {
    indices_.push_back(index);
}

void DrawList::ensure_command() {
    if (commands_.empty()) {
        DrawCommand cmd;
        cmd.clip_rect = current_clip_rect();
        commands_.push_back(cmd);
    }
}

void DrawList::push_clip_rect(const Rect& rect) {
    clip_stack_.push_back(rect);
    // Start a new command with the new clip rect
    DrawCommand cmd;
    cmd.clip_rect = rect;
    cmd.vertex_offset = static_cast<u32>(vertices_.size());
    cmd.index_offset = static_cast<u32>(indices_.size());
    commands_.push_back(cmd);
}

void DrawList::pop_clip_rect() {
    if (!clip_stack_.empty()) {
        clip_stack_.pop_back();
        // Start a new command with the restored clip rect
        DrawCommand cmd;
        cmd.clip_rect = current_clip_rect();
        cmd.vertex_offset = static_cast<u32>(vertices_.size());
        cmd.index_offset = static_cast<u32>(indices_.size());
        commands_.push_back(cmd);
    }
}

Rect DrawList::current_clip_rect() const {
    if (clip_stack_.empty()) {
        return Rect{-1e6f, -1e6f, 2e6f, 2e6f};  // Effectively infinite
    }
    return clip_stack_.back();
}

void DrawList::add_rect_filled(const Rect& rect, Color color, f32 corner_radius) {
    ensure_command();

    u32 base = static_cast<u32>(vertices_.size());
    u32 col = color.to_abgr32();

    if (corner_radius <= 0.0f) {
        // Simple rectangle - 4 vertices, 6 indices
        add_vertex(DrawVertex{{rect.x, rect.y}, {0, 0}, col});
        add_vertex(DrawVertex{{rect.x + rect.width, rect.y}, {1, 0}, col});
        add_vertex(DrawVertex{{rect.x + rect.width, rect.y + rect.height}, {1, 1}, col});
        add_vertex(DrawVertex{{rect.x, rect.y + rect.height}, {0, 1}, col});

        add_index(base);
        add_index(base + 1);
        add_index(base + 2);
        add_index(base);
        add_index(base + 2);
        add_index(base + 3);
    } else {
        // Rounded rectangle - create a polygon with arced corners
        const u32 segments_per_corner = 8;
        f32 r = std::min(corner_radius, std::min(rect.width, rect.height) / 2.0f);

        // Center vertex for fan
        Point2D center{rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f};
        u32 center_idx = base;
        add_vertex(DrawVertex{{center.x, center.y}, {0.5f, 0.5f}, col});

        // Generate corner arcs
        auto add_corner = [&](f32 cx, f32 cy, f32 start_angle) {
            for (u32 i = 0; i <= segments_per_corner; ++i) {
                f32 angle = start_angle + (3.14159f / 2.0f) * (static_cast<f32>(i) / segments_per_corner);
                f32 x = cx + r * std::cos(angle);
                f32 y = cy + r * std::sin(angle);
                add_vertex(DrawVertex{{x, y}, {0, 0}, col});
            }
        };

        // Top-left corner
        add_corner(rect.x + r, rect.y + r, 3.14159f);
        // Top-right corner
        add_corner(rect.x + rect.width - r, rect.y + r, -3.14159f / 2.0f);
        // Bottom-right corner
        add_corner(rect.x + rect.width - r, rect.y + rect.height - r, 0.0f);
        // Bottom-left corner
        add_corner(rect.x + r, rect.y + rect.height - r, 3.14159f / 2.0f);

        // Create triangle fan
        u32 num_outer_verts = 4 * (segments_per_corner + 1);
        for (u32 i = 0; i < num_outer_verts; ++i) {
            add_index(center_idx);
            add_index(base + 1 + i);
            add_index(base + 1 + ((i + 1) % num_outer_verts));
        }
    }

    // Update command
    if (!commands_.empty()) {
        commands_.back().index_count = static_cast<u32>(indices_.size()) - commands_.back().index_offset;
    }
}

void DrawList::add_rect_outline(const Rect& rect, Color color, f32 thickness, f32 corner_radius) {
    ensure_command();

    f32 half_t = thickness / 2.0f;

    if (corner_radius <= 0.0f) {
        // Simple rectangle outline - 4 lines
        add_line(Point2D{rect.x, rect.y}, Point2D{rect.x + rect.width, rect.y}, color, thickness);
        add_line(Point2D{rect.x + rect.width, rect.y}, Point2D{rect.x + rect.width, rect.y + rect.height}, color, thickness);
        add_line(Point2D{rect.x + rect.width, rect.y + rect.height}, Point2D{rect.x, rect.y + rect.height}, color, thickness);
        add_line(Point2D{rect.x, rect.y + rect.height}, Point2D{rect.x, rect.y}, color, thickness);
    } else {
        // TODO: Rounded rectangle outline
        add_rect_outline(rect, color, thickness, 0.0f);
    }
}

void DrawList::add_line(Point2D p1, Point2D p2, Color color, f32 thickness) {
    ensure_command();

    u32 base = static_cast<u32>(vertices_.size());
    u32 col = color.to_abgr32();

    // Calculate perpendicular direction
    f32 dx = p2.x - p1.x;
    f32 dy = p2.y - p1.y;
    f32 len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.0001f) return;

    f32 nx = -dy / len * thickness / 2.0f;
    f32 ny = dx / len * thickness / 2.0f;

    add_vertex(DrawVertex{{p1.x + nx, p1.y + ny}, {0, 0}, col});
    add_vertex(DrawVertex{{p1.x - nx, p1.y - ny}, {0, 1}, col});
    add_vertex(DrawVertex{{p2.x - nx, p2.y - ny}, {1, 1}, col});
    add_vertex(DrawVertex{{p2.x + nx, p2.y + ny}, {1, 0}, col});

    add_index(base);
    add_index(base + 1);
    add_index(base + 2);
    add_index(base);
    add_index(base + 2);
    add_index(base + 3);

    if (!commands_.empty()) {
        commands_.back().index_count = static_cast<u32>(indices_.size()) - commands_.back().index_offset;
    }
}

void DrawList::add_circle_filled(Point2D center, f32 radius, Color color, u32 segments) {
    ensure_command();

    u32 base = static_cast<u32>(vertices_.size());
    u32 col = color.to_abgr32();

    // Center vertex
    add_vertex(DrawVertex{{center.x, center.y}, {0.5f, 0.5f}, col});

    // Edge vertices
    for (u32 i = 0; i < segments; ++i) {
        f32 angle = 2.0f * 3.14159f * static_cast<f32>(i) / static_cast<f32>(segments);
        f32 x = center.x + radius * std::cos(angle);
        f32 y = center.y + radius * std::sin(angle);
        add_vertex(DrawVertex{{x, y}, {0, 0}, col});
    }

    // Triangle fan
    for (u32 i = 0; i < segments; ++i) {
        add_index(base);  // Center
        add_index(base + 1 + i);
        add_index(base + 1 + ((i + 1) % segments));
    }

    if (!commands_.empty()) {
        commands_.back().index_count = static_cast<u32>(indices_.size()) - commands_.back().index_offset;
    }
}

void DrawList::add_circle_outline(Point2D center, f32 radius, Color color, f32 thickness, u32 segments) {
    // Draw as connected lines
    for (u32 i = 0; i < segments; ++i) {
        f32 angle1 = 2.0f * 3.14159f * static_cast<f32>(i) / static_cast<f32>(segments);
        f32 angle2 = 2.0f * 3.14159f * static_cast<f32>(i + 1) / static_cast<f32>(segments);

        Point2D p1{center.x + radius * std::cos(angle1), center.y + radius * std::sin(angle1)};
        Point2D p2{center.x + radius * std::cos(angle2), center.y + radius * std::sin(angle2)};

        add_line(p1, p2, color, thickness);
    }
}

void DrawList::add_triangle_filled(Point2D p1, Point2D p2, Point2D p3, Color color) {
    ensure_command();

    u32 base = static_cast<u32>(vertices_.size());
    u32 col = color.to_abgr32();

    add_vertex(DrawVertex{{p1.x, p1.y}, {0, 0}, col});
    add_vertex(DrawVertex{{p2.x, p2.y}, {1, 0}, col});
    add_vertex(DrawVertex{{p3.x, p3.y}, {0.5f, 1}, col});

    add_index(base);
    add_index(base + 1);
    add_index(base + 2);

    if (!commands_.empty()) {
        commands_.back().index_count = static_cast<u32>(indices_.size()) - commands_.back().index_offset;
    }
}

void DrawList::add_text(Point2D pos, StringView text, Color color, f32 size) {
    TextCommand cmd;
    cmd.position = pos;
    cmd.text = String(text);
    cmd.color = color;
    cmd.size = size;
    cmd.clip_rect = current_clip_rect();
    text_commands_.push_back(std::move(cmd));
}

} // namespace frost
