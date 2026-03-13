#pragma once

#include "frost/core/types.hpp"

namespace frost {

enum class RendererBackend : u8 {
    Software = 0,
    Gpu = 1,
};

[[nodiscard]] bool is_renderer_backend_available(RendererBackend backend);
[[nodiscard]] StringView renderer_backend_name(RendererBackend backend);

} // namespace frost
