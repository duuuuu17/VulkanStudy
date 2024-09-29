#pragma once

#include "../lve_device.hpp"
#include "../lve_game_object.hpp"
#include "../lve_pipeline.hpp"
#include "../lve_swap_chain.hpp"
#include "../lve_window.hpp"
#include "../lve_renderer.hpp"
#include "../lve_camera.hpp"
#include "../lve_frame_info.hpp"
// std
#include <memory>
#include <vector>

namespace lve {
	// encapsulated pipelineLayout & pipeline
	class PointLight {
	public:

		PointLight(LveDevice& device, VkRenderPass renderPass, 
			VkDescriptorSetLayout descriptorSetLayout);
		~PointLight();

		PointLight(const PointLight&) = delete;
		PointLight& operator=(const PointLight&) = delete;
		void update(FrameInfo& frameInfo, GLoableUbo& ubo);
		void render(FrameInfo& frameInfo);


	private:
		void createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout);
		void createPipeline(VkRenderPass& renderPass);

		LveDevice& lveDevice;
		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
	};
}  // namespace lve