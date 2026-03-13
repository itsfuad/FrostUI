#include "frost/graphics/vulkan/vk_swapchain.hpp"

namespace frost {

Result<void> VkSwapchain::initialize(i32 width, i32 height) {
	if (width <= 0 || height <= 0) {
		return Error{ErrorCode::InvalidArgument, "Invalid swapchain dimensions"};
	}

	initialized_ = true;
	return {};
}

Result<void> VkSwapchain::resize(i32 width, i32 height) {
	if (!initialized_) {
		return Error{ErrorCode::InvalidArgument, "Swapchain is not initialized"};
	}
	if (width <= 0 || height <= 0) {
		return Error{ErrorCode::InvalidArgument, "Invalid swapchain dimensions"};
	}

	return {};
}

void VkSwapchain::shutdown() {
	initialized_ = false;
}

} // namespace frost
