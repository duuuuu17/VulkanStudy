#pragma once

#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_window.hpp"

// std
#include <memory>
#include <vector>
#include <cassert>

namespace lve {
	// encapsulated beginCommandBuffer & endCommandBuffer | CmdBeginRenderPass & CmdEndRenderPass
	class LveRenderer {
	public:

		LveRenderer(LveWindow& window, LveDevice& device);
		~LveRenderer();

		LveRenderer(const LveRenderer&) = delete;
		LveRenderer& operator=(const LveRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const 
		{	return lveSwapChain->getRenderPass(); }
		bool isFrameInProgress() const { return isFrameStarted; }
		float getAspectRatio() const {
			return lveSwapChain->getSwapChainAspectRatio();
		}
		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Connot get command buffer where frame not in progress");
			return commandBuffers[currentFrameIndex];
		}
		int getCurrentFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}
		VkCommandBuffer beginFrames();
		void endFrames();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);


	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		LveWindow& lveWindow;
		LveDevice& lveDevice;
		std::unique_ptr<LveSwapChain> lveSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex = 0;
		int currentFrameIndex = 0;
		bool isFrameStarted = false;
	};
}  // namespace lve