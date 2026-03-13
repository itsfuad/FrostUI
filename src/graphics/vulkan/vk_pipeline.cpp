#include "frost/graphics/vulkan/vk_pipeline.hpp"

namespace frost {

Result<void> VkPipelineState::initialize() {
	initialized_ = true;
	return {};
}

void VkPipelineState::shutdown() {
	initialized_ = false;
}

} // namespace frost
