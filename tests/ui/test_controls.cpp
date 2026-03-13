#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "frost/ui/widgets/checkbox.hpp"
#include "frost/ui/widgets/radio_button.hpp"
#include "frost/ui/widgets/slider.hpp"

using namespace frost;
using Catch::Matchers::WithinRel;

TEST_CASE("Checkbox toggle behavior", "[ui][checkbox]") {
    auto checkbox = Checkbox::create("Enable feature");

    SECTION("starts unchecked") {
        REQUIRE_FALSE(checkbox->checked());
    }

    SECTION("set_checked emits changes") {
        bool latest = false;
        usize count = 0;

        checkbox->on_toggled.connect([&latest, &count](bool checked) {
            latest = checked;
            count++;
        });

        checkbox->set_checked(true);
        REQUIRE(checkbox->checked());
        REQUIRE(latest);
        REQUIRE(count == 1);
    }

    SECTION("mouse release toggles when hovered") {
        checkbox->on_mouse_enter();
        checkbox->on_mouse_button(MouseButton::Left, KeyAction::Release, ModifierFlags::None);
        REQUIRE(checkbox->checked());
    }
}

TEST_CASE("RadioButton group exclusivity", "[ui][radio]") {
    auto first = RadioButton::create("Option A", "group-main");
    auto second = RadioButton::create("Option B", "group-main");

    SECTION("selecting one deselects the other") {
        first->set_selected(true);
        REQUIRE(first->selected());
        REQUIRE_FALSE(second->selected());

        second->set_selected(true);
        REQUIRE_FALSE(first->selected());
        REQUIRE(second->selected());
    }

    SECTION("click select") {
        second->on_mouse_enter();
        second->on_mouse_button(MouseButton::Left, KeyAction::Release, ModifierFlags::None);
        REQUIRE(second->selected());
    }
}

TEST_CASE("Slider value and range behavior", "[ui][slider]") {
    auto slider = Slider::create(0.0f, 100.0f, 25.0f);
    slider->layout(Rect{0.0f, 0.0f, 200.0f, 40.0f});

    SECTION("initial value is clamped in range") {
        REQUIRE_THAT(slider->value(), WithinRel(25.0f, 0.001f));
    }

    SECTION("keyboard adjusts value") {
        slider->set_step(5.0f);
        slider->on_key(KeyCode::Right, KeyAction::Press, ModifierFlags::None);
        REQUIRE_THAT(slider->value(), WithinRel(30.0f, 0.001f));

        slider->on_key(KeyCode::Left, KeyAction::Press, ModifierFlags::None);
        REQUIRE_THAT(slider->value(), WithinRel(25.0f, 0.001f));
    }

    SECTION("mouse drag updates value") {
        slider->on_mouse_enter();
        slider->on_mouse_move(100.0, 20.0);
        slider->on_mouse_button(MouseButton::Left, KeyAction::Press, ModifierFlags::None);
        slider->on_mouse_move(190.0, 20.0);
        slider->on_mouse_button(MouseButton::Left, KeyAction::Release, ModifierFlags::None);

        REQUIRE(slider->value() > 85.0f);
    }

    SECTION("click updates to clicked position") {
        slider->on_mouse_enter();
        slider->on_mouse_move(190.0, 20.0);
        slider->on_mouse_button(MouseButton::Left, KeyAction::Press, ModifierFlags::None);
        slider->on_mouse_button(MouseButton::Left, KeyAction::Release, ModifierFlags::None);

        REQUIRE(slider->value() > 85.0f);
    }

    SECTION("hover move without click does not update value") {
        slider->on_mouse_enter();
        slider->on_mouse_move(190.0, 20.0);
        REQUIRE_THAT(slider->value(), WithinRel(25.0f, 0.001f));
    }

    SECTION("range update clamps value") {
        slider->set_range(0.0f, 20.0f);
        REQUIRE_THAT(slider->value(), WithinRel(20.0f, 0.001f));
    }
}
