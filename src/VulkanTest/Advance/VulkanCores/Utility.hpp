#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

// ��ʹ�ÿ������캯���Ϳ�����ֵ����
// ��ʹ���ƶ����캯�����ƶ���ֵ����
#define MOVABLE_ONLY(CLASS_NAME)                     \
  CLASS_NAME(const CLASS_NAME&) = delete;            \
  CLASS_NAME& operator=(const CLASS_NAME&) = delete; \
  CLASS_NAME(CLASS_NAME&&) noexcept = default;       \
  CLASS_NAME& operator=(CLASS_NAME&&) noexcept = default;



namespace util {
	// ���ɹ̶��������ݵĹ�ϣֵ
	uint32_t fnv_hash(const void* key, int len);

	std::vector<char> readFile(const std::string& filePath);
	
	void writeFile(const std::string& filePath,
		const std::vector<char>& fileContents);
	
	std::unordered_set<std::string> filterExtensions(
		std::vector<std::string> availableExtensions,
		std::vector<std::string> requiredExtensions
	);
	// ���part�Ƿ���s�ַ���β��
	int endsWith(const char* s, const char* part);
	// ��϶��ֵ�Ĺ�ϣֵ
	template<typename T, typename...Rest>
	void hash_combine(size_t& seed, const T& v, const Rest&... rest)
	{
		seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hash_combine(seed, rest),...);
	}
}
