#pragma once
#include<iostream>
#include <vulkan/vulkan_core.h>
#define VK_CHECK(func) \
do{	\
	VkResult result = func; \
	if(result != VK_SUCCESS) \
	{	\
		std::cerr << "Error using func: "<< #func \
		<< ". File: " << __FILE__ << ". Line:" << __LINE__ << std::endl;\
	}	\
}while(0)
namespace lve {
	template<typename T, typename...Rest>
	void hashCombine(std::size_t& seed,const T& v,const Rest&... rest)
	{
		seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	}
}
