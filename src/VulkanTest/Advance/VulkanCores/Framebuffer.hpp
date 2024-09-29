#pragma once

#include <memory>
#include <vector>
#include <vector>
#include <memory>
#include "Common.hpp"
#include "Utility.hpp"
namespace vulkanCore {
	class Context;
	class Texture;

	class Framebuffer final {
	public:
		MOVABLE_ONLY(Framebuffer);

		explicit Framebuffer(const Context& context, VkDevice device, VkRenderPass renderPass,
			std::vector<std::shared_ptr<Texture>>& attachments,
			const std::shared_ptr<Texture>& depthAttachment,
			const std::shared_ptr<Texture>& stencilAttachment,
			const std::string& name = "");
		~Framebuffer();
		VkFramebuffer vkFramebuffer()const;
	private:
		VkDevice device_ = VK_NULL_HANDLE;
		VkFramebuffer framebuffer_ = VK_NULL_HANDLE;
	};
}
