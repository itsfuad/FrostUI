# ─────────────────────────────────────────────────────────────────────────────
# Dependencies Configuration
# ─────────────────────────────────────────────────────────────────────────────

include(FetchContent)

# ─────────────────────────────────────────────────────────────────────────────
# Vulkan SDK (optional for now)
# ─────────────────────────────────────────────────────────────────────────────

find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    message(STATUS "Found Vulkan: ${Vulkan_INCLUDE_DIRS}")
    set(FROST_HAS_VULKAN TRUE CACHE INTERNAL "")
else()
    message(WARNING "Vulkan SDK not found - graphics library will be stubbed")
    set(FROST_HAS_VULKAN FALSE CACHE INTERNAL "")
endif()

# ─────────────────────────────────────────────────────────────────────────────
# GLM (Mathematics)
# ─────────────────────────────────────────────────────────────────────────────

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)

set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

# ─────────────────────────────────────────────────────────────────────────────
# Vulkan Memory Allocator (only if Vulkan found)
# ─────────────────────────────────────────────────────────────────────────────

if(FROST_HAS_VULKAN)
    FetchContent_Declare(
        VulkanMemoryAllocator
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG v3.1.0
        GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(VulkanMemoryAllocator)
endif()

# ─────────────────────────────────────────────────────────────────────────────
# STB Libraries (header-only)
# ─────────────────────────────────────────────────────────────────────────────

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(stb)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

# ─────────────────────────────────────────────────────────────────────────────
# Catch2 (Testing)
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
