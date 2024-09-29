#pragma once

#include "lve_device.hpp"

#include <vulkan/vulkan.h>
#include <unordered_map>
#include <vector>
#include <memory>

namespace lve {
	class LveDescriptorSetLayout
	{
	public:

		class Builder {
		public:
			Builder(LveDevice& device) : device(device) {}
			Builder& addBinding(uint32_t binding, 
				VkDescriptorType descriptorType,
				VkShaderStageFlags stageFlags,
				uint32_t descriptorCount = 1);
			std::unique_ptr<LveDescriptorSetLayout> build() const;
		private:
			LveDevice& device;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
		};
		
		LveDescriptorSetLayout(const LveDescriptorSetLayout&) = delete;
		LveDescriptorSetLayout& operator=(const LveDescriptorSetLayout&) = delete;
		LveDescriptorSetLayout(
			LveDevice& device, 
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bingdings);
		~LveDescriptorSetLayout();
		VkDescriptorSetLayout getDescriptorSetLayout() const
		{
			return descriptorsetlayout;
		}
	private:
		LveDevice& device;
		VkDescriptorSetLayout descriptorsetlayout = VK_NULL_HANDLE;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

		friend class LveDescriptorWriter;
	};

	class LveDescriptorPool
	{
	public:

		class Builder {
		public:
			Builder(LveDevice& device) : device(device) {}
			Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t descriptorCount);
			Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
			Builder& setMaxSets(uint32_t count);
			std::unique_ptr<LveDescriptorPool> build() const;
		private:
			LveDevice& device;
			std::vector<VkDescriptorPoolSize> poolSizes;
			uint32_t maxSets = 1000;
			VkDescriptorPoolCreateFlags poolFlags = 0;
		};
	
		LveDescriptorPool(const LveDescriptorPool&) = delete;
		LveDescriptorPool& operator=(const LveDescriptorPool&) = delete;
		LveDescriptorPool(
			LveDevice& device, 
			uint32_t maxSets,
			VkDescriptorPoolCreateFlags flags,
			const std::vector<VkDescriptorPoolSize> &poolSizes);
		~LveDescriptorPool();
		// Õ®π˝√Ë ˆ∑˚≥ÿ…Ë÷√√Ë ˆ∑˚ºØ
		bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout,
			VkDescriptorSet& descripotrSet) const;
		void freeDescriptor(std::vector<VkDescriptorSet> &descriptorSets) const;
		void resetPool();
	private:
		LveDevice& device;
		VkDescriptorPool descriptorPool;
		
		friend class LveDescriptorWriter;
	};

	// update descriptor sets
	class LveDescriptorWriter {
	public:
		LveDescriptorWriter(LveDescriptorSetLayout& setlayout, LveDescriptorPool& pool)
			: lveSetlayout(setlayout), lvePool(pool) {}

		LveDescriptorWriter& writeBuffer(uint32_t binding,
			VkDescriptorBufferInfo* bufferInfo);
		LveDescriptorWriter& writeImage(uint32_t binding,
			VkDescriptorImageInfo* imageInfo);
		bool build(VkDescriptorSet& set);
		void overWrite(VkDescriptorSet& set);
	private:
		LveDescriptorSetLayout& lveSetlayout;
		LveDescriptorPool& lvePool;
		std::vector<VkWriteDescriptorSet> writes;
	};
} // namespace lve


