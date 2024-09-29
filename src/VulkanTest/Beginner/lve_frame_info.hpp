#pragma once

// libs 
#include "lve_camera.hpp"
#include "lve_game_object.hpp"
#include <vulkan/vulkan.h>


namespace lve{
	const int MAX_POINT_LIGHTS = 10;
	struct PointLights {
		glm::vec4 lightPosition{ -1.f };
		glm::vec4 lightColor{ 1.f };
	};

	struct GLoableUbo {
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };
		glm::mat4 inverseView{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f }; // w is intensity
		PointLights pointLights[MAX_POINT_LIGHTS];
		int numLights = 0;
	};

	// 使用frameInfo管理每个帧的部分信息
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		LveCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		LveGameObject::Map& globalGameObjects;
	};
}