# FrostUI Examples

# Demo application
add_executable(frost_demo
    demo/main.cpp
)

target_link_libraries(frost_demo PRIVATE frost)

# Set output directory
set_target_properties(frost_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
