#pragma once

#include "frost/core/types.hpp"
#include "frost/core/math.hpp"
#include "frost/graphics/color.hpp"

namespace frost {

/// Vertex data for rendering
struct DrawVertex {
    Vec2 position;
    Vec2 uv;
    u32 color;  // ABGR packed

    static constexpr DrawVertex make(f32 x, f32 y, f32 u, f32 v, Color c) {
        return DrawVertex{
            Vec2{x, y},
            Vec2{u, v},
            c.to_abgr32()
        };
    }
};

/// A draw command representing a batch of primitives
struct DrawCommand {
    Rect clip_rect;
    u32 vertex_offset{0};
    u32 index_offset{0};
    u32 index_count{0};
    u32 texture_id{0};  // 0 = no texture
};

/// Immediate-mode style draw list for 2D rendering
class DrawList {
public:
    DrawList() = default;

    /// Clear all data
    void clear();

    /// Reserve capacity for vertices and indices
    void reserve(usize vertex_count, usize index_count);

    // ─────────────────────────────────────────────────────────────────────────
    // Primitives
    // ─────────────────────────────────────────────────────────────────────────

    /// Draw a filled rectangle
    void add_rect_filled(const Rect& rect, Color color, f32 corner_radius = 0.0f);

    /// Draw a rectangle outline
    void add_rect_outline(const Rect& rect, Color color, f32 thickness = 1.0f, f32 corner_radius = 0.0f);

    /// Draw a line between two points
    void add_line(Point2D p1, Point2D p2, Color color, f32 thickness = 1.0f);

    /// Draw a filled circle
    void add_circle_filled(Point2D center, f32 radius, Color color, u32 segments = 32);

    /// Draw a circle outline
    void add_circle_outline(Point2D center, f32 radius, Color color, f32 thickness = 1.0f, u32 segments = 32);

    /// Draw a filled triangle
    void add_triangle_filled(Point2D p1, Point2D p2, Point2D p3, Color color);

    /// Draw text (placeholder - actual text rendering requires font atlas)
    void add_text(Point2D pos, StringView text, Color color, f32 size = 14.0f);

    // ─────────────────────────────────────────────────────────────────────────
    // Clipping
    // ─────────────────────────────────────────────────────────────────────────

    /// Push a clip rectangle onto the stack
    void push_clip_rect(const Rect& rect);

    /// Pop the most recent clip rectangle
    void pop_clip_rect();

    /// Get the current clip rectangle
    [[nodiscard]] Rect current_clip_rect() const;

    // ─────────────────────────────────────────────────────────────────────────
    // Data Access
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] Span<const DrawVertex> vertices() const { return vertices_; }
    [[nodiscard]] Span<const u32> indices() const { return indices_; }
    [[nodiscard]] Span<const DrawCommand> commands() const { return commands_; }

    [[nodiscard]] bool empty() const { return vertices_.empty(); }

private:
    void add_vertex(const DrawVertex& v);
    void add_index(u32 index);
    void ensure_command();

    // Helpers for rounded rectangles
    void path_arc(Point2D center, f32 radius, f32 start_angle, f32 end_angle, u32 segments);

    Vector<DrawVertex> vertices_;
    Vector<u32> indices_;
    Vector<DrawCommand> commands_;
    Vector<Rect> clip_stack_;
};

} // namespace frost
