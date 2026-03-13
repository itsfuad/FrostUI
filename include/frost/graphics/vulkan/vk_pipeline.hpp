#pragma once

#include "frost/core/result.hpp"

namespace frost {

class VkPipelineState {
public:
    VkPipelineState() = default;
    ~VkPipelineState() = default;

    [[nodiscard]] Result<void> initialize();
    void shutdown();

    [[nodiscard]] bool is_initialized() const { return initialized_; }

private:
    bool initialized_{false};
};

} // namespace frost
