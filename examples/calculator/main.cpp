#include <frost/frost.hpp>

#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>

using namespace frost;

namespace {

struct SurfaceStyle {
    Color fill;
    Color border;
    f32 radius{0.0f};
    f32 border_width{1.0f};
};

class Surface : public Container {
public:
    static Unique<Surface> create(const ContainerStyle& layout, const SurfaceStyle& style) {
        return Unique<Surface>(new Surface(layout, style));
    }

    void render(DrawList& draw_list) override {
        draw_list.add_rect_filled(bounds_, style_.fill, style_.radius);
        if (style_.border_width > 0.0f) {
            draw_list.add_rect_outline(bounds_, style_.border, style_.border_width, style_.radius);
        }
        Container::render(draw_list);
    }

private:
    Surface(const ContainerStyle& layout, const SurfaceStyle& style)
        : Container(layout)
        , style_(style) {}

    SurfaceStyle style_;
};

std::string format_number(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(8) << value;

    auto text = stream.str();
    while (!text.empty() && text.back() == '0') {
        text.pop_back();
    }
    if (!text.empty() && text.back() == '.') {
        text.pop_back();
    }
    if (text.empty() || text == "-0") {
        text = "0";
    }
    return text;
}

std::optional<double> parse_number(const String& text) {
    if (text.empty() || text == "-") {
        return std::nullopt;
    }

    try {
        size_t parsed = 0;
        double value = std::stod(text, &parsed);
        if (parsed != text.size()) {
            return std::nullopt;
        }
        return value;
    } catch (...) {
        return std::nullopt;
    }
}

ButtonStyle digit_button_style() {
    ButtonStyle style;
    style.background_color = Color{0.16f, 0.19f, 0.24f, 1.0f};
    style.hover_color = Color{0.21f, 0.25f, 0.31f, 1.0f};
    style.pressed_color = Color{0.11f, 0.14f, 0.18f, 1.0f};
    style.font_size = 22.0f;
    style.corner_radius = 12.0f;
    style.padding = Edges{16.0f, 16.0f, 16.0f, 16.0f};
    return style;
}

ButtonStyle operator_button_style() {
    ButtonStyle style;
    style.background_color = Color{0.88f, 0.56f, 0.18f, 1.0f};
    style.hover_color = Color{0.93f, 0.62f, 0.24f, 1.0f};
    style.pressed_color = Color{0.74f, 0.44f, 0.11f, 1.0f};
    style.font_size = 22.0f;
    style.corner_radius = 12.0f;
    style.padding = Edges{16.0f, 16.0f, 16.0f, 16.0f};
    return style;
}

ButtonStyle utility_button_style() {
    ButtonStyle style;
    style.background_color = Color{0.28f, 0.22f, 0.25f, 1.0f};
    style.hover_color = Color{0.37f, 0.29f, 0.33f, 1.0f};
    style.pressed_color = Color{0.21f, 0.16f, 0.19f, 1.0f};
    style.font_size = 18.0f;
    style.corner_radius = 12.0f;
    style.padding = Edges{16.0f, 16.0f, 16.0f, 16.0f};
    return style;
}

LabelStyle display_style() {
    LabelStyle style;
    style.font_size = 34.0f;
    style.text_align = TextAlign::Right;
    style.vertical_align = VerticalAlign::Middle;
    style.text_color = Color{0.97f, 0.98f, 1.0f, 1.0f};
    return style;
}

LabelStyle status_style() {
    LabelStyle style;
    style.font_size = 13.0f;
    style.text_align = TextAlign::Left;
    style.vertical_align = VerticalAlign::Middle;
    style.text_color = Color{0.58f, 0.68f, 0.82f, 1.0f};
    return style;
}

void set_status(Label* status, StringView text, Color color) {
    status->set_text(text);
    status->set_text_color(color);
}

void update_display(Label* display, const String& text) {
    display->set_text(text.empty() ? "0" : text);
}

} // namespace

int main() {
    auto init_result = platform::initialize();
    if (!init_result) {
        std::cerr << "Failed to initialize platform: " << init_result.error().message << "\n";
        return 1;
    }

    ApplicationConfig config;
    config.window.title = "FrostUI Calculator";
    config.window.show_maximize_button = false;
    config.window.width = 436;
    config.window.height = 500;

    auto app_result = Application::create(config);
    if (!app_result) {
        std::cerr << "Failed to create application: " << app_result.error().message << "\n";
        return 1;
    }

    auto& app = *app_result.value();

    ContainerStyle shell_layout;
    shell_layout.direction = FlexDirection::Column;
    shell_layout.gap = 18.0f;

    SurfaceStyle shell_style{
        .fill = Color{0.09f, 0.11f, 0.15f, 1.0f},
        .border = Color{0.09f, 0.11f, 0.15f, 1.0f},
        .radius = 0.0f,
        .border_width = 0.0f
    };

    auto root = Surface::create(shell_layout, shell_style);
    root->set_padding(Edges{24.0f, 24.0f, 24.0f, 24.0f});

    ContainerStyle display_layout;
    display_layout.direction = FlexDirection::Column;
    display_layout.gap = 8.0f;

    SurfaceStyle display_surface_style{
        .fill = Color{0.04f, 0.07f, 0.11f, 1.0f},
        .border = Color{0.04f, 0.07f, 0.11f, 1.0f},
        .radius = 18.0f,
        .border_width = 0.0f
    };

    auto display_panel = Surface::create(display_layout, display_surface_style);
    display_panel->set_padding(Edges{16.0f, 16.0f, 16.0f, 16.0f});

    auto status = Label::create("", status_style());
    auto* status_ptr = status.get();
    display_panel->add_child(std::move(status));

    auto display = Label::create("0", display_style());
    display->set_padding(Edges{10.0f, 4.0f, 10.0f, 4.0f});
    auto* display_ptr = display.get();
    display_panel->add_child(std::move(display));

    root->add_child(std::move(display_panel));

    String current_input = "0";
    std::optional<double> stored_value;
    char pending_operator = '\0';
    bool reset_input = false;
    bool showing_error = false;

    auto set_error = [&](StringView message) {
        showing_error = true;
        current_input = "Error";
        stored_value.reset();
        pending_operator = '\0';
        reset_input = true;
        update_display(display_ptr, current_input);
        set_status(status_ptr, message, Color{0.88f, 0.42f, 0.42f, 1.0f});
    };

    auto clear_all = [&]() {
        current_input = "0";
        stored_value.reset();
        pending_operator = '\0';
        reset_input = false;
        showing_error = false;
        update_display(display_ptr, current_input);
        set_status(status_ptr, "", Color{0.67f, 0.75f, 0.86f, 1.0f});
    };

    auto apply_operation = [&](double lhs, double rhs, char op) -> std::optional<double> {
        switch (op) {
            case '+':
                return lhs + rhs;
            case '-':
                return lhs - rhs;
            case '*':
                return lhs * rhs;
            case '/':
                if (rhs == 0.0) {
                    return std::nullopt;
                }
                return lhs / rhs;
            default:
                return rhs;
        }
    };

    auto append_digit = [&](StringView digit) {
        if (showing_error) {
            clear_all();
        }

        if (reset_input) {
            current_input = "0";
            reset_input = false;
        }

        if (current_input == "0") {
            current_input = String(digit);
        } else if (current_input == "-0") {
            current_input = "-" + String(digit);
        } else {
            current_input += digit;
        }

        update_display(display_ptr, current_input);
        set_status(status_ptr, "", Color{0.67f, 0.75f, 0.86f, 1.0f});
    };

    auto append_decimal = [&]() {
        if (showing_error) {
            clear_all();
        }

        if (reset_input) {
            current_input = "0";
            reset_input = false;
        }

        if (current_input.find('.') == String::npos) {
            current_input += ".";
        }

        update_display(display_ptr, current_input);
    };

    auto toggle_sign = [&]() {
        if (showing_error) {
            clear_all();
        }

        if (current_input == "0") {
            return;
        }

        if (!current_input.empty() && current_input.front() == '-') {
            current_input.erase(0, 1);
        } else {
            current_input.insert(0, "-");
        }

        update_display(display_ptr, current_input);
    };

    auto backspace = [&]() {
        if (showing_error) {
            clear_all();
            return;
        }

        if (reset_input) {
            current_input = "0";
            reset_input = false;
            update_display(display_ptr, current_input);
            return;
        }

        if (!current_input.empty()) {
            current_input.pop_back();
        }

        if (current_input.empty() || current_input == "-") {
            current_input = "0";
        }

        update_display(display_ptr, current_input);
    };

    auto choose_operator = [&](char op) {
        if (showing_error) {
            return;
        }

        auto value = parse_number(current_input);
        if (!value) {
            set_error("Invalid number");
            return;
        }

        if (stored_value && pending_operator != '\0' && !reset_input) {
            auto result = apply_operation(*stored_value, *value, pending_operator);
            if (!result) {
                set_error("Cannot divide by zero");
                return;
            }
            stored_value = *result;
            current_input = format_number(*result);
            update_display(display_ptr, current_input);
        } else {
            stored_value = *value;
        }

        pending_operator = op;
        reset_input = true;
        set_status(status_ptr, String("Operator: ") + op, Color{0.94f, 0.78f, 0.40f, 1.0f});
    };

    auto evaluate = [&]() {
        if (showing_error) {
            return;
        }

        if (!stored_value || pending_operator == '\0') {
            return;
        }

        auto rhs = parse_number(current_input);
        if (!rhs) {
            set_error("Invalid number");
            return;
        }

        auto result = apply_operation(*stored_value, *rhs, pending_operator);
        if (!result) {
            set_error("Cannot divide by zero");
            return;
        }

        current_input = format_number(*result);
        stored_value.reset();
        pending_operator = '\0';
        reset_input = true;
        update_display(display_ptr, current_input);
        set_status(status_ptr, "Result", Color{0.46f, 0.82f, 0.52f, 1.0f});
    };

    ContainerStyle keypad_layout;
    keypad_layout.direction = FlexDirection::Column;
    keypad_layout.gap = 10.0f;

    SurfaceStyle keypad_surface_style{
        .fill = Color{0.11f, 0.14f, 0.19f, 1.0f},
        .border = Color{0.11f, 0.14f, 0.19f, 1.0f},
        .radius = 20.0f,
        .border_width = 0.0f
    };

    auto grid = Surface::create(keypad_layout, keypad_surface_style);
    grid->set_padding(Edges{14.0f, 14.0f, 14.0f, 14.0f});

    auto add_flex_button = [&](Container* row, Unique<Button> button) {
        row->add_child(std::move(button));
        row->set_child_flex(row->children().size() - 1, FlexItem{.grow = 1.0f, .shrink = 1.0f});
    };

    auto make_digit_button = [&](StringView text) {
        auto button = Button::create(text, digit_button_style());
        button->on_click.connect([&, digit = String(text)]() {
            append_digit(digit);
        });
        return button;
    };

    auto make_operator_button = [&](StringView text, char op) {
        auto button = Button::create(text, operator_button_style());
        button->on_click.connect([&, op]() {
            choose_operator(op);
        });
        return button;
    };

    auto make_utility_button = [&](StringView text, auto handler) {
        auto button = Button::create(text, utility_button_style());
        button->on_click.connect(handler);
        return button;
    };

    auto row1 = HBox(10.0f);
    add_flex_button(row1.get(), make_utility_button("AC", [&]() { clear_all(); }));
    add_flex_button(row1.get(), make_utility_button("DEL", [&]() { backspace(); }));
    add_flex_button(row1.get(), make_utility_button("+/-", [&]() { toggle_sign(); }));
    add_flex_button(row1.get(), make_operator_button("/", '/'));
    grid->add_child(std::move(row1));

    auto row2 = HBox(10.0f);
    add_flex_button(row2.get(), make_digit_button("7"));
    add_flex_button(row2.get(), make_digit_button("8"));
    add_flex_button(row2.get(), make_digit_button("9"));
    add_flex_button(row2.get(), make_operator_button("*", '*'));
    grid->add_child(std::move(row2));

    auto row3 = HBox(10.0f);
    add_flex_button(row3.get(), make_digit_button("4"));
    add_flex_button(row3.get(), make_digit_button("5"));
    add_flex_button(row3.get(), make_digit_button("6"));
    add_flex_button(row3.get(), make_operator_button("-", '-'));
    grid->add_child(std::move(row3));

    auto row4 = HBox(10.0f);
    add_flex_button(row4.get(), make_digit_button("1"));
    add_flex_button(row4.get(), make_digit_button("2"));
    add_flex_button(row4.get(), make_digit_button("3"));
    add_flex_button(row4.get(), make_operator_button("+", '+'));
    grid->add_child(std::move(row4));

    auto row5 = HBox(10.0f);
    auto quit_button = Button::create("Q", utility_button_style());
    quit_button->on_click.connect([&app]() {
        app.quit();
    });
    add_flex_button(row5.get(), std::move(quit_button));
    
    add_flex_button(row5.get(), make_digit_button("0"));
    auto decimal_button = Button::create(".", digit_button_style());
    decimal_button->on_click.connect([&]() {
        append_decimal();
    });
    add_flex_button(row5.get(), std::move(decimal_button));
    add_flex_button(row5.get(), make_utility_button("=", [&]() { evaluate(); }));

    
    grid->add_child(std::move(row5));

    root->add_child(std::move(grid));

    clear_all();

    app.set_root(std::move(root));
    app.run();

    platform::shutdown();
    return 0;
}
