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

    /// Load PSF1/PSF2 bitmap font from file.
    [[nodiscard]] static Result<Font> load_from_file(StringView file_path);

    [[nodiscard]] i32 glyph_width() const { return glyph_width_; }
    [[nodiscard]] i32 glyph_height() const { return glyph_height_; }
    [[nodiscard]] i32 first_char() const { return first_char_; }
    [[nodiscard]] i32 last_char() const { return last_char_; }
    [[nodiscard]] i32 glyph_count() const { return glyph_count_; }

    [[nodiscard]] f32 char_width_for_size(f32 size) const;
    [[nodiscard]] f32 line_height_for_size(f32 size) const;

    [[nodiscard]] bool get_pixel(char c, i32 x, i32 y) const;
    [[nodiscard]] bool valid() const;

private:
    i32 glyph_width_{8};
    i32 glyph_height_{16};
    i32 first_char_{32};
    i32 last_char_{126};
    i32 glyph_count_{95};
    i32 bytes_per_glyph_{16};

    Vector<u8> glyph_data_;
};

} // namespace frost
