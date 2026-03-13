# FrostUI Examples

# Demo application
add_executable(frost_demo
    demo/main.cpp
)

target_link_libraries(frost_demo PRIVATE frost)

# Font showcase example
add_executable(frost_font_showcase
    font_showcase/main.cpp
)

target_link_libraries(frost_font_showcase PRIVATE frost)
target_compile_definitions(frost_font_showcase PRIVATE FROSTUI_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Calculator example
add_executable(frost_calculator
    calculator/main.cpp
)

target_link_libraries(frost_calculator PRIVATE frost)

# Unit converter example
add_executable(frost_unit_converter
    unit_converter/main.cpp
)

target_link_libraries(frost_unit_converter PRIVATE frost)

# Set output directory
set_target_properties(
    frost_demo
    frost_font_showcase
    frost_calculator
    frost_unit_converter
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
