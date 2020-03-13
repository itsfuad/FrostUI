#include <catch2/catch_test_macros.hpp>
#include "frost/ui/widget.hpp"

using namespace frost;

// Test widget implementation with exposed state manipulation
class TestWidget : public Widget {
public:
    static Unique<TestWidget> create(Size2D size = {100.0f, 50.0f}) {
        auto w = Unique<TestWidget>(new TestWidget());
        w->size_ = size;
        return w;
    }

    Size2D measure(const LayoutConstraints&) override {
        return size_;
    }

    void render(DrawList&) override {
        render_count++;
    }

    // Expose protected methods for testing
    using Widget::set_state;
    using Widget::add_state_flag;
    using Widget::remove_state_flag;

    int render_count = 0;
    Size2D size_;
};

TEST_CASE("Widget basic properties", "[widget]") {
    SECTION("visibility") {
        auto widget = TestWidget::create();

        REQUIRE(widget->is_visible());

        widget->set_visible(false);
        REQUIRE_FALSE(widget->is_visible());

        widget->set_visible(true);
        REQUIRE(widget->is_visible());
    }

    SECTION("enabled state") {
        auto widget = TestWidget::create();

        REQUIRE(widget->is_enabled());

        widget->set_enabled(false);
        REQUIRE_FALSE(widget->is_enabled());
    }

    SECTION("padding") {
        auto widget = TestWidget::create();

        // Edges constructor: left, top, right, bottom
        widget->set_padding(Edges{10.0f, 20.0f, 30.0f, 40.0f});

        auto padding = widget->padding();
        REQUIRE(padding.left == 10.0f);
        REQUIRE(padding.top == 20.0f);
        REQUIRE(padding.right == 30.0f);
        REQUIRE(padding.bottom == 40.0f);
    }
}

TEST_CASE("Widget state flags", "[widget]") {
    SECTION("hovered state") {
        auto widget = TestWidget::create();

        REQUIRE_FALSE(widget->is_hovered());

        widget->add_state_flag(WidgetState::Hovered);
        REQUIRE(widget->is_hovered());

        widget->remove_state_flag(WidgetState::Hovered);
        REQUIRE_FALSE(widget->is_hovered());
    }

    SECTION("pressed state") {
        auto widget = TestWidget::create();

        REQUIRE_FALSE(widget->is_pressed());

        widget->add_state_flag(WidgetState::Pressed);
        REQUIRE(widget->is_pressed());
    }

    SECTION("focused state") {
        auto widget = TestWidget::create();

        REQUIRE_FALSE(widget->is_focused());

        widget->add_state_flag(WidgetState::Focused);
        REQUIRE(widget->is_focused());
    }

    SECTION("multiple states") {
        auto widget = TestWidget::create();

        widget->add_state_flag(WidgetState::Hovered);
        widget->add_state_flag(WidgetState::Focused);

        REQUIRE(widget->is_hovered());
        REQUIRE(widget->is_focused());

        widget->remove_state_flag(WidgetState::Hovered);
        REQUIRE_FALSE(widget->is_hovered());
        REQUIRE(widget->is_focused());
    }
}

TEST_CASE("Widget hierarchy", "[widget]") {
    SECTION("add child") {
        auto parent = TestWidget::create();
        auto child = TestWidget::create();
        auto* child_ptr = child.get();

        parent->add_child(std::move(child));

        REQUIRE(parent->children().size() == 1);
        REQUIRE(parent->children()[0].get() == child_ptr);
        REQUIRE(child_ptr->parent() == parent.get());
    }

    SECTION("remove child") {
        auto parent = TestWidget::create();
        auto child = TestWidget::create();
        auto* child_ptr = child.get();

        parent->add_child(std::move(child));
        REQUIRE(parent->children().size() == 1);

        auto removed = parent->remove_child(child_ptr);
        REQUIRE(parent->children().empty());
        REQUIRE(removed.get() == child_ptr);
        REQUIRE(child_ptr->parent() == nullptr);
    }

    SECTION("child bounds") {
        auto parent = TestWidget::create({200.0f, 200.0f});
        parent->layout(Rect{0.0f, 0.0f, 200.0f, 200.0f});

        REQUIRE(parent->bounds().width == 200.0f);
        REQUIRE(parent->bounds().height == 200.0f);
    }
}

TEST_CASE("Widget hit testing", "[widget]") {
    SECTION("point inside returns true") {
        auto widget = TestWidget::create({100.0f, 100.0f});
        widget->layout(Rect{0.0f, 0.0f, 100.0f, 100.0f});

        REQUIRE(widget->hit_test(Point2D{50.0f, 50.0f}));
    }

    SECTION("point outside returns false") {
        auto widget = TestWidget::create({100.0f, 100.0f});
        widget->layout(Rect{0.0f, 0.0f, 100.0f, 100.0f});

        REQUIRE_FALSE(widget->hit_test(Point2D{150.0f, 50.0f}));
    }

    SECTION("invisible widget not hit") {
        auto widget = TestWidget::create({100.0f, 100.0f});
        widget->layout(Rect{0.0f, 0.0f, 100.0f, 100.0f});
        widget->set_visible(false);

        REQUIRE_FALSE(widget->hit_test(Point2D{50.0f, 50.0f}));
    }
}
