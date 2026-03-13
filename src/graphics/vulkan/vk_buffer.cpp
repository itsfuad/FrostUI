#include "frost/graphics/vulkan/vk_buffer.hpp"

namespace frost {

Result<void> VkBufferResource::initialize(usize size) {
	if (size == 0) {
		return Error{ErrorCode::InvalidArgument, "Buffer size must be greater than zero"};
	}

	size_ = size;
	initialized_ = true;
	return {};
}

void VkBufferResource::shutdown() {
	size_ = 0;
	initialized_ = false;
}

} // namespace frost
