#pragma once

#include "frost/core/result.hpp"

namespace frost {

class VkSwapchain {
public:
    VkSwapchain() = default;
    ~VkSwapchain() = default;

    [[nodiscard]] Result<void> initialize(i32 width, i32 height);
    [[nodiscard]] Result<void> resize(i32 width, i32 height);
    void shutdown();

    [[nodiscard]] bool is_initialized() const { return initialized_; }

private:
    bool initialized_{false};
};

} // namespace frost
