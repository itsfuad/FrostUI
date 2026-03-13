#pragma once

#include "frost/core/result.hpp"
#include "frost/graphics/draw_list.hpp"

namespace frost {

class PlatformWindow;

class VkRenderer {
public:
    struct Impl;

    VkRenderer();
    ~VkRenderer();

    [[nodiscard]] Result<void> initialize(PlatformWindow& window);
    [[nodiscard]] Result<void> render(const DrawList& draw_list, i32 width, i32 height);
    void shutdown();

    [[nodiscard]] bool is_initialized() const;

private:
    Unique<Impl> impl_;
};

} // namespace frost
