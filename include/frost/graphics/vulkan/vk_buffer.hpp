#pragma once

#include "frost/core/result.hpp"

namespace frost {

class VkBufferResource {
public:
    VkBufferResource() = default;
    ~VkBufferResource() = default;

    [[nodiscard]] Result<void> initialize(usize size);
    void shutdown();

    [[nodiscard]] bool is_initialized() const { return initialized_; }
    [[nodiscard]] usize size() const { return size_; }

private:
    bool initialized_{false};
    usize size_{0};
};

} // namespace frost
