cmake_minimum_required(VERSION 3.20)

project(FrostUI
    VERSION 0.1.0
    DESCRIPTION "Industry-grade GUI framework with Vulkan backend"
    LANGUAGES CXX
)

# ─────────────────────────────────────────────────────────────────────────────
# Build Options
# ─────────────────────────────────────────────────────────────────────────────

option(FROST_BUILD_EXAMPLES "Build example applications" ON)
option(FROST_BUILD_TESTS "Build unit tests" ON)
option(FROST_ENABLE_VALIDATION "Enable Vulkan validation layers in debug" ON)
option(FROST_ENABLE_PROFILING "Enable GPU profiling support" OFF)

# ─────────────────────────────────────────────────────────────────────────────
# Global Settings
# ─────────────────────────────────────────────────────────────────────────────

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include our CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CompilerFlags)
include(Dependencies)

# ─────────────────────────────────────────────────────────────────────────────
# Core Library
# ─────────────────────────────────────────────────────────────────────────────

add_library(frost_core STATIC
    src/core/math.cpp
    src/core/signals.cpp
)

target_include_directories(frost_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(frost_core
    PUBLIC
        glm::glm
)

# ─────────────────────────────────────────────────────────────────────────────
# Platform Library
# ─────────────────────────────────────────────────────────────────────────────

set(PLATFORM_SOURCES
    src/platform/window_base.cpp
)

if(WIN32)
    list(APPEND PLATFORM_SOURCES
        src/platform/win32/window_win32.cpp
        src/platform/win32/input_win32.cpp
    )
elseif(UNIX AND NOT APPLE)
    list(APPEND PLATFORM_SOURCES
        src/platform/linux/window_x11.cpp
    )
endif()

add_library(frost_platform STATIC ${PLATFORM_SOURCES})

target_include_directories(frost_platform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(frost_platform
    PUBLIC
        frost_core
)

if(WIN32)
    target_compile_definitions(frost_platform PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
    )
elseif(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(frost_platform PUBLIC X11::X11)
endif()

# ─────────────────────────────────────────────────────────────────────────────
# Graphics Library (Vulkan Backend)
# ─────────────────────────────────────────────────────────────────────────────

if(FROST_HAS_VULKAN)
    add_library(frost_graphics STATIC
        src/graphics/draw_list.cpp
        src/graphics/renderer.cpp
        src/graphics/vulkan/vk_context.cpp
        src/graphics/vulkan/vk_swapchain.cpp
        src/graphics/vulkan/vk_pipeline.cpp
        src/graphics/vulkan/vk_buffer.cpp
        src/graphics/vulkan/vk_renderer.cpp
    )

    target_include_directories(frost_graphics
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(frost_graphics
        PUBLIC
            frost_core
            frost_platform
            Vulkan::Vulkan
            GPUOpen::VulkanMemoryAllocator
    )

    if(FROST_ENABLE_VALIDATION)
        target_compile_definitions(frost_graphics PRIVATE FROST_VULKAN_VALIDATION=1)
    endif()
else()
    # Stub graphics library when Vulkan is not available
    add_library(frost_graphics STATIC
        src/graphics/draw_list.cpp
        src/graphics/renderer.cpp
    )

    target_include_directories(frost_graphics
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(frost_graphics
        PUBLIC
            frost_core
            frost_platform
    )

    target_compile_definitions(frost_graphics PUBLIC FROST_NO_VULKAN=1)
endif()

# ─────────────────────────────────────────────────────────────────────────────
# UI Library
# ─────────────────────────────────────────────────────────────────────────────

add_library(frost_ui STATIC
    src/ui/application.cpp
    src/ui/widget.cpp
    src/ui/layout.cpp
)

target_include_directories(frost_ui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(frost_ui
    PUBLIC
        frost_core
        frost_platform
        frost_graphics
)

# ─────────────────────────────────────────────────────────────────────────────
# Main Interface Library (combines all)
# ─────────────────────────────────────────────────────────────────────────────

add_library(frost INTERFACE)
target_link_libraries(frost INTERFACE
    frost_core
    frost_platform
    frost_graphics
    frost_ui
)

# ─────────────────────────────────────────────────────────────────────────────
# Shader Compilation (only when Vulkan is available)
# ─────────────────────────────────────────────────────────────────────────────

if(FROST_HAS_VULKAN)
    find_program(GLSLC_EXECUTABLE glslc HINTS $ENV{VULKAN_SDK}/Bin)

    if(GLSLC_EXECUTABLE)
        set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
        set(SHADER_BINARY_DIR ${CMAKE_BINARY_DIR}/shaders)

        file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

        set(SHADER_SOURCES
            ${SHADER_SOURCE_DIR}/rect.vert
            ${SHADER_SOURCE_DIR}/rect.frag
        )

        set(SHADER_SPIRV "")

        foreach(SHADER ${SHADER_SOURCES})
            get_filename_component(SHADER_NAME ${SHADER} NAME)
            set(SPIRV_OUTPUT ${SHADER_BINARY_DIR}/${SHADER_NAME}.spv)

            add_custom_command(
                OUTPUT ${SPIRV_OUTPUT}
                COMMAND ${GLSLC_EXECUTABLE} -O ${SHADER} -o ${SPIRV_OUTPUT}
                DEPENDS ${SHADER}
                COMMENT "Compiling shader ${SHADER_NAME}"
            )

            list(APPEND SHADER_SPIRV ${SPIRV_OUTPUT})
        endforeach()

        add_custom_target(frost_shaders ALL DEPENDS ${SHADER_SPIRV})
        add_dependencies(frost_graphics frost_shaders)
    else()
        message(WARNING "glslc not found - shaders will not be compiled")
    endif()
endif()

# ─────────────────────────────────────────────────────────────────────────────
# Examples
# ─────────────────────────────────────────────────────────────────────────────

if(FROST_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ─────────────────────────────────────────────────────────────────────────────
# Tests
# ─────────────────────────────────────────────────────────────────────────────

if(FROST_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ─────────────────────────────────────────────────────────────────────────────
# Installation (disabled during development)
# ─────────────────────────────────────────────────────────────────────────────

# TODO: Fix export sets for FetchContent dependencies before enabling
# include(GNUInstallDirs)
#
# install(TARGETS frost frost_core frost_platform frost_graphics frost_ui
#     EXPORT FrostUITargets
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )
#
# install(DIRECTORY include/frost
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )
#
# install(EXPORT FrostUITargets
#     FILE FrostUITargets.cmake
#     NAMESPACE FrostUI::
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FrostUI
# )
