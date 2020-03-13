#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// FrostUI Color Types
// ─────────────────────────────────────────────────────────────────────────────

#include "frost/core/types.hpp"
#include "frost/core/math.hpp"
#include <cmath>
#include <algorithm>

namespace frost {

// ─────────────────────────────────────────────────────────────────────────────
// Color (RGBA float)
// ─────────────────────────────────────────────────────────────────────────────

struct Color {
    f32 r{0.0f};
    f32 g{0.0f};
    f32 b{0.0f};
    f32 a{1.0f};

    constexpr Color() = default;
    constexpr Color(f32 r_, f32 g_, f32 b_, f32 a_ = 1.0f)
        : r(r_), g(g_), b(b_), a(a_) {}

    // ─────────────────────────────────────────────────────────────────────────
    // Factory Methods
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] static constexpr Color from_rgb(u8 r, u8 g, u8 b) {
        return {r / 255.0f, g / 255.0f, b / 255.0f, 1.0f};
    }

    [[nodiscard]] static constexpr Color from_rgba(u8 r, u8 g, u8 b, u8 a) {
        return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
    }

    [[nodiscard]] static constexpr Color from_hex(u32 hex) {
        return {
            ((hex >> 16) & 0xFF) / 255.0f,
            ((hex >> 8) & 0xFF) / 255.0f,
            (hex & 0xFF) / 255.0f,
            1.0f
        };
    }

    [[nodiscard]] static constexpr Color from_hex_alpha(u32 hex) {
        return {
            ((hex >> 24) & 0xFF) / 255.0f,
            ((hex >> 16) & 0xFF) / 255.0f,
            ((hex >> 8) & 0xFF) / 255.0f,
            (hex & 0xFF) / 255.0f
        };
    }

    [[nodiscard]] static Color from_hsv(f32 h, f32 s, f32 v, f32 a = 1.0f) {
        if (s <= 0.0f) {
            return {v, v, v, a};
        }

        h = std::fmod(h, 360.0f);
        if (h < 0.0f) h += 360.0f;
        h /= 60.0f;

        int i = static_cast<int>(h);
        f32 f = h - static_cast<f32>(i);
        f32 p = v * (1.0f - s);
        f32 q = v * (1.0f - s * f);
        f32 t = v * (1.0f - s * (1.0f - f));

        switch (i) {
            case 0: return {v, t, p, a};
            case 1: return {q, v, p, a};
            case 2: return {p, v, t, a};
            case 3: return {p, q, v, a};
            case 4: return {t, p, v, a};
            default: return {v, p, q, a};
        }
    }

    [[nodiscard]] static Color from_hsl(f32 h, f32 s, f32 l, f32 a = 1.0f) {
        if (s <= 0.0f) {
            return {l, l, l, a};
        }

        auto hue_to_rgb = [](f32 p, f32 q, f32 t) {
            if (t < 0.0f) t += 1.0f;
            if (t > 1.0f) t -= 1.0f;
            if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
            if (t < 1.0f / 2.0f) return q;
            if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
            return p;
        };

        f32 q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        f32 p = 2.0f * l - q;
        f32 h_norm = h / 360.0f;

        return {
            hue_to_rgb(p, q, h_norm + 1.0f / 3.0f),
            hue_to_rgb(p, q, h_norm),
            hue_to_rgb(p, q, h_norm - 1.0f / 3.0f),
            a
        };
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Conversions
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] constexpr u32 to_rgba32() const {
        u8 rb = static_cast<u8>(clamp(r, 0.0f, 1.0f) * 255.0f);
        u8 gb = static_cast<u8>(clamp(g, 0.0f, 1.0f) * 255.0f);
        u8 bb = static_cast<u8>(clamp(b, 0.0f, 1.0f) * 255.0f);
        u8 ab = static_cast<u8>(clamp(a, 0.0f, 1.0f) * 255.0f);
        return (rb << 24) | (gb << 16) | (bb << 8) | ab;
    }

    [[nodiscard]] constexpr u32 to_abgr32() const {
        u8 rb = static_cast<u8>(clamp(r, 0.0f, 1.0f) * 255.0f);
        u8 gb = static_cast<u8>(clamp(g, 0.0f, 1.0f) * 255.0f);
        u8 bb = static_cast<u8>(clamp(b, 0.0f, 1.0f) * 255.0f);
        u8 ab = static_cast<u8>(clamp(a, 0.0f, 1.0f) * 255.0f);
        return (ab << 24) | (bb << 16) | (gb << 8) | rb;
    }

    [[nodiscard]] constexpr Vec4 to_vec4() const {
        return {r, g, b, a};
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Modifiers
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] constexpr Color with_alpha(f32 new_alpha) const {
        return {r, g, b, new_alpha};
    }

    [[nodiscard]] constexpr Color multiplied_alpha(f32 factor) const {
        return {r, g, b, a * factor};
    }

    [[nodiscard]] constexpr Color premultiplied() const {
        return {r * a, g * a, b * a, a};
    }

    [[nodiscard]] Color lightened(f32 amount) const {
        return {
            std::min(r + amount, 1.0f),
            std::min(g + amount, 1.0f),
            std::min(b + amount, 1.0f),
            a
        };
    }

    [[nodiscard]] Color darkened(f32 amount) const {
        return {
            std::max(r - amount, 0.0f),
            std::max(g - amount, 0.0f),
            std::max(b - amount, 0.0f),
            a
        };
    }

    [[nodiscard]] f32 luminance() const {
        return 0.2126f * r + 0.7152f * g + 0.0722f * b;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Blending
    // ─────────────────────────────────────────────────────────────────────────

    [[nodiscard]] static Color lerp(const Color& a, const Color& b, f32 t) {
        return {
            frost::lerp(a.r, b.r, t),
            frost::lerp(a.g, b.g, t),
            frost::lerp(a.b, b.b, t),
            frost::lerp(a.a, b.a, t)
        };
    }

    [[nodiscard]] Color blended_over(const Color& background) const {
        f32 out_a = a + background.a * (1.0f - a);
        if (out_a <= 0.0f) {
            return Color::transparent();
        }
        return {
            (r * a + background.r * background.a * (1.0f - a)) / out_a,
            (g * a + background.g * background.a * (1.0f - a)) / out_a,
            (b * a + background.b * background.a * (1.0f - a)) / out_a,
            out_a
        };
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Operators
    // ─────────────────────────────────────────────────────────────────────────

    constexpr bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    constexpr bool operator!=(const Color& other) const {
        return !(*this == other);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Predefined Colors
    // ─────────────────────────────────────────────────────────────────────────

    static constexpr Color transparent() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
    static constexpr Color black() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr Color white() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr Color red() { return {1.0f, 0.0f, 0.0f, 1.0f}; }
    static constexpr Color green() { return {0.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr Color blue() { return {0.0f, 0.0f, 1.0f, 1.0f}; }
    static constexpr Color yellow() { return {1.0f, 1.0f, 0.0f, 1.0f}; }
    static constexpr Color cyan() { return {0.0f, 1.0f, 1.0f, 1.0f}; }
    static constexpr Color magenta() { return {1.0f, 0.0f, 1.0f, 1.0f}; }
    static constexpr Color orange() { return {1.0f, 0.647f, 0.0f, 1.0f}; }
    static constexpr Color purple() { return {0.5f, 0.0f, 0.5f, 1.0f}; }
    static constexpr Color gray() { return {0.5f, 0.5f, 0.5f, 1.0f}; }
    static constexpr Color light_gray() { return {0.75f, 0.75f, 0.75f, 1.0f}; }
    static constexpr Color dark_gray() { return {0.25f, 0.25f, 0.25f, 1.0f}; }

    // Modern UI colors
    static constexpr Color bg_dark() { return from_hex(0x1E1E1E); }
    static constexpr Color bg_light() { return from_hex(0xF5F5F5); }
    static constexpr Color accent() { return from_hex(0x0078D4); }
    static constexpr Color success() { return from_hex(0x107C10); }
    static constexpr Color warning() { return from_hex(0xFFB900); }
    static constexpr Color error_color() { return from_hex(0xD83B01); }
};

// ─────────────────────────────────────────────────────────────────────────────
// Gradient Types
// ─────────────────────────────────────────────────────────────────────────────

struct GradientStop {
    f32 position{0.0f}; // 0.0 to 1.0
    Color color;

    constexpr GradientStop() = default;
    constexpr GradientStop(f32 pos, Color c) : position(pos), color(c) {}
};

struct LinearGradient {
    Point2D start;
    Point2D end;
    Vector<GradientStop> stops;

    LinearGradient() = default;
    LinearGradient(Point2D s, Point2D e, Vector<GradientStop> st)
        : start(s), end(e), stops(std::move(st)) {}

    static LinearGradient horizontal(const Rect& rect, Color start_color, Color end_color) {
        return {
            rect.top_left(),
            rect.top_right(),
            {{0.0f, start_color}, {1.0f, end_color}}
        };
    }

    static LinearGradient vertical(const Rect& rect, Color start_color, Color end_color) {
        return {
            rect.top_left(),
            rect.bottom_left(),
            {{0.0f, start_color}, {1.0f, end_color}}
        };
    }
};

struct RadialGradient {
    Point2D center;
    f32 radius{0.0f};
    Vector<GradientStop> stops;

    RadialGradient() = default;
    RadialGradient(Point2D c, f32 r, Vector<GradientStop> st)
        : center(c), radius(r), stops(std::move(st)) {}
};

} // namespace frost
