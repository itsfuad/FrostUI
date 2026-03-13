#include <frost/frost.hpp>
#include <iostream>

using namespace frost;

int main() {
    // Initialize platform
    auto init_result = platform::initialize();
    if (!init_result) {
        std::cerr << "Failed to initialize platform: " << init_result.error().message << "\n";
        return 1;
    }

    // Create application
    ApplicationConfig config;
    config.window.title = "FrostUI Demo";
    config.window.width = 800;
    config.window.height = 600;

    auto app_result = Application::create(config);
    if (!app_result) {
        std::cerr << "Failed to create application: " << app_result.error().message << "\n";
        return 1;
    }

    auto& app = *app_result.value();

    // Optional custom font loading (PSF1/PSF2)
    // app.load_font_from_file("/path/to/font.psf");

    // Build UI
    auto root = VBox(16.0f);
    root->set_padding(Edges{20.0f, 20.0f, 20.0f, 20.0f});

    // Title label
    auto title = Label::create("FrostUI Demo");
    LabelStyle title_style;
    title_style.font_size = 24.0f;
    title_style.text_align = TextAlign::Center;
    title->set_style(title_style);
    root->add_child(std::move(title));

    // Description label
    auto desc = Label::create("A minimal GUI framework demonstration");
    LabelStyle desc_style;
    desc_style.font_size = 14.0f;
    desc_style.text_color = Color{0.7f, 0.7f, 0.7f, 1.0f};
    desc_style.text_align = TextAlign::Center;
    desc->set_style(desc_style);
    root->add_child(std::move(desc));

    // Create a horizontal row for buttons
    auto button_row = HBox(12.0f);

    // Counter display
    int counter = 0;
    auto counter_label = Label::create("Counter: 0");
    counter_label->set_text_align(TextAlign::Center);
    auto* counter_ptr = counter_label.get();

    // Increment button
    auto inc_button = Button::create("Increment");
    inc_button->on_click.connect([&counter, counter_ptr]() {
        counter++;
        counter_ptr->set_text("Counter: " + std::to_string(counter));
    });
    button_row->add_child(std::move(inc_button));

    // Decrement button
    auto dec_button = Button::create("Decrement");
    dec_button->on_click.connect([&counter, counter_ptr]() {
        counter--;
        counter_ptr->set_text("Counter: " + std::to_string(counter));
    });
    button_row->add_child(std::move(dec_button));

    // Reset button
    auto reset_button = Button::create("Reset");
    reset_button->on_click.connect([&counter, counter_ptr]() {
        counter = 0;
        counter_ptr->set_text("Counter: 0");
    });
    button_row->add_child(std::move(reset_button));

    root->add_child(std::move(counter_label));
    root->add_child(std::move(button_row));

    // Text input section
    auto input_section = VBox(8.0f);

    auto input_label = Label::create("Enter your name:");
    input_section->add_child(std::move(input_label));

    auto name_input = TextInput::create("Type here...");
    auto* name_input_ptr = name_input.get();
    input_section->add_child(std::move(name_input));

    auto greeting_label = Label::create("");
    greeting_label->set_text_color(Color{0.4f, 0.8f, 0.4f, 1.0f});
    auto* greeting_ptr = greeting_label.get();

    auto greet_button = Button::create("Greet");
    greet_button->on_click.connect([name_input_ptr, greeting_ptr]() {
        const auto& name = name_input_ptr->text();
        if (name.empty()) {
            greeting_ptr->set_text("Please enter your name!");
        } else {
            greeting_ptr->set_text("Hello, " + name + "!");
        }
    });
    input_section->add_child(std::move(greet_button));
    input_section->add_child(std::move(greeting_label));

    root->add_child(std::move(input_section));

    // New controls section
    auto controls_section = VBox(8.0f);

    auto controls_title = Label::create("Controls");
    LabelStyle controls_title_style;
    controls_title_style.font_size = 18.0f;
    controls_title->set_style(controls_title_style);
    controls_section->add_child(std::move(controls_title));

    auto feature_checkbox = Checkbox::create("Enable advanced mode");
    auto controls_status = Label::create("Advanced mode: off");
    auto* controls_status_ptr = controls_status.get();
    feature_checkbox->on_toggled.connect([controls_status_ptr](bool checked) {
        controls_status_ptr->set_text(checked ? "Advanced mode: on" : "Advanced mode: off");
    });
    controls_section->add_child(std::move(feature_checkbox));

    auto quality_row = HBox(10.0f);
    auto quality_label = Label::create("Quality:");
    auto quality_value = Label::create("Balanced");
    auto* quality_value_ptr = quality_value.get();

    auto quality_low = RadioButton::create("Low", "quality");
    auto quality_balanced = RadioButton::create("Balanced", "quality");
    auto quality_high = RadioButton::create("High", "quality");
    quality_balanced->set_selected(true);

    quality_low->on_selected.connect([quality_value_ptr](bool selected) {
        if (selected) quality_value_ptr->set_text("Low");
    });
    quality_balanced->on_selected.connect([quality_value_ptr](bool selected) {
        if (selected) quality_value_ptr->set_text("Balanced");
    });
    quality_high->on_selected.connect([quality_value_ptr](bool selected) {
        if (selected) quality_value_ptr->set_text("High");
    });

    quality_row->add_child(std::move(quality_label));
    quality_row->add_child(std::move(quality_low));
    quality_row->add_child(std::move(quality_balanced));
    quality_row->add_child(std::move(quality_high));
    quality_row->add_child(std::move(quality_value));
    controls_section->add_child(std::move(quality_row));

    auto volume_row = VBox(4.0f);
    auto volume_label = Label::create("Volume: 50");
    auto* volume_label_ptr = volume_label.get();
    auto volume_slider = Slider::create(0.0f, 100.0f, 50.0f);
    volume_slider->set_step(1.0f);
    volume_slider->on_value_changed.connect([volume_label_ptr](f32 value) {
        volume_label_ptr->set_text("Volume: " + std::to_string(static_cast<int>(value + 0.5f)));
    });
    volume_row->add_child(std::move(volume_label));
    volume_row->add_child(std::move(volume_slider));
    controls_section->add_child(std::move(volume_row));

    controls_section->add_child(std::move(controls_status));
    root->add_child(std::move(controls_section));

    // Quit button at bottom
    auto quit_button = Button::create("Quit");
    quit_button->on_click.connect([&app]() {
        app.quit();
    });
    root->add_child(std::move(quit_button));

    // Set root and run
    app.set_root(std::move(root));

    std::cout << "FrostUI Demo started. Close the window to exit.\n";
    app.run();

    platform::shutdown();
    return 0;
}
