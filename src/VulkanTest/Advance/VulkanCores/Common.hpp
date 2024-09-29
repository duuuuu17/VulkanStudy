#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>
#include <glm/glm.hpp>

#include <cassert>
#define VK_CHECK(func) \
{	\
	VkResult result = func; \
	if(result != VK_SUCCESS)\
	{						\
		std::cerr << "Failed to create some object:" << #func \
		<< "at " << __FILE__ << ":" << __LINE__ \
		<< ". Result is "<< string_VkResult(result) << std::endl; \
	}						\
	assert(false); \
}

namespace vulkanCore{

	constexpr glm::vec4 RENDER_COLOR(1.0f, 0.0f, 0.0f, 1.0f);

    VkImageViewType imageTypeToImageViewType(VkImageType imageType, VkImageCreateFlags flags,
        bool multiview);

	uint32_t bytesPerPixel(VkFormat format);
}