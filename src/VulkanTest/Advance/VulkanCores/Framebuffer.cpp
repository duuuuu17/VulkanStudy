#include "Framebuffer.hpp"

namespace vulkanCore {

	explicit Framebuffer::Framebuffer(
		const Context& context, 
		VkDevice device, VkRenderPass renderPass,
		std::vector<std::shared_ptr<Texture>>& attachments,
		const std::shared_ptr<Texture>& depthAttachment,
		const std::shared_ptr<Texture>& stencilAttachment,
		const std::string& name = "")
		:device_{device}
	{
		std::vector<VkImageView> imageViews;
		for (const auto texture : attachments)
		{
			imageViews.push_back(texture->vkImageView(0));
		}
		if (depthAttachment)
			imageViews.push_back(depthAttachment->vkImageView(0));
		if (stencilAttachment)
			imageViews.push_back(stencilAttachment->vkImageVIew(0));
		assert(!imageViews.empty(),
			"Create a framebuffer with no attachments is not supported");
		const uint32_t widht = !attachments.empty() ?
			attachments[0]->vkExtents().width : depthAttachment->vkExtents().width;
		const uint32_t height = !attachments.empty() ?
			attachments[0]->vkExtents().height : depthAttachment->vkExtents().height;
		const VkFramebufferCreateInfo framebufferInfo{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = static_cast<uint32_t>(imageViews.size()),
			.pAttachments = imageViews.data(),
			.width = widht,
			.height = height,
			.layers = 1
		};
		VK_CHECK(vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &framebuffer_));
		context.setVKObjectname(framebuffer_, VK_OBJECT_TYPE_FRAMEBUFFER,
			"framebuffer: " + name);
	}

	Framebuffer::~Framebuffer()
	{
		vkDestroyFramebuffer(device_, framebuffer_, nullptr);
	}
	VkFramebuffer Framebuffer::vkFramebuffer() const { return framebuffer_; }
}

