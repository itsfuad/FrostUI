# FrostUI

A modern C++20 GUI framework with platform abstraction for Linux (X11) and Windows (Win32).

## Features

- **Zero external dependencies** - Custom math types, embedded bitmap font
- **Retained-mode UI** with signal-slot architecture
- **Flexbox-like layout system** (HBox, VBox, Center containers)
- **Essential widgets**: Button, Label, TextInput, Container
- **Cross-platform**: Linux (X11) and Windows (Win32)
- **Modern C++20**: concepts, constexpr, std::optional, RAII
- **Rust-style error handling** with `Result<T>`
- **Software renderer** by default, optional Vulkan backend

## Requirements

- C++20 compatible compiler (GCC 11+, Clang 14+, MSVC 2022+)
- CMake 3.20+
- **Linux**: X11 development libraries
- **Windows**: Win32 SDK (included with Visual Studio)

### Linux Dependencies

```bash
# Ubuntu/Debian
sudo apt install libx11-dev

# Arch Linux
sudo pacman -S libx11

# Fedora
sudo dnf install libX11-devel
```

### Optional: Vulkan SDK (for GPU rendering)

If you want to use the Vulkan renderer instead of the software renderer:

```bash
# Ubuntu/Debian
sudo apt install vulkan-sdk

# Arch Linux
sudo pacman -S vulkan-devel

# Fedora
sudo dnf install vulkan-devel
```

## Building

```bash
# Clone and enter directory
cd FrostUI

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run tests
ctest --test-dir build
```

## Running the Demo

After building:

```bash
# Linux
./build/bin/frost_demo

# Windows
.\build\bin\Release\frost_demo.exe
```

The demo application showcases:
- Counter with increment/decrement/reset buttons
- Text input with greeting functionality
- Flexbox layout with VBox and HBox containers

## Quick Start

```cpp
#include <frost/frost.hpp>

using namespace frost;

int main() {
    // Initialize platform
    if (!platform::initialize()) {
        return 1;
    }

    // Create application
    ApplicationConfig config;
    config.window.title = "My App";
    config.window.width = 800;
    config.window.height = 600;

    auto app_result = Application::create(config);
    if (!app_result) {
        return 1;
    }
    auto& app = *app_result.value();

    // Build UI with flexbox layout
    auto root = VBox(10.0f);  // 10px gap between children

    // Add a label
    root->add_child(Label::create("Hello, FrostUI!"));

    // Add a button with click handler
    auto button = Button::create("Click Me");
    button->on_click.connect([]() {
        // Handle click
    });
    root->add_child(std::move(button));

    // Add text input
    auto input = TextInput::create("Enter text...");
    input->on_text_changed.connect([](const String& text) {
        // Handle text changes
    });
    root->add_child(std::move(input));

    // Run the application
    app.set_root(std::move(root));
    app.run();

    platform::shutdown();
    return 0;
}
```

## Layout System

FrostUI uses a flexbox-inspired layout system:

```cpp
// Vertical stack with 16px spacing
auto vbox = VBox(16.0f);

// Horizontal row with 8px spacing
auto hbox = HBox(8.0f);

// Centered content
auto centered = Center();

// Full control with ContainerStyle
ContainerStyle style;
style.direction = FlexDirection::Row;
style.justify = FlexJustify::SpaceBetween;
style.align_items = FlexAlign::Center;
style.gap = 12.0f;
auto container = Container::create(style);
```

## Widgets

### Button
```cpp
auto btn = Button::create("Label");
btn->on_click.connect([]() { /* clicked */ });
```

### Label
```cpp
auto label = Label::create("Text");
label->set_text_align(TextAlign::Center);
label->set_text_color(Color{1.0f, 0.0f, 0.0f, 1.0f});
```

### TextInput
```cpp
auto input = TextInput::create("Placeholder");
input->on_text_changed.connect([](const String& text) { });
input->on_submit.connect([](const String& text) { });  // Enter pressed
```

## Project Structure

```
include/frost/
  frost.hpp              # Master header
  core/                  # Types, math, signals, result
  platform/              # Window, input abstractions
  graphics/              # DrawList, renderer interfaces
  ui/                    # Widget, layout, application
    widgets/             # Button, Label, TextInput

src/
  core/                  # Core implementations
  platform/
    linux/               # X11 window
    win32/               # Win32 window
  graphics/              # DrawList implementation
  ui/                    # Widget, layout, application

tests/                   # Catch2 unit tests
examples/demo/           # Demo application
```

## License

MIT License
