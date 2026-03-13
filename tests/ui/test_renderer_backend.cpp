#include <catch2/catch_test_macros.hpp>
#include "frost/graphics/renderer.hpp"

using namespace frost;

TEST_CASE("Renderer backend availability", "[ui][renderer]") {
    REQUIRE(is_renderer_backend_available(RendererBackend::Software));
    REQUIRE(renderer_backend_name(RendererBackend::Software) == "software");
    REQUIRE(renderer_backend_name(RendererBackend::Gpu) == "gpu");

#ifdef FROST_NO_VULKAN
    REQUIRE_FALSE(is_renderer_backend_available(RendererBackend::Gpu));
#else
    REQUIRE(is_renderer_backend_available(RendererBackend::Gpu));
#endif
}
