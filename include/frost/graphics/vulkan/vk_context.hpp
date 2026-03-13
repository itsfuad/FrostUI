#pragma once

#include "frost/core/result.hpp"

namespace frost {

class PlatformWindow;

class VkContext {
public:
    VkContext() = default;
    ~VkContext() = default;

    [[nodiscard]] Result<void> initialize(PlatformWindow& window);
    void shutdown();

    [[nodiscard]] bool is_initialized() const { return initialized_; }

private:
    bool initialized_{false};
};

} // namespace frost
