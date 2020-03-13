#pragma once

#include "frost/core/types.hpp"

namespace frost {

/// Simple embedded 8x16 bitmap font for ASCII characters 32-127
/// Each character is 8 pixels wide, 16 pixels tall
/// Data is stored as 16 bytes per character (1 bit per pixel, MSB first)
struct BitmapFont {
    static constexpr i32 GLYPH_WIDTH = 8;
    static constexpr i32 GLYPH_HEIGHT = 16;
    static constexpr i32 FIRST_CHAR = 32;   // Space
    static constexpr i32 LAST_CHAR = 126;   // Tilde
    static constexpr i32 CHAR_COUNT = LAST_CHAR - FIRST_CHAR + 1;

    /// Get glyph data for a character (16 bytes, one per row)
    static const u8* get_glyph(char c) {
        if (c < FIRST_CHAR || c > LAST_CHAR) {
            c = '?';  // Fallback for unsupported characters
        }
        return &FONT_DATA[(c - FIRST_CHAR) * GLYPH_HEIGHT];
    }

    /// Check if a pixel is set in the glyph
    static bool get_pixel(char c, i32 x, i32 y) {
        if (x < 0 || x >= GLYPH_WIDTH || y < 0 || y >= GLYPH_HEIGHT) {
            return false;
        }
        const u8* glyph = get_glyph(c);
        return (glyph[y] >> (7 - x)) & 1;
    }

    // Font data: 8x16 bitmap font, 95 printable ASCII characters
    // Generated from a standard VGA-style font
    static const u8 FONT_DATA[];
};

} // namespace frost
