# ─────────────────────────────────────────────────────────────────────────────
# Dependencies Configuration
# ─────────────────────────────────────────────────────────────────────────────

include(FetchContent)

# ─────────────────────────────────────────────────────────────────────────────
# FreeType (optional - enables TTF/OTF loading)
# ─────────────────────────────────────────────────────────────────────────────

option(FROST_USE_FREETYPE "Enable TTF/OTF font loading via FreeType" ON)

if(FROST_USE_FREETYPE)
    find_package(Freetype)
    if(Freetype_FOUND)
        message(STATUS "Found FreeType: ${FREETYPE_INCLUDE_DIRS}")
        set(FROST_HAS_FREETYPE TRUE CACHE INTERNAL "")
    else()
        message(WARNING "FreeType not found - TTF/OTF font loading will be disabled")
        set(FROST_HAS_FREETYPE FALSE CACHE INTERNAL "")
    endif()
else()
    message(STATUS "FreeType disabled - TTF/OTF font loading will be disabled")
    set(FROST_HAS_FREETYPE FALSE CACHE INTERNAL "")
endif()

# ─────────────────────────────────────────────────────────────────────────────
# Vulkan SDK (optional - user can enable with FROST_USE_VULKAN)
# ─────────────────────────────────────────────────────────────────────────────

option(FROST_USE_VULKAN "Use Vulkan renderer instead of software renderer" OFF)

if(FROST_USE_VULKAN)
    find_package(Vulkan REQUIRED)
    message(STATUS "Found Vulkan: ${Vulkan_INCLUDE_DIRS}")
    set(FROST_HAS_VULKAN TRUE CACHE INTERNAL "")

    # Vulkan Memory Allocator
    FetchContent_Declare(
        VulkanMemoryAllocator
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG v3.1.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(VulkanMemoryAllocator)
else()
    message(STATUS "Using software renderer (no external dependencies)")
    set(FROST_HAS_VULKAN FALSE CACHE INTERNAL "")
endif()

# ─────────────────────────────────────────────────────────────────────────────
# Catch2 (Testing) - only external dependency for tests
# ─────────────────────────────────────────────────────────────────────────────

if(FROST_BUILD_TESTS)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
        GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
endif()
