#include <frost/frost.hpp>

#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>

using namespace frost;

namespace {

std::string format_number(double value) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(4) << value;

    auto text = stream.str();
    while (!text.empty() && text.back() == '0') {
        text.pop_back();
    }
    if (!text.empty() && text.back() == '.') {
        text.pop_back();
    }
    if (text.empty()) {
        text = "0";
    }
    return text;
}

std::optional<double> parse_number(const String& text) {
    if (text.empty()) {
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

ButtonStyle primary_button_style() {
    ButtonStyle style;
    style.background_color = Color{0.15f, 0.52f, 0.42f, 1.0f};
    style.hover_color = Color{0.21f, 0.62f, 0.50f, 1.0f};
    style.pressed_color = Color{0.11f, 0.41f, 0.33f, 1.0f};
    return style;
}

void set_message(Label* label, StringView text, Color color) {
    label->set_text(text);
    label->set_text_color(color);
}

} // namespace

int main() {
    auto init_result = platform::initialize();
    if (!init_result) {
        std::cerr << "Failed to initialize platform: " << init_result.error().message << "\n";
        return 1;
    }

    ApplicationConfig config;
    config.window.title = "FrostUI Unit Converter";
    config.window.width = 560;
    config.window.height = 520;

    auto app_result = Application::create(config);
    if (!app_result) {
        std::cerr << "Failed to create application: " << app_result.error().message << "\n";
        return 1;
    }

    auto& app = *app_result.value();

    auto root = VBox(14.0f);
    root->set_padding(Edges{20.0f, 20.0f, 20.0f, 20.0f});

    auto title = Label::create("Unit Converter");
    title->set_font_size(24.0f);
    title->set_text_align(TextAlign::Center);
    root->add_child(std::move(title));

    auto subtitle = Label::create("Three small tools built from Button, Label and TextInput.");
    subtitle->set_text_color(Color{0.72f, 0.76f, 0.84f, 1.0f});
    subtitle->set_text_align(TextAlign::Center);
    root->add_child(std::move(subtitle));

    auto value_input = TextInput::create("Value");
    auto* value_input_ptr = value_input.get();
    root->add_child(std::move(value_input));

    auto result_label = Label::create("Converted value: ");
    result_label->set_font_size(20.0f);
    result_label->set_text_align(TextAlign::Center);
    auto* result_label_ptr = result_label.get();

    auto message_label = Label::create("Select a conversion.");
    message_label->set_text_align(TextAlign::Center);
    message_label->set_text_color(Color{0.67f, 0.75f, 0.86f, 1.0f});
    auto* message_label_ptr = message_label.get();

    auto sections = VBox(10.0f);

    auto make_converter_button = [&](StringView text, StringView suffix, auto convert) {
        auto button = Button::create(text, primary_button_style());
        button->on_click.connect([value_input_ptr, result_label_ptr, message_label_ptr, suffix = String(suffix), convert]() {
            auto input = parse_number(value_input_ptr->text());
            if (!input) {
                set_message(message_label_ptr, "Enter a valid number first.", Color{0.85f, 0.45f, 0.45f, 1.0f});
                result_label_ptr->set_text("Converted value: ");
                return;
            }

            const auto converted = convert(*input);
            result_label_ptr->set_text("Converted value: " + format_number(converted) + " " + suffix);
            set_message(message_label_ptr, "Conversion complete.", Color{0.46f, 0.82f, 0.52f, 1.0f});
        });
        return button;
    };

    auto length_title = Label::create("Length");
    length_title->set_text_color(Color{0.94f, 0.86f, 0.64f, 1.0f});
    sections->add_child(std::move(length_title));

    auto length_row = HBox(10.0f);
    length_row->add_child(make_converter_button("km -> mi", "mi", [](double value) {
        return value * 0.621371;
    }));
    length_row->add_child(make_converter_button("mi -> km", "km", [](double value) {
        return value / 0.621371;
    }));
    sections->add_child(std::move(length_row));

    auto weight_title = Label::create("Weight");
    weight_title->set_text_color(Color{0.94f, 0.86f, 0.64f, 1.0f});
    sections->add_child(std::move(weight_title));

    auto weight_row = HBox(10.0f);
    weight_row->add_child(make_converter_button("kg -> lb", "lb", [](double value) {
        return value * 2.20462;
    }));
    weight_row->add_child(make_converter_button("lb -> kg", "kg", [](double value) {
        return value / 2.20462;
    }));
    sections->add_child(std::move(weight_row));

    auto temp_title = Label::create("Temperature");
    temp_title->set_text_color(Color{0.94f, 0.86f, 0.64f, 1.0f});
    sections->add_child(std::move(temp_title));

    auto temp_row = HBox(10.0f);
    temp_row->add_child(make_converter_button("C -> F", "F", [](double value) {
        return (value * 9.0 / 5.0) + 32.0;
    }));
    temp_row->add_child(make_converter_button("F -> C", "C", [](double value) {
        return (value - 32.0) * 5.0 / 9.0;
    }));
    sections->add_child(std::move(temp_row));

    auto utility_row = HBox(10.0f);

    auto clear_button = Button::create("Clear");
    clear_button->on_click.connect([value_input_ptr, result_label_ptr, message_label_ptr]() {
        value_input_ptr->set_text("");
        result_label_ptr->set_text("Converted value: ");
        set_message(message_label_ptr, "Select a conversion.", Color{0.67f, 0.75f, 0.86f, 1.0f});
    });
    utility_row->add_child(std::move(clear_button));

    auto quit_button = Button::create("Quit");
    quit_button->on_click.connect([&app]() {
        app.quit();
    });
    utility_row->add_child(std::move(quit_button));

    sections->add_child(std::move(utility_row));

    root->add_child(std::move(sections));
    root->add_child(std::move(result_label));
    root->add_child(std::move(message_label));

    app.set_root(std::move(root));
    app.run();

    platform::shutdown();
    return 0;
}
