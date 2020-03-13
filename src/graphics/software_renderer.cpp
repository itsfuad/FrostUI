#include "frost/graphics/software_renderer.hpp"
#include <algorithm>
#include <cmath>

namespace frost {

const u8* SoftwareRenderer::render(const DrawList& draw_list, i32 width, i32 height) {
    // Resize buffer if needed
    if (width != width_ || height != height_) {
        width_ = width;
        height_ = height;
        pixels_.resize(width * height * 4);
    }

    // Clear to dark gray background
    clear(Color{0.15f, 0.15f, 0.15f, 1.0f});

    // Process draw commands
    for (const auto& cmd : draw_list.commands()) {
        Rect clip = cmd.clip_rect;
        if (clip.width <= 0 || clip.height <= 0) {
            clip = Rect{0, 0, static_cast<f32>(width), static_cast<f32>(height)};
        }

        // Get vertices and indices for this command
        auto vertices = draw_list.vertices();
        auto indices = draw_list.indices();

        // Process triangles
        for (u32 i = cmd.index_offset; i < cmd.index_offset + cmd.index_count; i += 3) {
            if (i + 2 >= indices.size()) break;

            u32 i0 = indices[i];
            u32 i1 = indices[i + 1];
            u32 i2 = indices[i + 2];

            if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) {
                continue;
            }

            const auto& v0 = vertices[i0];
            const auto& v1 = vertices[i1];
            const auto& v2 = vertices[i2];

            // Simple triangle rasterization using bounding box
            f32 minX = std::min({v0.position.x, v1.position.x, v2.position.x});
            f32 maxX = std::max({v0.position.x, v1.position.x, v2.position.x});
            f32 minY = std::min({v0.position.y, v1.position.y, v2.position.y});
            f32 maxY = std::max({v0.position.y, v1.position.y, v2.position.y});

            // Clamp to clip rect
            i32 startX = std::max(static_cast<i32>(minX), static_cast<i32>(clip.x));
            i32 endX = std::min(static_cast<i32>(maxX) + 1, static_cast<i32>(clip.x + clip.width));
            i32 startY = std::max(static_cast<i32>(minY), static_cast<i32>(clip.y));
            i32 endY = std::min(static_cast<i32>(maxY) + 1, static_cast<i32>(clip.y + clip.height));

            // Clamp to screen
            startX = std::max(0, startX);
            endX = std::min(width_, endX);
            startY = std::max(0, startY);
            endY = std::min(height_, endY);

            // Convert packed color to Color
            u32 packed = v0.color;  // Using first vertex color (flat shading)
            Color color{
                static_cast<f32>((packed >> 0) & 0xFF) / 255.0f,   // R (in ABGR)
                static_cast<f32>((packed >> 8) & 0xFF) / 255.0f,   // G
                static_cast<f32>((packed >> 16) & 0xFF) / 255.0f,  // B
                static_cast<f32>((packed >> 24) & 0xFF) / 255.0f   // A
            };

            // Edge function helper
            auto edge = [](f32 x0, f32 y0, f32 x1, f32 y1, f32 px, f32 py) {
                return (px - x0) * (y1 - y0) - (py - y0) * (x1 - x0);
            };

            f32 area = edge(v0.position.x, v0.position.y,
                           v1.position.x, v1.position.y,
                           v2.position.x, v2.position.y);

            if (std::abs(area) < 0.001f) continue;  // Degenerate triangle

            for (i32 y = startY; y < endY; ++y) {
                for (i32 x = startX; x < endX; ++x) {
                    f32 px = x + 0.5f;
                    f32 py = y + 0.5f;

                    f32 w0 = edge(v1.position.x, v1.position.y,
                                 v2.position.x, v2.position.y, px, py);
                    f32 w1 = edge(v2.position.x, v2.position.y,
                                 v0.position.x, v0.position.y, px, py);
                    f32 w2 = edge(v0.position.x, v0.position.y,
                                 v1.position.x, v1.position.y, px, py);

                    // Check if point is inside triangle (same sign as area)
                    bool inside = (area > 0) ? (w0 >= 0 && w1 >= 0 && w2 >= 0)
                                             : (w0 <= 0 && w1 <= 0 && w2 <= 0);

                    if (inside) {
                        blend_pixel(x, y, color);
                    }
                }
            }
        }
    }

    // Also render text placeholders directly from commands
    // Since text is added via add_text which creates rectangles, this is handled above

    // Render text commands
    for (const auto& text_cmd : draw_list.text_commands()) {
        Rect clip = text_cmd.clip_rect;
        if (clip.width <= 0 || clip.height <= 0) {
            clip = Rect{0, 0, static_cast<f32>(width), static_cast<f32>(height)};
        }
        draw_text(text_cmd.position, text_cmd.text, text_cmd.color, text_cmd.size, clip);
    }

    return pixels_.data();
}

void SoftwareRenderer::clear(Color color) {
    u8 r = static_cast<u8>(color.r * 255);
    u8 g = static_cast<u8>(color.g * 255);
    u8 b = static_cast<u8>(color.b * 255);
    u8 a = static_cast<u8>(color.a * 255);

    for (i32 i = 0; i < width_ * height_; ++i) {
        pixels_[i * 4 + 0] = r;
        pixels_[i * 4 + 1] = g;
        pixels_[i * 4 + 2] = b;
        pixels_[i * 4 + 3] = a;
    }
}

void SoftwareRenderer::set_pixel(i32 x, i32 y, Color color) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return;
    }

    i32 idx = (y * width_ + x) * 4;
    pixels_[idx + 0] = static_cast<u8>(color.r * 255);
    pixels_[idx + 1] = static_cast<u8>(color.g * 255);
    pixels_[idx + 2] = static_cast<u8>(color.b * 255);
    pixels_[idx + 3] = static_cast<u8>(color.a * 255);
}

void SoftwareRenderer::blend_pixel(i32 x, i32 y, Color color) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return;
    }

    i32 idx = (y * width_ + x) * 4;

    // Simple alpha blending
    f32 alpha = color.a;
    f32 inv_alpha = 1.0f - alpha;

    f32 dst_r = pixels_[idx + 0] / 255.0f;
    f32 dst_g = pixels_[idx + 1] / 255.0f;
    f32 dst_b = pixels_[idx + 2] / 255.0f;

    pixels_[idx + 0] = static_cast<u8>((color.r * alpha + dst_r * inv_alpha) * 255);
    pixels_[idx + 1] = static_cast<u8>((color.g * alpha + dst_g * inv_alpha) * 255);
    pixels_[idx + 2] = static_cast<u8>((color.b * alpha + dst_b * inv_alpha) * 255);
    pixels_[idx + 3] = 255;
}

void SoftwareRenderer::draw_rect_filled(const Rect& rect, Color color, const Rect& clip) {
    i32 startX = std::max(static_cast<i32>(rect.x), static_cast<i32>(clip.x));
    i32 endX = std::min(static_cast<i32>(rect.x + rect.width), static_cast<i32>(clip.x + clip.width));
    i32 startY = std::max(static_cast<i32>(rect.y), static_cast<i32>(clip.y));
    i32 endY = std::min(static_cast<i32>(rect.y + rect.height), static_cast<i32>(clip.y + clip.height));

    startX = std::max(0, startX);
    endX = std::min(width_, endX);
    startY = std::max(0, startY);
    endY = std::min(height_, endY);

    for (i32 y = startY; y < endY; ++y) {
        for (i32 x = startX; x < endX; ++x) {
            blend_pixel(x, y, color);
        }
    }
}

void SoftwareRenderer::draw_rect_outline(const Rect& rect, Color color, f32 thickness, const Rect& clip) {
    // Top edge
    draw_rect_filled(Rect{rect.x, rect.y, rect.width, thickness}, color, clip);
    // Bottom edge
    draw_rect_filled(Rect{rect.x, rect.y + rect.height - thickness, rect.width, thickness}, color, clip);
    // Left edge
    draw_rect_filled(Rect{rect.x, rect.y, thickness, rect.height}, color, clip);
    // Right edge
    draw_rect_filled(Rect{rect.x + rect.width - thickness, rect.y, thickness, rect.height}, color, clip);
}

void SoftwareRenderer::draw_text(Point2D pos, StringView text, Color color, f32 size, const Rect& clip) {
    // Calculate scale factor from font size
    f32 scale = size / static_cast<f32>(BitmapFont::GLYPH_HEIGHT);
    f32 char_width = BitmapFont::GLYPH_WIDTH * scale;
    f32 char_height = BitmapFont::GLYPH_HEIGHT * scale;

    f32 x = pos.x;
    f32 y = pos.y;

    for (char c : text) {
        if (c == '\n') {
            x = pos.x;
            y += char_height;
            continue;
        }

        // Render each pixel of the glyph
        for (i32 gy = 0; gy < BitmapFont::GLYPH_HEIGHT; ++gy) {
            for (i32 gx = 0; gx < BitmapFont::GLYPH_WIDTH; ++gx) {
                if (BitmapFont::get_pixel(c, gx, gy)) {
                    // Calculate screen position with scaling
                    i32 sx = static_cast<i32>(x + gx * scale);
                    i32 sy = static_cast<i32>(y + gy * scale);

                    // For larger sizes, fill a scaled pixel
                    i32 w = static_cast<i32>(scale + 0.5f);
                    if (w < 1) w = 1;

                    for (i32 dy = 0; dy < w; ++dy) {
                        for (i32 dx = 0; dx < w; ++dx) {
                            i32 px = sx + dx;
                            i32 py = sy + dy;

                            // Clip check
                            if (px >= clip.x && px < clip.x + clip.width &&
                                py >= clip.y && py < clip.y + clip.height) {
                                blend_pixel(px, py, color);
                            }
                        }
                    }
                }
            }
        }

        x += char_width;
    }
}

} // namespace frost
