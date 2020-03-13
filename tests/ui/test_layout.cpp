#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "frost/ui/layout.hpp"

using namespace frost;
using Catch::Matchers::WithinRel;

// Simple test widget with fixed size
class SizedWidget : public Widget {
public:
    static Unique<SizedWidget> create(f32 width, f32 height) {
        auto w = Unique<SizedWidget>(new SizedWidget());
        w->desired_size_ = Size2D{width, height};
        return w;
    }

    Size2D measure(const LayoutConstraints&) override {
        return desired_size_;
    }

    void render(DrawList&) override {}

    Size2D desired_size_;
};

TEST_CASE("Container basic layout", "[layout][container]") {
    SECTION("column layout") {
        ContainerStyle style;
        style.direction = FlexDirection::Column;
        style.gap = 10.0f;

        auto container = Container::create(style);
        container->add_child(SizedWidget::create(100.0f, 50.0f));
        container->add_child(SizedWidget::create(100.0f, 50.0f));
        container->add_child(SizedWidget::create(100.0f, 50.0f));

        container->layout(Rect{0.0f, 0.0f, 200.0f, 300.0f});

        // First child at top
        REQUIRE(container->children()[0]->bounds().y == 0.0f);
        // Second child after first + gap
        REQUIRE(container->children()[1]->bounds().y == 60.0f);
        // Third child after second + gap
        REQUIRE(container->children()[2]->bounds().y == 120.0f);
    }

    SECTION("row layout") {
        ContainerStyle style;
        style.direction = FlexDirection::Row;
        style.gap = 10.0f;

        auto container = Container::create(style);
        container->add_child(SizedWidget::create(50.0f, 100.0f));
        container->add_child(SizedWidget::create(50.0f, 100.0f));

        container->layout(Rect{0.0f, 0.0f, 200.0f, 100.0f});

        // First child at left
        REQUIRE(container->children()[0]->bounds().x == 0.0f);
        // Second child after first + gap
        REQUIRE(container->children()[1]->bounds().x == 60.0f);
    }
}

TEST_CASE("Container justify content", "[layout][justify]") {
    SECTION("justify start") {
        ContainerStyle style;
        style.direction = FlexDirection::Row;
        style.justify = FlexJustify::Start;

        auto container = Container::create(style);
        container->add_child(SizedWidget::create(50.0f, 50.0f));

        container->layout(Rect{0.0f, 0.0f, 200.0f, 50.0f});

        REQUIRE(container->children()[0]->bounds().x == 0.0f);
    }

    SECTION("justify end") {
        ContainerStyle style;
        style.direction = FlexDirection::Row;
        style.justify = FlexJustify::End;

        auto container = Container::create(style);
        container->add_child(SizedWidget::create(50.0f, 50.0f));

        container->layout(Rect{0.0f, 0.0f, 200.0f, 50.0f});

        REQUIRE(container->children()[0]->bounds().x == 150.0f);
    }

    SECTION("justify center") {
        ContainerStyle style;
        style.direction = FlexDirection::Row;
        style.justify = FlexJustify::Center;

        auto container = Container::create(style);
        container->add_child(SizedWidget::create(50.0f, 50.0f));

        container->layout(Rect{0.0f, 0.0f, 200.0f, 50.0f});

        REQUIRE(container->children()[0]->bounds().x == 75.0f);
    }

    SECTION("justify space-between") {
        ContainerStyle style;
        style.direction = FlexDirection::Row;
        style.justify = FlexJustify::SpaceBetween;

        auto container = Container::create(style);
        container->add_child(SizedWidget::create(50.0f, 50.0f));
        container->add_child(SizedWidget::create(50.0f, 50.0f));

        container->layout(Rect{0.0f, 0.0f, 200.0f, 50.0f});

        REQUIRE(container->children()[0]->bounds().x == 0.0f);
        REQUIRE(container->children()[1]->bounds().x == 150.0f);
    }
}

TEST_CASE("Container align items", "[layout][align]") {
    SECTION("align start") {
        ContainerStyle style;
        style.direction = FlexDirection::Row;
        style.align_items = FlexAlign::Start;

        auto container = Container::create(style);
        container->add_child(SizedWidget::create(50.0f, 30.0f));

        container->layout(Rect{0.0f, 0.0f, 100.0f, 100.0f});

        REQUIRE(container->children()[0]->bounds().y == 0.0f);
    }

    SECTION("align end") {
        ContainerStyle style;
        style.direction = FlexDirection::Row;
        style.align_items = FlexAlign::End;

        auto container = Container::create(style);
        container->add_child(SizedWidget::create(50.0f, 30.0f));

        container->layout(Rect{0.0f, 0.0f, 100.0f, 100.0f});

        REQUIRE(container->children()[0]->bounds().y == 70.0f);
    }

    SECTION("align center") {
        ContainerStyle style;
        style.direction = FlexDirection::Row;
        style.align_items = FlexAlign::Center;

        auto container = Container::create(style);
        container->add_child(SizedWidget::create(50.0f, 30.0f));

        container->layout(Rect{0.0f, 0.0f, 100.0f, 100.0f});

        REQUIRE(container->children()[0]->bounds().y == 35.0f);
    }

    SECTION("align stretch") {
        ContainerStyle style;
        style.direction = FlexDirection::Row;
        style.align_items = FlexAlign::Stretch;

        auto container = Container::create(style);
        container->add_child(SizedWidget::create(50.0f, 30.0f));

        container->layout(Rect{0.0f, 0.0f, 100.0f, 100.0f});

        REQUIRE(container->children()[0]->bounds().height == 100.0f);
    }
}

TEST_CASE("Flex grow/shrink", "[layout][flex]") {
    SECTION("flex grow distributes space") {
        ContainerStyle style;
        style.direction = FlexDirection::Row;

        auto container = Container::create(style);

        auto child1 = SizedWidget::create(50.0f, 50.0f);
        auto child2 = SizedWidget::create(50.0f, 50.0f);

        container->add_child(std::move(child1));
        container->add_child(std::move(child2));

        // Set flex grow on both children
        container->set_child_flex(0, FlexItem{1.0f, 1.0f, -1.0f, FlexAlign::Stretch});
        container->set_child_flex(1, FlexItem{1.0f, 1.0f, -1.0f, FlexAlign::Stretch});

        container->layout(Rect{0.0f, 0.0f, 200.0f, 50.0f});

        // Each should get half of 200px
        REQUIRE(container->children()[0]->bounds().width == 100.0f);
        REQUIRE(container->children()[1]->bounds().width == 100.0f);
    }

    SECTION("unequal flex grow") {
        ContainerStyle style;
        style.direction = FlexDirection::Row;

        auto container = Container::create(style);
        container->add_child(SizedWidget::create(0.0f, 50.0f));
        container->add_child(SizedWidget::create(0.0f, 50.0f));

        // First child grows 1x, second grows 2x
        container->set_child_flex(0, FlexItem{1.0f, 1.0f, -1.0f, FlexAlign::Stretch});
        container->set_child_flex(1, FlexItem{2.0f, 1.0f, -1.0f, FlexAlign::Stretch});

        container->layout(Rect{0.0f, 0.0f, 300.0f, 50.0f});

        // First gets 100px, second gets 200px
        REQUIRE_THAT(container->children()[0]->bounds().width, WithinRel(100.0f, 0.01f));
        REQUIRE_THAT(container->children()[1]->bounds().width, WithinRel(200.0f, 0.01f));
    }
}

TEST_CASE("Convenience functions", "[layout][helpers]") {
    SECTION("HBox creates row container") {
        auto hbox = HBox(5.0f);

        REQUIRE(hbox->style().direction == FlexDirection::Row);
        REQUIRE(hbox->style().gap == 5.0f);
    }

    SECTION("VBox creates column container") {
        auto vbox = VBox(10.0f);

        REQUIRE(vbox->style().direction == FlexDirection::Column);
        REQUIRE(vbox->style().gap == 10.0f);
    }

    SECTION("Center creates centered container") {
        auto center = Center();

        REQUIRE(center->style().direction == FlexDirection::Column);
        REQUIRE(center->style().justify == FlexJustify::Center);
        REQUIRE(center->style().align_items == FlexAlign::Center);
    }
}
