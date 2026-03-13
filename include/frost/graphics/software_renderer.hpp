#pragma once

#include "frost/core/types.hpp"
#include "frost/core/result.hpp"
#include "frost/graphics/draw_list.hpp"
#include "frost/graphics/font.hpp"

namespace frost {

/// Simple software renderer that rasterizes a DrawList to a pixel buffer
class SoftwareRenderer {
public:
    SoftwareRenderer() = default;

    /// Set active font used by text rendering.
    void set_font(Font font);

    /// Reset active font back to the embedded default font.
    void reset_font();

    /// Load and activate a custom font (PSF1/PSF2).
    [[nodiscard]] Result<void> load_font_from_file(StringView file_path, i32 pixel_height = 16);

    [[nodiscard]] const Font& font() const { return font_; }

    /// Render the draw list to the internal pixel buffer
    /// Returns pointer to RGBA8 pixel data
    const u8* render(const DrawList& draw_list, i32 width, i32 height);

    /// Get the pixel buffer (RGBA8 format, width * height * 4 bytes)
    [[nodiscard]] const u8* pixels() const { return pixels_.data(); }

    /// Get buffer dimensions
    [[nodiscard]] i32 width() const { return width_; }
    [[nodiscard]] i32 height() const { return height_; }

private:
    void clear(Color color);
    void draw_rect_filled(const Rect& rect, Color color, const Rect& clip);
    void draw_rect_outline(const Rect& rect, Color color, f32 thickness, const Rect& clip);
    void draw_text(Point2D pos, StringView text, Color color, f32 size, const Rect& clip);

    void set_pixel(i32 x, i32 y, Color color);
    void blend_pixel(i32 x, i32 y, Color color);

    Vector<u8> pixels_;
    i32 width_{0};
    i32 height_{0};
    Font font_{Font::make_default()};
};

} // namespace frost
