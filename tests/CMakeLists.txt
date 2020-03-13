# FrostUI Tests

include(CTest)
include(Catch)

# Core tests
add_executable(frost_core_tests
    core/test_math.cpp
    core/test_signals.cpp
    core/test_result.cpp
)

target_link_libraries(frost_core_tests
    PRIVATE
        frost_core
        Catch2::Catch2WithMain
)

# UI tests
add_executable(frost_ui_tests
    ui/test_widget.cpp
    ui/test_layout.cpp
)

target_link_libraries(frost_ui_tests
    PRIVATE
        frost_ui
        Catch2::Catch2WithMain
)

# Discover tests
catch_discover_tests(frost_core_tests)
catch_discover_tests(frost_ui_tests)
