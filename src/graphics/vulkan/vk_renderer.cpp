#include "frost/graphics/vulkan/vk_renderer.hpp"
#include "frost/platform/window.hpp"

#ifdef FROST_NO_VULKAN

namespace frost {

struct VkRenderer::Impl {};

VkRenderer::VkRenderer()
	: impl_(make_unique<Impl>()) {}

VkRenderer::~VkRenderer() = default;

Result<void> VkRenderer::initialize(PlatformWindow& window) {
	return Error{ErrorCode::NotSupported, "Vulkan backend is not enabled in this build"};
}

Result<void> VkRenderer::render(const DrawList& draw_list, i32 width, i32 height) {
	return Error{ErrorCode::NotSupported, "Vulkan draw path is unavailable"};
}

void VkRenderer::shutdown() {}

bool VkRenderer::is_initialized() const {
	return false;
}

} // namespace frost

#else

#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#endif
#include <vulkan/vulkan.h>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>

namespace frost {

namespace {

Vector<char> read_binary_file(const std::filesystem::path& path) {
	std::ifstream file(path, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		return {};
	}

	const auto size = static_cast<usize>(file.tellg());
	Vector<char> buffer(size);
	file.seekg(0);
	file.read(buffer.data(), static_cast<std::streamsize>(size));
	return buffer;
}

Vector<char> load_shader_bytes(StringView filename) {
	const std::filesystem::path candidates[] = {
		std::filesystem::path("build/shaders") / String(filename),
		std::filesystem::path("../shaders") / String(filename),
		std::filesystem::path("shaders") / String(filename)
	};

	for (const auto& path : candidates) {
		auto bytes = read_binary_file(path);
		if (!bytes.empty()) {
			return bytes;
		}
	}

	return {};
}

} // namespace

struct VkRenderer::Impl {
	VkInstance instance{VK_NULL_HANDLE};
	VkPhysicalDevice physical_device{VK_NULL_HANDLE};
	VkDevice device{VK_NULL_HANDLE};
	VkQueue graphics_queue{VK_NULL_HANDLE};
	VkQueue present_queue{VK_NULL_HANDLE};
	VkSurfaceKHR surface{VK_NULL_HANDLE};
	VkSwapchainKHR swapchain{VK_NULL_HANDLE};
	Vector<VkImage> swapchain_images;
	Vector<VkImageView> swapchain_views;
	Vector<VkFramebuffer> framebuffers;
	Vector<VkCommandBuffer> command_buffers;
	VkRenderPass render_pass{VK_NULL_HANDLE};
	VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
	VkPipeline pipeline{VK_NULL_HANDLE};
	VkCommandPool command_pool{VK_NULL_HANDLE};
	VkSemaphore image_available{VK_NULL_HANDLE};
	VkSemaphore render_finished{VK_NULL_HANDLE};
	VkFence in_flight{VK_NULL_HANDLE};
	VkFormat swapchain_format{VK_FORMAT_UNDEFINED};
	VkExtent2D swapchain_extent{0, 0};
	VkBuffer vertex_buffer{VK_NULL_HANDLE};
	VkDeviceMemory vertex_memory{VK_NULL_HANDLE};
	VkDeviceSize vertex_capacity{0};
	VkBuffer index_buffer{VK_NULL_HANDLE};
	VkDeviceMemory index_memory{VK_NULL_HANDLE};
	VkDeviceSize index_capacity{0};
	VkPhysicalDeviceMemoryProperties memory_properties{};
	u32 graphics_queue_family{0};
	u32 present_queue_family{0};
	bool initialized{false};
};

VkRenderer::VkRenderer()
	: impl_(make_unique<Impl>()) {}

VkRenderer::~VkRenderer() {
	shutdown();
}

static u32 find_memory_type(const VkRenderer::Impl& impl, u32 type_filter, VkMemoryPropertyFlags properties) {
	for (u32 index = 0; index < impl.memory_properties.memoryTypeCount; ++index) {
		if ((type_filter & (1u << index)) != 0
			&& (impl.memory_properties.memoryTypes[index].propertyFlags & properties) == properties) {
			return index;
		}
	}

	return UINT32_MAX;
}

static Result<void> create_buffer(
	VkRenderer::Impl& impl,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkBuffer& buffer,
	VkDeviceMemory& memory
) {
	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(impl.device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
		return Error{ErrorCode::VulkanInitFailed, "Failed to create Vulkan buffer"};
	}

	VkMemoryRequirements requirements{};
	vkGetBufferMemoryRequirements(impl.device, buffer, &requirements);

	u32 memory_type = find_memory_type(impl, requirements.memoryTypeBits, properties);
	if (memory_type == UINT32_MAX) {
		vkDestroyBuffer(impl.device, buffer, nullptr);
		buffer = VK_NULL_HANDLE;
		return Error{ErrorCode::VulkanOutOfDeviceMemory, "No suitable Vulkan memory type found"};
	}

	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = requirements.size;
	alloc_info.memoryTypeIndex = memory_type;

	if (vkAllocateMemory(impl.device, &alloc_info, nullptr, &memory) != VK_SUCCESS) {
		vkDestroyBuffer(impl.device, buffer, nullptr);
		buffer = VK_NULL_HANDLE;
		return Error{ErrorCode::VulkanOutOfDeviceMemory, "Failed to allocate Vulkan buffer memory"};
	}

	vkBindBufferMemory(impl.device, buffer, memory, 0);
	return {};
}

static void destroy_buffer(VkRenderer::Impl& impl, VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize& capacity) {
	if (buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(impl.device, buffer, nullptr);
		buffer = VK_NULL_HANDLE;
	}
	if (memory != VK_NULL_HANDLE) {
		vkFreeMemory(impl.device, memory, nullptr);
		memory = VK_NULL_HANDLE;
	}
	capacity = 0;
}

static void destroy_swapchain_resources(VkRenderer::Impl& impl) {
	if (impl.device == VK_NULL_HANDLE) {
		return;
	}

	for (auto framebuffer : impl.framebuffers) {
		if (framebuffer != VK_NULL_HANDLE) {
			vkDestroyFramebuffer(impl.device, framebuffer, nullptr);
		}
	}
	impl.framebuffers.clear();

	for (auto view : impl.swapchain_views) {
		if (view != VK_NULL_HANDLE) {
			vkDestroyImageView(impl.device, view, nullptr);
		}
	}
	impl.swapchain_views.clear();
	impl.swapchain_images.clear();

	if (impl.render_pass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(impl.device, impl.render_pass, nullptr);
		impl.render_pass = VK_NULL_HANDLE;
	}

	if (impl.swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(impl.device, impl.swapchain, nullptr);
		impl.swapchain = VK_NULL_HANDLE;
	}
}

static Result<void> create_swapchain_resources(VkRenderer::Impl& impl, i32 width, i32 height) {
	VkSurfaceCapabilitiesKHR capabilities{};
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(impl.physical_device, impl.surface, &capabilities) != VK_SUCCESS) {
		return Error{ErrorCode::VulkanSwapchainCreationFailed, "Failed to query Vulkan surface capabilities"};
	}

	u32 format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(impl.physical_device, impl.surface, &format_count, nullptr);
	if (format_count == 0) {
		return Error{ErrorCode::VulkanSwapchainCreationFailed, "No Vulkan surface formats available"};
	}

	Vector<VkSurfaceFormatKHR> formats(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(impl.physical_device, impl.surface, &format_count, formats.data());

	u32 present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(impl.physical_device, impl.surface, &present_mode_count, nullptr);
	if (present_mode_count == 0) {
		return Error{ErrorCode::VulkanSwapchainCreationFailed, "No Vulkan present modes available"};
	}

	Vector<VkPresentModeKHR> present_modes(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(impl.physical_device, impl.surface, &present_mode_count, present_modes.data());

	VkSurfaceFormatKHR chosen_format = formats[0];
	for (const auto& candidate : formats) {
		if (candidate.format == VK_FORMAT_B8G8R8A8_UNORM && candidate.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			chosen_format = candidate;
			break;
		}
	}

	VkPresentModeKHR chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto& mode : present_modes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			chosen_present_mode = mode;
			break;
		}
	}

	VkExtent2D extent{};
	if (capabilities.currentExtent.width != UINT32_MAX) {
		extent = capabilities.currentExtent;
	} else {
		extent.width = static_cast<u32>(std::clamp(width, static_cast<i32>(capabilities.minImageExtent.width), static_cast<i32>(capabilities.maxImageExtent.width)));
		extent.height = static_cast<u32>(std::clamp(height, static_cast<i32>(capabilities.minImageExtent.height), static_cast<i32>(capabilities.maxImageExtent.height)));
	}

	u32 image_count = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0) {
		image_count = std::min(image_count, capabilities.maxImageCount);
	}

	VkSwapchainCreateInfoKHR swapchain_info{};
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.surface = impl.surface;
	swapchain_info.minImageCount = image_count;
	swapchain_info.imageFormat = chosen_format.format;
	swapchain_info.imageColorSpace = chosen_format.colorSpace;
	swapchain_info.imageExtent = extent;
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	u32 family_indices[] = {impl.graphics_queue_family, impl.present_queue_family};
	if (impl.graphics_queue_family != impl.present_queue_family) {
		swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_info.queueFamilyIndexCount = 2;
		swapchain_info.pQueueFamilyIndices = family_indices;
	} else {
		swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	swapchain_info.preTransform = capabilities.currentTransform;
	swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_info.presentMode = chosen_present_mode;
	swapchain_info.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(impl.device, &swapchain_info, nullptr, &impl.swapchain) != VK_SUCCESS) {
		return Error{ErrorCode::VulkanSwapchainCreationFailed, "Failed to create Vulkan swapchain"};
	}

	vkGetSwapchainImagesKHR(impl.device, impl.swapchain, &image_count, nullptr);
	impl.swapchain_images.resize(image_count);
	vkGetSwapchainImagesKHR(impl.device, impl.swapchain, &image_count, impl.swapchain_images.data());

	impl.swapchain_format = chosen_format.format;
	impl.swapchain_extent = extent;

	impl.swapchain_views.resize(impl.swapchain_images.size());
	for (usize i = 0; i < impl.swapchain_images.size(); ++i) {
		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = impl.swapchain_images[i];
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = impl.swapchain_format;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(impl.device, &view_info, nullptr, &impl.swapchain_views[i]) != VK_SUCCESS) {
			destroy_swapchain_resources(impl);
			return Error{ErrorCode::VulkanSwapchainCreationFailed, "Failed to create Vulkan image view"};
		}
	}

	VkAttachmentDescription color_attachment{};
	color_attachment.format = impl.swapchain_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	if (vkCreateRenderPass(impl.device, &render_pass_info, nullptr, &impl.render_pass) != VK_SUCCESS) {
		destroy_swapchain_resources(impl);
		return Error{ErrorCode::VulkanPipelineCreationFailed, "Failed to create Vulkan render pass"};
	}

	auto vert_shader = load_shader_bytes("rect.vert.spv");
	auto frag_shader = load_shader_bytes("rect.frag.spv");
	if (vert_shader.empty() || frag_shader.empty()) {
		destroy_swapchain_resources(impl);
		return Error{ErrorCode::VulkanShaderCompilationFailed, "Missing SPIR-V shaders (rect.vert.spv/rect.frag.spv)"};
	}

	VkShaderModuleCreateInfo vert_module_info{};
	vert_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vert_module_info.codeSize = vert_shader.size();
	vert_module_info.pCode = reinterpret_cast<const u32*>(vert_shader.data());

	VkShaderModuleCreateInfo frag_module_info{};
	frag_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	frag_module_info.codeSize = frag_shader.size();
	frag_module_info.pCode = reinterpret_cast<const u32*>(frag_shader.data());

	VkShaderModule vert_module{VK_NULL_HANDLE};
	VkShaderModule frag_module{VK_NULL_HANDLE};

	if (vkCreateShaderModule(impl.device, &vert_module_info, nullptr, &vert_module) != VK_SUCCESS
		|| vkCreateShaderModule(impl.device, &frag_module_info, nullptr, &frag_module) != VK_SUCCESS) {
		if (vert_module != VK_NULL_HANDLE) {
			vkDestroyShaderModule(impl.device, vert_module, nullptr);
		}
		if (frag_module != VK_NULL_HANDLE) {
			vkDestroyShaderModule(impl.device, frag_module, nullptr);
		}
		destroy_swapchain_resources(impl);
		return Error{ErrorCode::VulkanPipelineCreationFailed, "Failed to create Vulkan shader modules"};
	}

	VkPipelineShaderStageCreateInfo shader_stages[2]{};
	shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages[0].module = vert_module;
	shader_stages[0].pName = "main";
	shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages[1].module = frag_module;
	shader_stages[1].pName = "main";

	VkVertexInputBindingDescription binding{};
	binding.binding = 0;
	binding.stride = sizeof(DrawVertex);
	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription attributes[3]{};
	attributes[0].location = 0;
	attributes[0].binding = 0;
	attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributes[0].offset = static_cast<u32>(offsetof(DrawVertex, position));
	attributes[1].location = 1;
	attributes[1].binding = 0;
	attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributes[1].offset = static_cast<u32>(offsetof(DrawVertex, uv));
	attributes[2].location = 2;
	attributes[2].binding = 0;
	attributes[2].format = VK_FORMAT_R8G8B8A8_UNORM;
	attributes[2].offset = static_cast<u32>(offsetof(DrawVertex, color));

	VkPipelineVertexInputStateCreateInfo vertex_input{};
	vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input.vertexBindingDescriptionCount = 1;
	vertex_input.pVertexBindingDescriptions = &binding;
	vertex_input.vertexAttributeDescriptionCount = 3;
	vertex_input.pVertexAttributeDescriptions = attributes;

	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineViewportStateCreateInfo viewport_state{};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState blend_attachment{};
	blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blend_attachment.blendEnable = VK_TRUE;
	blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &blend_attachment;

	VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamic_state{};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = 2;
	dynamic_state.pDynamicStates = dynamic_states;

	VkPushConstantRange push_constants{};
	push_constants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	push_constants.offset = 0;
	push_constants.size = sizeof(f32) * 2;

	VkPipelineLayoutCreateInfo layout_info{};
	layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_info.pushConstantRangeCount = 1;
	layout_info.pPushConstantRanges = &push_constants;

	if (vkCreatePipelineLayout(impl.device, &layout_info, nullptr, &impl.pipeline_layout) != VK_SUCCESS) {
		vkDestroyShaderModule(impl.device, frag_module, nullptr);
		vkDestroyShaderModule(impl.device, vert_module, nullptr);
		destroy_swapchain_resources(impl);
		return Error{ErrorCode::VulkanPipelineCreationFailed, "Failed to create Vulkan pipeline layout"};
	}

	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.layout = impl.pipeline_layout;
	pipeline_info.renderPass = impl.render_pass;
	pipeline_info.subpass = 0;

	if (vkCreateGraphicsPipelines(impl.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &impl.pipeline) != VK_SUCCESS) {
		vkDestroyShaderModule(impl.device, frag_module, nullptr);
		vkDestroyShaderModule(impl.device, vert_module, nullptr);
		destroy_swapchain_resources(impl);
		return Error{ErrorCode::VulkanPipelineCreationFailed, "Failed to create Vulkan graphics pipeline"};
	}

	vkDestroyShaderModule(impl.device, frag_module, nullptr);
	vkDestroyShaderModule(impl.device, vert_module, nullptr);

	impl.framebuffers.resize(impl.swapchain_views.size());
	for (usize i = 0; i < impl.swapchain_views.size(); ++i) {
		VkImageView attachments[] = {impl.swapchain_views[i]};

		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = impl.render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = impl.swapchain_extent.width;
		framebuffer_info.height = impl.swapchain_extent.height;
		framebuffer_info.layers = 1;

		if (vkCreateFramebuffer(impl.device, &framebuffer_info, nullptr, &impl.framebuffers[i]) != VK_SUCCESS) {
			destroy_swapchain_resources(impl);
			return Error{ErrorCode::VulkanSwapchainCreationFailed, "Failed to create Vulkan framebuffer"};
		}
	}

	if (!impl.command_buffers.empty()) {
		vkFreeCommandBuffers(
			impl.device,
			impl.command_pool,
			static_cast<u32>(impl.command_buffers.size()),
			impl.command_buffers.data()
		);
		impl.command_buffers.clear();
	}

	if (impl.pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(impl.device, impl.pipeline, nullptr);
		impl.pipeline = VK_NULL_HANDLE;
	}

	if (impl.pipeline_layout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(impl.device, impl.pipeline_layout, nullptr);
		impl.pipeline_layout = VK_NULL_HANDLE;
	}

	impl.command_buffers.resize(impl.framebuffers.size());
	VkCommandBufferAllocateInfo allocate_info{};
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.commandPool = impl.command_pool;
	allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate_info.commandBufferCount = static_cast<u32>(impl.command_buffers.size());

	if (vkAllocateCommandBuffers(impl.device, &allocate_info, impl.command_buffers.data()) != VK_SUCCESS) {
		destroy_swapchain_resources(impl);
		return Error{ErrorCode::VulkanInitFailed, "Failed to allocate Vulkan command buffers"};
	}

	return {};
}

Result<void> VkRenderer::initialize(PlatformWindow& window) {
	if (impl_->initialized) {
		return {};
	}

#ifndef __linux__
	return Error{ErrorCode::NotSupported, "Vulkan renderer currently supports Linux/X11 only"};
#else

	const char* extensions[] = {
		"VK_KHR_surface",
		"VK_KHR_xlib_surface",
	};

	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "FrostUI";
	app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	app_info.pEngineName = "FrostUI";
	app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instance_info{};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledExtensionCount = static_cast<u32>(sizeof(extensions) / sizeof(extensions[0]));
	instance_info.ppEnabledExtensionNames = extensions;

	if (vkCreateInstance(&instance_info, nullptr, &impl_->instance) != VK_SUCCESS) {
		return Error{ErrorCode::VulkanInitFailed, "Failed to create Vulkan instance"};
	}

	auto handle = window.native_handle();
	if (!handle.x11.display || handle.x11.window == 0) {
		shutdown();
		return Error{ErrorCode::VulkanSurfaceCreationFailed, "Invalid X11 native window handle"};
	}

	VkXlibSurfaceCreateInfoKHR surface_info{};
	surface_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surface_info.dpy = reinterpret_cast<Display*>(handle.x11.display);
	surface_info.window = static_cast<Window>(handle.x11.window);

	if (vkCreateXlibSurfaceKHR(impl_->instance, &surface_info, nullptr, &impl_->surface) != VK_SUCCESS) {
		shutdown();
		return Error{ErrorCode::VulkanSurfaceCreationFailed, "Failed to create Vulkan X11 surface"};
	}

	u32 physical_count = 0;
	if (vkEnumeratePhysicalDevices(impl_->instance, &physical_count, nullptr) != VK_SUCCESS || physical_count == 0) {
		shutdown();
		return Error{ErrorCode::VulkanDeviceNotSuitable, "No Vulkan physical devices found"};
	}

	Vector<VkPhysicalDevice> devices(physical_count);
	vkEnumeratePhysicalDevices(impl_->instance, &physical_count, devices.data());

	bool selected_device = false;
	for (const auto& device : devices) {
		u32 queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
		if (queue_family_count == 0) {
			continue;
		}

		Vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

		Option<u32> graphics_index;
		Option<u32> present_index;

		for (u32 index = 0; index < queue_family_count; ++index) {
			if ((queue_families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
				graphics_index = index;
			}

			VkBool32 supports_present = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, index, impl_->surface, &supports_present);
			if (supports_present == VK_TRUE) {
				present_index = index;
			}
		}

		if (graphics_index.has_value() && present_index.has_value()) {
			impl_->physical_device = device;
			impl_->graphics_queue_family = graphics_index.value();
			impl_->present_queue_family = present_index.value();
			selected_device = true;
			break;
		}
	}

	if (!selected_device) {
		shutdown();
		return Error{ErrorCode::VulkanDeviceNotSuitable, "No suitable Vulkan device with graphics and present queues"};
	}

	f32 queue_priority = 1.0f;
	Vector<VkDeviceQueueCreateInfo> queue_infos;

	VkDeviceQueueCreateInfo graphics_queue_info{};
	graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphics_queue_info.queueFamilyIndex = impl_->graphics_queue_family;
	graphics_queue_info.queueCount = 1;
	graphics_queue_info.pQueuePriorities = &queue_priority;
	queue_infos.push_back(graphics_queue_info);

	if (impl_->present_queue_family != impl_->graphics_queue_family) {
		VkDeviceQueueCreateInfo present_queue_info{};
		present_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		present_queue_info.queueFamilyIndex = impl_->present_queue_family;
		present_queue_info.queueCount = 1;
		present_queue_info.pQueuePriorities = &queue_priority;
		queue_infos.push_back(present_queue_info);
	}

	const char* device_extensions[] = {"VK_KHR_swapchain"};

	VkDeviceCreateInfo device_info{};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.queueCreateInfoCount = static_cast<u32>(queue_infos.size());
	device_info.pQueueCreateInfos = queue_infos.data();
	device_info.enabledExtensionCount = 1;
	device_info.ppEnabledExtensionNames = device_extensions;

	if (vkCreateDevice(impl_->physical_device, &device_info, nullptr, &impl_->device) != VK_SUCCESS) {
		shutdown();
		return Error{ErrorCode::VulkanInitFailed, "Failed to create Vulkan device"};
	}

	vkGetPhysicalDeviceMemoryProperties(impl_->physical_device, &impl_->memory_properties);

	vkGetDeviceQueue(impl_->device, impl_->graphics_queue_family, 0, &impl_->graphics_queue);
	vkGetDeviceQueue(impl_->device, impl_->present_queue_family, 0, &impl_->present_queue);

	VkCommandPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = impl_->graphics_queue_family;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(impl_->device, &pool_info, nullptr, &impl_->command_pool) != VK_SUCCESS) {
		shutdown();
		return Error{ErrorCode::VulkanInitFailed, "Failed to create Vulkan command pool"};
	}

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(impl_->device, &semaphore_info, nullptr, &impl_->image_available) != VK_SUCCESS
		|| vkCreateSemaphore(impl_->device, &semaphore_info, nullptr, &impl_->render_finished) != VK_SUCCESS) {
		shutdown();
		return Error{ErrorCode::VulkanInitFailed, "Failed to create Vulkan semaphores"};
	}

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if (vkCreateFence(impl_->device, &fence_info, nullptr, &impl_->in_flight) != VK_SUCCESS) {
		shutdown();
		return Error{ErrorCode::VulkanInitFailed, "Failed to create Vulkan fence"};
	}

	auto size = window.size();
	auto swapchain_result = create_swapchain_resources(*impl_, static_cast<i32>(size.width), static_cast<i32>(size.height));
	if (!swapchain_result) {
		shutdown();
		return swapchain_result.error();
	}

	impl_->initialized = true;
	return {};
#endif
}

Result<void> VkRenderer::render(const DrawList& draw_list, i32 width, i32 height) {
	if (!impl_->initialized) {
		return Error{ErrorCode::VulkanInitFailed, "Vulkan renderer is not initialized"};
	}
	if (width <= 0 || height <= 0) {
		return {};
	}

	if (impl_->swapchain_extent.width != static_cast<u32>(width)
		|| impl_->swapchain_extent.height != static_cast<u32>(height)) {
		vkDeviceWaitIdle(impl_->device);
		destroy_swapchain_resources(*impl_);
		auto recreate_result = create_swapchain_resources(*impl_, width, height);
		if (!recreate_result) {
			return recreate_result.error();
		}
	}

	const auto vertices = draw_list.vertices();
	const auto indices = draw_list.indices();

	const VkDeviceSize vertex_bytes = static_cast<VkDeviceSize>(vertices.size() * sizeof(DrawVertex));
	const VkDeviceSize index_bytes = static_cast<VkDeviceSize>(indices.size() * sizeof(u32));

	if (vertex_bytes > impl_->vertex_capacity) {
		destroy_buffer(*impl_, impl_->vertex_buffer, impl_->vertex_memory, impl_->vertex_capacity);
		auto create_result = create_buffer(
			*impl_,
			std::max<VkDeviceSize>(vertex_bytes, 4096),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			impl_->vertex_buffer,
			impl_->vertex_memory
		);
		if (!create_result) {
			return create_result.error();
		}
		impl_->vertex_capacity = std::max<VkDeviceSize>(vertex_bytes, 4096);
	}

	if (index_bytes > impl_->index_capacity) {
		destroy_buffer(*impl_, impl_->index_buffer, impl_->index_memory, impl_->index_capacity);
		auto create_result = create_buffer(
			*impl_,
			std::max<VkDeviceSize>(index_bytes, 4096),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			impl_->index_buffer,
			impl_->index_memory
		);
		if (!create_result) {
			return create_result.error();
		}
		impl_->index_capacity = std::max<VkDeviceSize>(index_bytes, 4096);
	}

	if (vertex_bytes > 0) {
		void* mapped = nullptr;
		vkMapMemory(impl_->device, impl_->vertex_memory, 0, vertex_bytes, 0, &mapped);
		std::memcpy(mapped, vertices.data(), static_cast<usize>(vertex_bytes));
		vkUnmapMemory(impl_->device, impl_->vertex_memory);
	}

	if (index_bytes > 0) {
		void* mapped = nullptr;
		vkMapMemory(impl_->device, impl_->index_memory, 0, index_bytes, 0, &mapped);
		std::memcpy(mapped, indices.data(), static_cast<usize>(index_bytes));
		vkUnmapMemory(impl_->device, impl_->index_memory);
	}

	vkWaitForFences(impl_->device, 1, &impl_->in_flight, VK_TRUE, UINT64_MAX);
	vkResetFences(impl_->device, 1, &impl_->in_flight);

	u32 image_index = 0;
	VkResult acquire_result = vkAcquireNextImageKHR(
		impl_->device,
		impl_->swapchain,
		UINT64_MAX,
		impl_->image_available,
		VK_NULL_HANDLE,
		&image_index
	);

	if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
		vkDeviceWaitIdle(impl_->device);
		destroy_swapchain_resources(*impl_);
		auto recreate_result = create_swapchain_resources(*impl_, width, height);
		if (!recreate_result) {
			return recreate_result.error();
		}
		return {};
	}
	if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR) {
		return Error{ErrorCode::VulkanSwapchainCreationFailed, "Failed to acquire Vulkan swapchain image"};
	}

	VkCommandBuffer command_buffer = impl_->command_buffers[image_index];
	vkResetCommandBuffer(command_buffer, 0);

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(command_buffer, &begin_info);

	VkClearValue clear_value{};
	clear_value.color = {{0.15f, 0.15f, 0.15f, 1.0f}};

	VkRenderPassBeginInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = impl_->render_pass;
	render_pass_info.framebuffer = impl_->framebuffers[image_index];
	render_pass_info.renderArea.offset = {0, 0};
	render_pass_info.renderArea.extent = impl_->swapchain_extent;
	render_pass_info.clearValueCount = 1;
	render_pass_info.pClearValues = &clear_value;

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	if (!indices.empty() && impl_->pipeline != VK_NULL_HANDLE) {
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<f32>(impl_->swapchain_extent.width);
		viewport.height = static_cast<f32>(impl_->swapchain_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, impl_->pipeline);

		VkBuffer vertex_buffers[] = {impl_->vertex_buffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
		vkCmdBindIndexBuffer(command_buffer, impl_->index_buffer, 0, VK_INDEX_TYPE_UINT32);

		const f32 viewport_size[2] = {
			static_cast<f32>(impl_->swapchain_extent.width),
			static_cast<f32>(impl_->swapchain_extent.height)
		};
		vkCmdPushConstants(command_buffer, impl_->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(viewport_size), viewport_size);

		for (const auto& cmd : draw_list.commands()) {
			if (cmd.index_count == 0) {
				continue;
			}

			i32 sx = static_cast<i32>(cmd.clip_rect.x);
			i32 sy = static_cast<i32>(cmd.clip_rect.y);
			i32 sw = static_cast<i32>(cmd.clip_rect.width);
			i32 sh = static_cast<i32>(cmd.clip_rect.height);

			if (sw <= 0 || sh <= 0 || sw > 1000000 || sh > 1000000) {
				sx = 0;
				sy = 0;
				sw = static_cast<i32>(impl_->swapchain_extent.width);
				sh = static_cast<i32>(impl_->swapchain_extent.height);
			}

			sx = std::clamp(sx, 0, static_cast<i32>(impl_->swapchain_extent.width));
			sy = std::clamp(sy, 0, static_cast<i32>(impl_->swapchain_extent.height));
			sw = std::clamp(sw, 0, static_cast<i32>(impl_->swapchain_extent.width) - sx);
			sh = std::clamp(sh, 0, static_cast<i32>(impl_->swapchain_extent.height) - sy);

			if (sw == 0 || sh == 0) {
				continue;
			}

			VkRect2D scissor{};
			scissor.offset = {sx, sy};
			scissor.extent = {static_cast<u32>(sw), static_cast<u32>(sh)};
			vkCmdSetScissor(command_buffer, 0, 1, &scissor);

			vkCmdDrawIndexed(command_buffer, cmd.index_count, 1, cmd.index_offset, 0, 0);
		}
	}

	vkCmdEndRenderPass(command_buffer);
	vkEndCommandBuffer(command_buffer);

	VkSemaphore wait_semaphores[] = {impl_->image_available};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signal_semaphores[] = {impl_->render_finished};

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	if (vkQueueSubmit(impl_->graphics_queue, 1, &submit_info, impl_->in_flight) != VK_SUCCESS) {
		return Error{ErrorCode::VulkanInitFailed, "Failed to submit Vulkan command buffer"};
	}

	VkSwapchainKHR swapchains[] = {impl_->swapchain};
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = &image_index;

	VkResult present_result = vkQueuePresentKHR(impl_->present_queue, &present_info);
	if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR) {
		vkDeviceWaitIdle(impl_->device);
		destroy_swapchain_resources(*impl_);
		auto recreate_result = create_swapchain_resources(*impl_, width, height);
		if (!recreate_result) {
			return recreate_result.error();
		}
		return {};
	}
	if (present_result != VK_SUCCESS) {
		return Error{ErrorCode::VulkanSwapchainCreationFailed, "Failed to present Vulkan swapchain image"};
	}

	return {};
}

void VkRenderer::shutdown() {
	if (impl_->device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(impl_->device);
	}

	destroy_swapchain_resources(*impl_);
	destroy_buffer(*impl_, impl_->vertex_buffer, impl_->vertex_memory, impl_->vertex_capacity);
	destroy_buffer(*impl_, impl_->index_buffer, impl_->index_memory, impl_->index_capacity);

	if (impl_->in_flight != VK_NULL_HANDLE) {
		vkDestroyFence(impl_->device, impl_->in_flight, nullptr);
		impl_->in_flight = VK_NULL_HANDLE;
	}

	if (impl_->render_finished != VK_NULL_HANDLE) {
		vkDestroySemaphore(impl_->device, impl_->render_finished, nullptr);
		impl_->render_finished = VK_NULL_HANDLE;
	}

	if (impl_->image_available != VK_NULL_HANDLE) {
		vkDestroySemaphore(impl_->device, impl_->image_available, nullptr);
		impl_->image_available = VK_NULL_HANDLE;
	}

	if (impl_->command_pool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(impl_->device, impl_->command_pool, nullptr);
		impl_->command_pool = VK_NULL_HANDLE;
	}

	if (impl_->device != VK_NULL_HANDLE) {
		vkDestroyDevice(impl_->device, nullptr);
		impl_->device = VK_NULL_HANDLE;
	}

	if (impl_->surface != VK_NULL_HANDLE && impl_->instance != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(impl_->instance, impl_->surface, nullptr);
		impl_->surface = VK_NULL_HANDLE;
	}

	if (impl_->instance != VK_NULL_HANDLE) {
		vkDestroyInstance(impl_->instance, nullptr);
		impl_->instance = VK_NULL_HANDLE;
	}

	impl_->physical_device = VK_NULL_HANDLE;
	impl_->graphics_queue = VK_NULL_HANDLE;
	impl_->present_queue = VK_NULL_HANDLE;
	impl_->graphics_queue_family = 0;
	impl_->present_queue_family = 0;
	impl_->swapchain_extent = {0, 0};
	impl_->swapchain_format = VK_FORMAT_UNDEFINED;
	impl_->memory_properties = {};
	impl_->initialized = false;
}

bool VkRenderer::is_initialized() const {
	return impl_->initialized;
}

} // namespace frost

#endif
