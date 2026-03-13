#include <frost/frost.hpp>
#include <array>
#include <iostream>

using namespace frost;

int main() {
    auto init_result = platform::initialize();
    if (!init_result) {
        std::cerr << "Failed to initialize platform: " << init_result.error().message << "\n";
        return 1;
    }

    ApplicationConfig config;
    config.window.title = "FrostUI Font Showcase";
    config.window.width = 920;
    config.window.height = 640;
    config.renderer_backend = RendererBackend::Software;

    auto app_result = Application::create(config);
    if (!app_result) {
        std::cerr << "Failed to create application: " << app_result.error().message << "\n";
        platform::shutdown();
        return 1;
    }

    auto& app = *app_result.value();

    struct FontOption {
        const char* name;
        const char* relative_path;
    };

    const std::array<FontOption, 3> fonts{{
        {"Fira Mono", "examples/assets/fonts/FiraMono-Regular.ttf"},
        {"Montserrat", "examples/assets/fonts/Montserrat-Var.ttf"},
        {"Playfair Display", "examples/assets/fonts/PlayfairDisplay-Var.ttf"},
    }};

    auto root = VBox(12.0f);
    root->set_padding(Edges{18.0f, 18.0f, 18.0f, 18.0f});

    auto title = Label::create("Font Showcase (TTF/OTF via FreeType)");
    title->set_font_size(24.0f);
    root->add_child(std::move(title));

    auto hint = Label::create("Click a button to switch the global UI font. Use Reset to return to built-in bitmap font.");
    hint->set_text_color(Color{0.72f, 0.72f, 0.72f, 1.0f});
    root->add_child(std::move(hint));

    auto status = Label::create("Status: ready");
    auto* status_ptr = status.get();
    root->add_child(std::move(status));

    auto current_font = Label::create("Current font: default built-in");
    auto* current_font_ptr = current_font.get();
    root->add_child(std::move(current_font));

    auto sample_text = Label::create("The quick brown fox jumps over the lazy dog  0123456789");
    sample_text->set_font_size(24.0f);
    root->add_child(std::move(sample_text));

    auto sample_text_2 = Label::create("Sphinx of black quartz, judge my vow.");
    sample_text_2->set_font_size(20.0f);
    root->add_child(std::move(sample_text_2));

    auto sample_text_3 = Label::create("ABCDEFGHIJKLMNOPQRSTUVWXYZ  abcdefghijklmnopqrstuvwxyz");
    sample_text_3->set_font_size(18.0f);
    root->add_child(std::move(sample_text_3));

    auto button_row = HBox(8.0f);

    for (const auto& entry : fonts) {
        auto button = Button::create(String("Use ") + entry.name);
        const String font_name = entry.name;
        const String relative_path = entry.relative_path;
        button->on_click.connect([&app, font_name, relative_path, status_ptr, current_font_ptr]() {
            const String absolute_path = String(FROSTUI_SOURCE_DIR) + "/" + relative_path;
            auto load_result = app.load_font_from_file(absolute_path, 24);
            if (!load_result) {
                status_ptr->set_text("Status: failed to load " + font_name + " (" + load_result.error().message + ")");
                return;
            }

            status_ptr->set_text("Status: loaded " + font_name);
            current_font_ptr->set_text("Current font: " + font_name);
        });
        button_row->add_child(std::move(button));
    }

    auto reset_button = Button::create("Reset Default");
    reset_button->on_click.connect([&app, status_ptr, current_font_ptr]() {
        app.reset_font();
        status_ptr->set_text("Status: reverted to built-in font");
        current_font_ptr->set_text("Current font: default built-in");
    });
    button_row->add_child(std::move(reset_button));

    root->add_child(std::move(button_row));

    auto quit = Button::create("Quit");
    quit->on_click.connect([&app]() {
        app.quit();
    });
    root->add_child(std::move(quit));

    app.set_root(std::move(root));
    app.run();

    platform::shutdown();
    return 0;
}