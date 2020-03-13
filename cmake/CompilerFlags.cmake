# ─────────────────────────────────────────────────────────────────────────────
# Compiler Flags Configuration
# ─────────────────────────────────────────────────────────────────────────────

# Common warning flags
add_compile_options(
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:MSVC>:/permissive->
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wextra>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wpedantic>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wno-unused-parameter>
)

# Treat warnings as errors in CI builds
if(DEFINED ENV{CI})
    add_compile_options(
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Werror>
    )
endif()

# Debug/Release specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(FROST_DEBUG=1)

    if(MSVC)
        add_compile_options(/Zi /Od /RTC1)
    else()
        add_compile_options(-g -O0)
    endif()
else()
    add_compile_definitions(FROST_DEBUG=0)

    if(MSVC)
        add_compile_options(/O2 /GL)
        add_link_options(/LTCG)
    else()
        add_compile_options(-O3 -flto)
        add_link_options(-flto)
    endif()
endif()

# ASAN support for development
option(FROST_ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(FROST_ENABLE_ASAN AND NOT MSVC)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()
