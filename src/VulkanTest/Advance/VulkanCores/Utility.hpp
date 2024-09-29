#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

// 不使用拷贝构造函数和拷贝赋值函数
// 仅使用移动构造函数和移动赋值函数
#define MOVABLE_ONLY(CLASS_NAME)                     \
  CLASS_NAME(const CLASS_NAME&) = delete;            \
  CLASS_NAME& operator=(const CLASS_NAME&) = delete; \
  CLASS_NAME(CLASS_NAME&&) noexcept = default;       \
  CLASS_NAME& operator=(CLASS_NAME&&) noexcept = default;



namespace util {
	// 生成固定长度数据的哈希值
	uint32_t fnv_hash(const void* key, int len);

	std::vector<char> readFile(const std::string& filePath);
	
	void writeFile(const std::string& filePath,
		const std::vector<char>& fileContents);
	
	std::unordered_set<std::string> filterExtensions(
		std::vector<std::string> availableExtensions,
		std::vector<std::string> requiredExtensions
	);
	// 检测part是否在s字符串尾部
	int endsWith(const char* s, const char* part);
	// 组合多个值的哈希值
	template<typename T, typename...Rest>
	void hash_combine(size_t& seed, const T& v, const Rest&... rest)
	{
		seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hash_combine(seed, rest),...);
	}
}
