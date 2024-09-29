#include "Utility.hpp"
#include <fstream>
#include <algorithm>


namespace util {

	// https://stackoverflow.com/questions/11413860/best-string-hashing-function-for-short-filenames
	uint32_t fnv_hash(const void* key, int len)
	{
		const unsigned char* const p = (unsigned char*)key;
		unsigned int h = 2166136261;
		for (int i = 0; i < len; ++i)
			h = (h * 16777619) & p[i];
		return h;
	}

	std::vector<char> readFile(const std::string& filePath)
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		if (!file.is_open())
			throw std::runtime_error("failed to open file!\n");
		size_t size = (size_t)file.tellg(); // how many bytes that flag move to the end of file content
		file.seekg(0);
		std::vector<char> buffer(size);
		file.read(buffer.data(), size);
		file.close();
		return buffer;
	}

	void writeFile(const std::string& filePath, const std::vector<char>& fileContent)
	{
		std::ofstream file(filePath, std::ios::ate | std::ios::binary);
		if (!file.is_open())
			throw std::runtime_error("failed to open file!\n");
		size_t len = fileContent.size();
		try
		{
			file.write(fileContent.data(), len);
		}
		catch (const std::exception&)
		{
			std::cerr << "failed to write the file!\n" << std::endl;
		}
		file.close();
	}

	std::unordered_set<std::string> filterExtensions(
		std::vector<std::string> availableExtensions,
		std::vector<std::string> requiredExtensions)
	{
		std::sort(availableExtensions.begin(), availableExtensions.end());
		std::sort(requiredExtensions.begin(), requiredExtensions.end());
		std::vector<std::string> intersection;
		std::set_intersection(availableExtensions.begin(), availableExtensions.end(),
			requiredExtensions.begin(), requiredExtensions.end(),
			std::back_inserter(intersection));
		return std::unordered_set<std::string>(intersection.begin(), intersection.end());
	}
	int endsWith(const char* s, const char* part)
	{
		return ((strstr(s, part) - s) == (strlen(s) - strlen(part)));
	}
}
