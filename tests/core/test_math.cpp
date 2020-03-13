#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "frost/core/math.hpp"

using namespace frost;
using Catch::Matchers::WithinRel;

TEST_CASE("Point2D operations", "[math][point]") {
    SECTION("default construction") {
        Point2D p;
        REQUIRE(p.x == 0.0f);
        REQUIRE(p.y == 0.0f);
    }

    SECTION("value construction") {
        Point2D p{3.0f, 4.0f};
        REQUIRE(p.x == 3.0f);
        REQUIRE(p.y == 4.0f);
    }

    SECTION("equality") {
        Point2D a{1.0f, 2.0f};
        Point2D b{1.0f, 2.0f};
        Point2D c{1.0f, 3.0f};

        REQUIRE(a == b);
        REQUIRE(a != c);
    }
}

TEST_CASE("Size2D operations", "[math][size]") {
    SECTION("default construction") {
        Size2D s;
        REQUIRE(s.width == 0.0f);
        REQUIRE(s.height == 0.0f);
    }

    SECTION("value construction") {
        Size2D s{100.0f, 200.0f};
        REQUIRE(s.width == 100.0f);
        REQUIRE(s.height == 200.0f);
    }

    SECTION("area calculation") {
        Size2D s{10.0f, 20.0f};
        REQUIRE(s.area() == 200.0f);
    }

    SECTION("is_empty") {
        Size2D empty1{0.0f, 100.0f};
        Size2D empty2{100.0f, 0.0f};
        Size2D valid{100.0f, 100.0f};

        REQUIRE(empty1.is_empty());
        REQUIRE(empty2.is_empty());
        REQUIRE_FALSE(valid.is_empty());
    }
}

TEST_CASE("Rect operations", "[math][rect]") {
    SECTION("default construction") {
        Rect r;
        REQUIRE(r.x == 0.0f);
        REQUIRE(r.y == 0.0f);
        REQUIRE(r.width == 0.0f);
        REQUIRE(r.height == 0.0f);
    }

    SECTION("value construction") {
        Rect r{10.0f, 20.0f, 100.0f, 50.0f};
        REQUIRE(r.x == 10.0f);
        REQUIRE(r.y == 20.0f);
        REQUIRE(r.width == 100.0f);
        REQUIRE(r.height == 50.0f);
    }

    SECTION("contains point") {
        Rect r{0.0f, 0.0f, 100.0f, 100.0f};

        REQUIRE(r.contains(Point2D{50.0f, 50.0f}));
        REQUIRE(r.contains(Point2D{0.0f, 0.0f}));
        REQUIRE(r.contains(Point2D{99.0f, 99.0f}));
        REQUIRE_FALSE(r.contains(Point2D{100.0f, 50.0f}));
        REQUIRE_FALSE(r.contains(Point2D{-1.0f, 50.0f}));
    }

    SECTION("intersects") {
        Rect r1{0.0f, 0.0f, 100.0f, 100.0f};
        Rect r2{50.0f, 50.0f, 100.0f, 100.0f};
        Rect r3{200.0f, 200.0f, 50.0f, 50.0f};

        REQUIRE(r1.intersects(r2));
        REQUIRE_FALSE(r1.intersects(r3));
    }

    SECTION("intersection") {
        Rect r1{0.0f, 0.0f, 100.0f, 100.0f};
        Rect r2{50.0f, 50.0f, 100.0f, 100.0f};

        auto inter = r1.intersection(r2);
        REQUIRE(inter.x == 50.0f);
        REQUIRE(inter.y == 50.0f);
        REQUIRE(inter.width == 50.0f);
        REQUIRE(inter.height == 50.0f);
    }

    SECTION("united") {
        Rect r1{0.0f, 0.0f, 50.0f, 50.0f};
        Rect r2{100.0f, 100.0f, 50.0f, 50.0f};

        auto u = r1.united(r2);
        REQUIRE(u.x == 0.0f);
        REQUIRE(u.y == 0.0f);
        REQUIRE(u.width == 150.0f);
        REQUIRE(u.height == 150.0f);
    }
}

TEST_CASE("Edges operations", "[math][edges]") {
    SECTION("uniform construction") {
        Edges e{10.0f};
        REQUIRE(e.top == 10.0f);
        REQUIRE(e.right == 10.0f);
        REQUIRE(e.bottom == 10.0f);
        REQUIRE(e.left == 10.0f);
    }

    SECTION("individual values") {
        Edges e{1.0f, 2.0f, 3.0f, 4.0f};  // left, top, right, bottom
        REQUIRE(e.left == 1.0f);
        REQUIRE(e.top == 2.0f);
        REQUIRE(e.right == 3.0f);
        REQUIRE(e.bottom == 4.0f);
    }

    SECTION("horizontal/vertical") {
        Edges e{10.0f, 20.0f, 30.0f, 40.0f};  // left=10, top=20, right=30, bottom=40
        REQUIRE(e.horizontal() == 40.0f);  // left + right = 10 + 30
        REQUIRE(e.vertical() == 60.0f);    // top + bottom = 20 + 40
    }
}
