#pragma once

#include "frost/core/types.hpp"
#include "frost/core/result.hpp"

namespace frost {

/// Monospace bitmap font used by the software renderer text pipeline.
class Font {
public:
    Font() = default;

    /// Create font from embedded built-in glyph table.
    [[nodiscard]] static Font make_default();

    /// Load font from file.
    /// Supported formats:
    /// - PSF1/PSF2 bitmap fonts
    /// - TTF/OTF when built with FreeType (FROST_USE_FREETYPE=ON and FreeType found)
    [[nodiscard]] static Result<Font> load_from_file(StringView file_path, i32 pixel_height = 16);

    [[nodiscard]] i32 glyph_width() const { return glyph_width_; }
    [[nodiscard]] i32 glyph_height() const { return glyph_height_; }
    [[nodiscard]] i32 first_char() const { return first_char_; }
    [[nodiscard]] i32 last_char() const { return last_char_; }
    [[nodiscard]] i32 glyph_count() const { return glyph_count_; }

    [[nodiscard]] f32 char_width_for_size(f32 size) const;
    [[nodiscard]] f32 line_height_for_size(f32 size) const;

    [[nodiscard]] bool get_pixel(char c, i32 x, i32 y) const;
    [[nodiscard]] bool valid() const;

    /// Returns true when 8-bit per-pixel alpha data is available (loaded from TTF/OTF).
    /// Use get_alpha() + glyph_advance_x() instead of get_pixel() for this case.
    [[nodiscard]] bool has_alpha_data() const { return !glyph_alpha_data_.empty(); }

    /// Returns 0-255 alpha for a given glyph pixel. Falls back to binary (0 or 255)
    /// for PSF fonts that only have 1-bit data.
    [[nodiscard]] u8 get_alpha(char c, i32 x, i32 y) const;

    /// Per-glyph advance width in pixels. Returns glyph_width_ for fixed-width fonts.
    [[nodiscard]] i32 glyph_advance_x(char c) const;

    /// Pixels from the top of the cell to the text baseline (only meaningful for TTF/OTF).
    [[nodiscard]] i32 ascender() const { return ascender_; }

private:
    i32 glyph_width_{8};
    i32 glyph_height_{16};
    i32 first_char_{32};
    i32 last_char_{126};
    i32 glyph_count_{95};
    i32 bytes_per_glyph_{16};
    i32 ascender_{0};

    Vector<u8> glyph_data_;         ///< 1-bit packed rows (PSF and backward compat)
    Vector<u8> glyph_alpha_data_;   ///< 8-bit per pixel alpha (TTF/OTF via FreeType)
    Vector<i32> glyph_advances_x_;  ///< per-glyph advance width (TTF/OTF only)
};

} // namespace frost
