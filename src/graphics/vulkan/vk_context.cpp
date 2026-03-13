#include "frost/graphics/vulkan/vk_context.hpp"
#include "frost/platform/window.hpp"

namespace frost {

Result<void> VkContext::initialize(PlatformWindow& window) {
	initialized_ = true;
	return {};
}

void VkContext::shutdown() {
	initialized_ = false;
}

} // namespace frost
