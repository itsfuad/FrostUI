#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// FrostUI Math Types
// ─────────────────────────────────────────────────────────────────────────────

#include "frost/core/types.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace frost {

// ─────────────────────────────────────────────────────────────────────────────
// Vector Types
// ─────────────────────────────────────────────────────────────────────────────

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

using IVec2 = glm::ivec2;
using IVec3 = glm::ivec3;
using IVec4 = glm::ivec4;

using UVec2 = glm::uvec2;
using UVec3 = glm::uvec3;
using UVec4 = glm::uvec4;

using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

// ─────────────────────────────────────────────────────────────────────────────
// Point2D
// ─────────────────────────────────────────────────────────────────────────────

struct Point2D {
    f32 x{0.0f};
    f32 y{0.0f};

    constexpr Point2D() = default;
    constexpr Point2D(f32 x_, f32 y_) : x(x_), y(y_) {}
    constexpr explicit Point2D(const Vec2& v) : x(v.x), y(v.y) {}

    [[nodiscard]] constexpr Vec2 to_vec2() const { return {x, y}; }

    constexpr Point2D operator+(const Point2D& other) const {
        return {x + other.x, y + other.y};
    }

    constexpr Point2D operator-(const Point2D& other) const {
        return {x - other.x, y - other.y};
    }

    constexpr Point2D operator*(f32 scalar) const {
        return {x * scalar, y * scalar};
    }

    constexpr Point2D& operator+=(const Point2D& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr bool operator==(const Point2D& other) const {
        return x == other.x && y == other.y;
    }

    [[nodiscard]] f32 distance_to(const Point2D& other) const {
        f32 dx = other.x - x;
        f32 dy = other.y - y;
        return std::sqrt(dx * dx + dy * dy);
    }

    [[nodiscard]] constexpr f32 distance_squared_to(const Point2D& other) const {
        f32 dx = other.x - x;
        f32 dy = other.y - y;
        return dx * dx + dy * dy;
    }

    static constexpr Point2D zero() { return {0.0f, 0.0f}; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Size2D
// ─────────────────────────────────────────────────────────────────────────────

struct Size2D {
    f32 width{0.0f};
    f32 height{0.0f};

    constexpr Size2D() = default;
    constexpr Size2D(f32 w, f32 h) : width(w), height(h) {}
    constexpr explicit Size2D(f32 uniform) : width(uniform), height(uniform) {}

    [[nodiscard]] constexpr Vec2 to_vec2() const { return {width, height}; }

    [[nodiscard]] constexpr f32 area() const { return width * height; }
    [[nodiscard]] constexpr f32 aspect_ratio() const {
        return height > 0.0f ? width / height : 0.0f;
    }

    [[nodiscard]] constexpr bool is_empty() const {
        return width <= 0.0f || height <= 0.0f;
    }

    [[nodiscard]] constexpr bool contains(const Point2D& point) const {
        return point.x >= 0.0f && point.x < width &&
               point.y >= 0.0f && point.y < height;
    }

    constexpr Size2D operator*(f32 scalar) const {
        return {width * scalar, height * scalar};
    }

    constexpr bool operator==(const Size2D& other) const {
        return width == other.width && height == other.height;
    }

    static constexpr Size2D zero() { return {0.0f, 0.0f}; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Rect
// ─────────────────────────────────────────────────────────────────────────────

struct Rect {
    f32 x{0.0f};
    f32 y{0.0f};
    f32 width{0.0f};
    f32 height{0.0f};

    constexpr Rect() = default;
    constexpr Rect(f32 x_, f32 y_, f32 w, f32 h)
        : x(x_), y(y_), width(w), height(h) {}
    constexpr Rect(Point2D origin, Size2D size)
        : x(origin.x), y(origin.y), width(size.width), height(size.height) {}

    // Accessors
    [[nodiscard]] constexpr f32 left() const { return x; }
    [[nodiscard]] constexpr f32 top() const { return y; }
    [[nodiscard]] constexpr f32 right() const { return x + width; }
    [[nodiscard]] constexpr f32 bottom() const { return y + height; }

    [[nodiscard]] constexpr Point2D origin() const { return {x, y}; }
    [[nodiscard]] constexpr Size2D size() const { return {width, height}; }

    [[nodiscard]] constexpr Point2D top_left() const { return {x, y}; }
    [[nodiscard]] constexpr Point2D top_right() const { return {right(), y}; }
    [[nodiscard]] constexpr Point2D bottom_left() const { return {x, bottom()}; }
    [[nodiscard]] constexpr Point2D bottom_right() const { return {right(), bottom()}; }
    [[nodiscard]] constexpr Point2D center() const {
        return {x + width * 0.5f, y + height * 0.5f};
    }

    // Operations
    [[nodiscard]] constexpr bool is_empty() const {
        return width <= 0.0f || height <= 0.0f;
    }

    [[nodiscard]] constexpr bool contains(const Point2D& point) const {
        return point.x >= x && point.x < right() &&
               point.y >= y && point.y < bottom();
    }

    [[nodiscard]] constexpr bool contains(f32 px, f32 py) const {
        return contains(Point2D{px, py});
    }

    [[nodiscard]] constexpr bool intersects(const Rect& other) const {
        return !(other.x >= right() || other.right() <= x ||
                 other.y >= bottom() || other.bottom() <= y);
    }

    [[nodiscard]] constexpr Rect intersection(const Rect& other) const {
        f32 ix = std::max(x, other.x);
        f32 iy = std::max(y, other.y);
        f32 iw = std::min(right(), other.right()) - ix;
        f32 ih = std::min(bottom(), other.bottom()) - iy;

        if (iw <= 0.0f || ih <= 0.0f) {
            return Rect{};
        }
        return {ix, iy, iw, ih};
    }

    [[nodiscard]] constexpr Rect united(const Rect& other) const {
        if (is_empty()) return other;
        if (other.is_empty()) return *this;

        f32 ux = std::min(x, other.x);
        f32 uy = std::min(y, other.y);
        f32 ur = std::max(right(), other.right());
        f32 ub = std::max(bottom(), other.bottom());

        return {ux, uy, ur - ux, ub - uy};
    }

    [[nodiscard]] constexpr Rect translated(f32 dx, f32 dy) const {
        return {x + dx, y + dy, width, height};
    }

    [[nodiscard]] constexpr Rect translated(const Point2D& delta) const {
        return translated(delta.x, delta.y);
    }

    [[nodiscard]] constexpr Rect inset(f32 amount) const {
        return {x + amount, y + amount,
                width - amount * 2.0f, height - amount * 2.0f};
    }

    [[nodiscard]] constexpr Rect inset(f32 horizontal, f32 vertical) const {
        return {x + horizontal, y + vertical,
                width - horizontal * 2.0f, height - vertical * 2.0f};
    }

    [[nodiscard]] constexpr Rect inset(f32 l, f32 t, f32 r, f32 b) const {
        return {x + l, y + t, width - l - r, height - t - b};
    }

    [[nodiscard]] constexpr Rect expanded(f32 amount) const {
        return inset(-amount);
    }

    constexpr bool operator==(const Rect& other) const {
        return x == other.x && y == other.y &&
               width == other.width && height == other.height;
    }

    // Factories
    static constexpr Rect from_ltrb(f32 left, f32 top, f32 right, f32 bottom) {
        return {left, top, right - left, bottom - top};
    }

    static constexpr Rect from_center(Point2D center, Size2D size) {
        return {center.x - size.width * 0.5f, center.y - size.height * 0.5f,
                size.width, size.height};
    }

    static constexpr Rect zero() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
};

// ─────────────────────────────────────────────────────────────────────────────
// RoundedRect
// ─────────────────────────────────────────────────────────────────────────────

struct CornerRadii {
    f32 top_left{0.0f};
    f32 top_right{0.0f};
    f32 bottom_left{0.0f};
    f32 bottom_right{0.0f};

    constexpr CornerRadii() = default;
    constexpr explicit CornerRadii(f32 uniform)
        : top_left(uniform), top_right(uniform),
          bottom_left(uniform), bottom_right(uniform) {}
    constexpr CornerRadii(f32 tl, f32 tr, f32 bl, f32 br)
        : top_left(tl), top_right(tr), bottom_left(bl), bottom_right(br) {}

    [[nodiscard]] constexpr bool is_uniform() const {
        return top_left == top_right && top_left == bottom_left && top_left == bottom_right;
    }

    [[nodiscard]] constexpr bool is_zero() const {
        return top_left == 0.0f && top_right == 0.0f &&
               bottom_left == 0.0f && bottom_right == 0.0f;
    }

    [[nodiscard]] constexpr f32 max() const {
        return std::max({top_left, top_right, bottom_left, bottom_right});
    }
};

struct RoundedRect {
    Rect rect;
    CornerRadii radii;

    constexpr RoundedRect() = default;
    constexpr RoundedRect(Rect r, f32 radius)
        : rect(r), radii(radius) {}
    constexpr RoundedRect(Rect r, CornerRadii cr)
        : rect(r), radii(cr) {}
};

// ─────────────────────────────────────────────────────────────────────────────
// Edges (for margins, padding, borders)
// ─────────────────────────────────────────────────────────────────────────────

struct Edges {
    f32 left{0.0f};
    f32 top{0.0f};
    f32 right{0.0f};
    f32 bottom{0.0f};

    constexpr Edges() = default;
    constexpr explicit Edges(f32 uniform)
        : left(uniform), top(uniform), right(uniform), bottom(uniform) {}
    constexpr Edges(f32 horizontal, f32 vertical)
        : left(horizontal), top(vertical), right(horizontal), bottom(vertical) {}
    constexpr Edges(f32 l, f32 t, f32 r, f32 b)
        : left(l), top(t), right(r), bottom(b) {}

    [[nodiscard]] constexpr f32 horizontal() const { return left + right; }
    [[nodiscard]] constexpr f32 vertical() const { return top + bottom; }
    [[nodiscard]] constexpr Size2D total() const { return {horizontal(), vertical()}; }

    [[nodiscard]] constexpr bool is_zero() const {
        return left == 0.0f && top == 0.0f && right == 0.0f && bottom == 0.0f;
    }

    constexpr Edges operator+(const Edges& other) const {
        return {left + other.left, top + other.top,
                right + other.right, bottom + other.bottom};
    }

    constexpr bool operator==(const Edges& other) const {
        return left == other.left && top == other.top &&
               right == other.right && bottom == other.bottom;
    }

    static constexpr Edges zero() { return {0.0f, 0.0f, 0.0f, 0.0f}; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Transform2D
// ─────────────────────────────────────────────────────────────────────────────

struct Transform2D {
    Mat3 matrix{1.0f};

    Transform2D() = default;
    explicit Transform2D(const Mat3& m) : matrix(m) {}

    [[nodiscard]] static Transform2D identity() { return {}; }

    [[nodiscard]] static Transform2D translation(f32 x, f32 y) {
        Transform2D t;
        t.matrix = glm::translate(Mat4(1.0f), Vec3(x, y, 0.0f));
        return t;
    }

    [[nodiscard]] static Transform2D scale(f32 sx, f32 sy) {
        Transform2D t;
        t.matrix = glm::scale(Mat4(1.0f), Vec3(sx, sy, 1.0f));
        return t;
    }

    [[nodiscard]] static Transform2D rotation(f32 radians) {
        Transform2D t;
        t.matrix = glm::rotate(Mat4(1.0f), radians, Vec3(0.0f, 0.0f, 1.0f));
        return t;
    }

    [[nodiscard]] Point2D transform_point(const Point2D& p) const {
        Vec3 result = matrix * Vec3(p.x, p.y, 1.0f);
        return {result.x, result.y};
    }

    [[nodiscard]] Transform2D then(const Transform2D& other) const {
        return Transform2D(other.matrix * matrix);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Utility Functions
// ─────────────────────────────────────────────────────────────────────────────

template<typename T>
[[nodiscard]] constexpr T lerp(T a, T b, f32 t) {
    return a + (b - a) * t;
}

template<typename T>
[[nodiscard]] constexpr T clamp(T value, T min_val, T max_val) {
    return std::max(min_val, std::min(max_val, value));
}

[[nodiscard]] inline f32 smoothstep(f32 edge0, f32 edge1, f32 x) {
    f32 t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

} // namespace frost
