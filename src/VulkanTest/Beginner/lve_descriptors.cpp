#include "lve_descriptors.hpp"
#include "lve_utility.hpp"

#include <cassert>
namespace lve {
	// ---- build Setlayout Bindings ----
	LveDescriptorSetLayout::Builder& 
		LveDescriptorSetLayout::Builder::addBinding(
			uint32_t binding, 
			VkDescriptorType descriptorType, 
			VkShaderStageFlags stageFlags,
			uint32_t descriptorCount)
	{
		assert(bindings.count(binding) == 0 && "Binding already in use");
		VkDescriptorSetLayoutBinding bindingInfo{
			.binding = binding,
			.descriptorType = descriptorType,
			.descriptorCount = descriptorCount,
			.stageFlags = stageFlags
		};
		bindings[binding] = bindingInfo;

		return *this;
	}
	std::unique_ptr<LveDescriptorSetLayout> LveDescriptorSetLayout::Builder::build() const
	{	

		return std::make_unique<LveDescriptorSetLayout>(device, bindings);
	}
	// ---- setLayout ----
	LveDescriptorSetLayout::LveDescriptorSetLayout(
		LveDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
		: device(device), bindings(bindings)
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
		for (auto& element : bindings)
			setLayoutBindings.push_back(element.second);
		VkDescriptorSetLayoutCreateInfo setLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
			.pBindings = setLayoutBindings.data(),
		};
		VK_CHECK(vkCreateDescriptorSetLayout(device.device(), &setLayoutInfo, nullptr, &descriptorsetlayout));
	}

	LveDescriptorSetLayout::~LveDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(device.device(), descriptorsetlayout, nullptr);
	}
	LveDescriptorPool::Builder& 
		LveDescriptorPool::Builder::addPoolSize( VkDescriptorType descriptorType, uint32_t descriptorCount)
	{
		poolSizes.push_back({descriptorType, descriptorCount});
		return *this;
	}

	LveDescriptorPool::Builder& LveDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags)
	{
		poolFlags = poolFlags;
		return *this;
	}

	LveDescriptorPool::Builder& LveDescriptorPool::Builder::setMaxSets(uint32_t count)
	{
		maxSets = count;
		return *this;
	}
	std::unique_ptr<LveDescriptorPool> LveDescriptorPool::Builder::build() const 
	{
		return std::make_unique<LveDescriptorPool>(device, maxSets, poolFlags, poolSizes);
	}
	// ---- descriptor pool ----
	LveDescriptorPool::LveDescriptorPool(
		LveDevice& device, 
		uint32_t maxSets, 
		VkDescriptorPoolCreateFlags flags,
		const std::vector<VkDescriptorPoolSize> &poolSizes)
		:device(device)
	{
		VkDescriptorPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = flags,
			.maxSets = maxSets,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};
		VK_CHECK(vkCreateDescriptorPool(device.device(), &poolInfo, nullptr, &descriptorPool));
	}

	LveDescriptorPool::~LveDescriptorPool()
	{
		vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
	}

	// ---- allocation descriptor sets ----
	bool LveDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout,
		VkDescriptorSet& descripotrSet) const 
	{	
		VkDescriptorSetAllocateInfo setsInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &descriptorSetLayout
		};
		// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
		// a new pool whenever an old pool fills up. But this is beyond our current scope
		if(vkAllocateDescriptorSets(device.device(), &setsInfo, &descripotrSet) != VK_SUCCESS)
			return false;
		return true;
	}

	void LveDescriptorPool::freeDescriptor(std::vector<VkDescriptorSet> &descriptorSets) const
	{	
		
		vkFreeDescriptorSets(
			device.device(),
			descriptorPool,
			static_cast<uint32_t>(descriptorSets.size()),
			descriptorSets.data());
	}
	void LveDescriptorPool::resetPool()
	{
		vkResetDescriptorPool(device.device(), descriptorPool, 0);
	}



	LveDescriptorWriter& LveDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
	{	
		// 检查对应的binding
		assert(lveSetlayout.bindings.count(binding) == 1 && "Layout does not contain sepecified binding");
		// 获取DescriptorBinding
		auto& bindingDescriptoin = lveSetlayout.bindings[binding];
		assert(bindingDescriptoin.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");
		
		VkWriteDescriptorSet write{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstBinding = binding,
			.descriptorCount = bindingDescriptoin.descriptorCount,
			.descriptorType = bindingDescriptoin.descriptorType,
			.pBufferInfo = bufferInfo
		};
		writes.push_back(write);
		return *this;
	}
	
	LveDescriptorWriter& LveDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo)
	{
		assert(lveSetlayout.bindings.count(binding) == 1 && "Layout does not contain sepecified binding");
		// 获取DescriptorBinding
		auto& bindingDescriptoin = lveSetlayout.bindings[binding];
		assert(bindingDescriptoin.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstBinding = binding,
			.descriptorCount = bindingDescriptoin.descriptorCount,
			.descriptorType = bindingDescriptoin.descriptorType,
			.pImageInfo = imageInfo
		};
		writes.push_back(write);
		return *this;
	}

	bool LveDescriptorWriter::build(VkDescriptorSet& set)
	{
		bool result = lvePool.allocateDescriptor(lveSetlayout.getDescriptorSetLayout(), set);
		if (!result)
			return false;
		overWrite(set);
		return true;
	}
	void LveDescriptorWriter::overWrite(VkDescriptorSet& set)
	{
		for (auto& write : writes)
		{
			write.dstSet = set;
		}
		vkUpdateDescriptorSets(lvePool.device.device(), static_cast<uint32_t>(writes.size()),
			writes.data(), 0, nullptr);
	}
}