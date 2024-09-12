// ����yingobj��ļ���ģ�͹���
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
// ָ��stb�ļ���ͼ����
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>
// ָ��glm�����ֵ��ΧΪvulkan�Ĺ淶��Χ[0,1]
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
// ʹ�� glm��gtx������Ҫ��������glm��ʵ�鹦����չ
#define GLM_ENABLE_EXPERIMENTAL
#include<glm/gtx/hash.hpp>
#include<chrono>
#include<vulkan/vulkan.h>
#include<iostream>
#include<stdexcept>
#include<cstdlib>
// ָ�� Vulkan ʹ�� Windows ƽ̨��չ
#define VK_USE_PLATFORM_WIN32_KHR
// ���� GLFW ���� Vulkan ��ͷ�ļ�
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// ���� GLFW ���� Windows ƽ̨�ض���ͷ�ļ�
#define GLFW_EXPOSE_NATIVE_WIN32
#include<GLFW/glfw3native.h>
#include<vector>
#include<map>
#include<optional>
#include <set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream>
#include <array>
#include <unordered_map>
#include <cmath>
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

const std::string MODEL_PATH = "src/VulkanTest/models/viking_room.obj";
const std::string TEXTURE_PATH = "src/VulkanTest/texture/viking_room.png";


const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif // NDEBUG

// ����������DebugUtilMesengerEXT��չ����
// ��Ϊ����������������չ���ܣ��ǲ����Զ����غ����ģ�
// ���Զ���Ҫʹ��vkGetInstanceProcAddr�����Ҷ�Ӧ�������ĵ�ַ���������ء�
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

}
static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT pDebugMessenger,const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, pDebugMessenger, pAllocator );
}
// ����ͼ�ζ���������չʾ����еĽṹ��
struct QueueFamilyIndices {
	std::optional<uint32_t> grahicsFamily;
	//std::optional<uint32_t> transferFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() const
	{
		return grahicsFamily.has_value() && presentFamily.has_value();
		//return transferFamily.has_value() && presentFamily.has_value();
	}
};



struct Vertex {
	glm::vec3 pos; // �������
	glm::vec3 color; // ��ɫ����
	glm::vec2 texCoord; // ��������
	// �����������������������δӶ��㻺�����ж�ȡ��������
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{
			.binding = 0, // ������
			.stride = sizeof(Vertex), // �󶨵Ķ�������֮��Ĳ���
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX // �������ʣ��˴�Ϊÿ�����㶼�����һ������
		};
		return bindingDescription;
	}
	// ������������������,
	// ָ���󶨶��������е�ÿ����Ա��ÿ����Ա�����ݵ���ʼ��ַ����������
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription() {
		std::array<VkVertexInputAttributeDescription, 3> attriDescription{
			attriDescription[0] = {
				.location = 0, // ����ɫ���ж������������location = index��ͬ
				.binding = 0, // �������������������ݴ����Ǹ��������ݶ�����
				.format = VK_FORMAT_R32G32B32_SFLOAT, // �������Ե���������
				.offset = offsetof(Vertex, pos)
			},
			attriDescription[1] = {
				.location = 1,// �󶨵���ɫ����location������
				.binding = 0,// ����ɫ���ж������������location = index��ͬ
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, color) // �������Ե���������
			},
			attriDescription[2] = {
				.location = 2, // �󶨵���������location����
				.binding = 0, // �󶨵����붥����������
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = offsetof(Vertex, texCoord)
			}
		};
		return attriDescription;
	}
	// ��ʹ�ù�ϣ�����������Vertex��Ϊʱ����ҪVertex�ṹ�ܹ���������ȱȽϲ���
	bool operator== (const Vertex & other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};
// ʹ��ģ���ػ���ʹ��glm��hash������ָ��Vertex��Ա���й�ϣ����
// ͨ��λ�������ٹ�ϣ��ײ
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const{
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

//const std::vector<Vertex> vertices{
//	{{0.0f, -0.5f}, {1.0f, .5f, 0.5f}},
//	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
//	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
//};
// ʹ��������������������
//const std::vector<Vertex> vertices{
//	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
//	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
//	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
//	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
//};

// �����˲�����������Ķ������ݶ���2D model
//const std::vector<Vertex> vertices = {
//	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
//	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
//	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
//};

// 3D model
//const std::vector<Vertex> vertices = {
//	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
//	
//	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//	{{0.5f, -0.5f,-0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
//};
//const std::vector<uint16_t> indices{
//	0,1,2,2,3,0,
//	4,5,6,6,7,4
//};
// 
// ��ͳһ���������󱣴�ģ��-��ͼ-ͶӰת������
struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class HelloTriangleApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:

	/*---- vulkan��ʼ����----*/
	/*---- ʵ������������չ�㡢(��ѡ)������֤�㡢ִ��ʵ���� ----*/
	/*---- �豸��ʼ����ѡ�������豸�������߼��豸 ----*/
	GLFWwindow* window; // ���ڶ���
	VkInstance instance; // ʵ������
	VkDebugUtilsMessengerEXT debugMessenger; // ��֤��ĵ��Թ��ߵ���Ϣ����

	// ��ʼ�������豸���󣬱�ʾ��ǰ�ö���û�з�����Դ���
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;// �߼��豸
	VkQueue graphicsQueue; // �������߼��豸һ���Զ�������������Ҫʹ�ó�Ա��ȡ
	VkQueue presentQueue; // չʾ���������
	// չʾ���������:
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities; // �������Ļ������湦��
		std::vector<VkSurfaceFormatKHR> formats; // �������ı����ʽ����
		std::vector<VkPresentModeKHR> presentModes; // ��������չ��ģʽ
		bool isFormatsAndPresentModesEmpty() const
		{
			return formats.empty() && presentModes.empty();
		}
	};
	VkSurfaceKHR surface; // ���ڱ��洴��
	VkSwapchainKHR swapchain; // ����������
	std::vector<VkImage> swapChainImages; // ������ͼ��
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;// ������ͼ����ͼ����
	std::vector<VkFramebuffer> swapChainFramebuffers;

	// ��Ⱦͨ�����ò���
	VkRenderPass renderPass; // ��Ⱦͨ������
	VkDescriptorSetLayout descriptorSetLayout; // �����������ֶ���
	VkPipelineLayout pipelineLayout; // �ܵ����ֶ���
	VkPipeline graphicsPipeline;	// ͼ�ιܵ�

	// ֡����������
	// �����������
	VkCommandPool commandPool;  // ����ض���
	std::vector<VkCommandBuffer> commandBuffers; // �����������
	// ͬ������
	std::vector<VkSemaphore> imageAvailableSemaphores; // ��ʾͼ������ź�
	std::vector<VkSemaphore> renderFinishedSemaphores; // ��ʾ��Ⱦ�����ź�
	std::vector<VkFence> inFlightFences; // ���ڿ�����Ⱦͨ��һ��ִ��ֻ��Ⱦһ֡

	uint32_t currentFrames = 0; // ��ǰ��Ⱦ��֡
	bool framebufferResized = false; //����ѷ���������С
	// ��ʹ��ģ�ͼ���ʱ�����ǲ���ʹ�ù̶��Ķ�����������ݶ��󣬶��Ƕ�̬����
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer; // ���㻺��������
	VkDeviceMemory vertexBufferMemory; // ʵ�ʶ��㻺�������ڴ����
	VkBuffer indicesBuffer; // ��������������
	VkDeviceMemory indicesBufferMemory; // ʵ���������������ڴ����

	// ����ͳһ����������Ļ��������Լ���Ӧ�ķ����豸�ڴ棬�Լ�ͳһ������������
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	VkDescriptorPool descriptorPool; // ��������
	std::vector<VkDescriptorSet> descriptorSets; // ��������

	VkImage textureImage; // ����ͼ�����
	VkImageView textureImageView; // ����ͼ����ͼ����
	VkDeviceMemory textureImageMemory; // ʵ�ʵ�����ͼ����ڴ����
	VkSampler textureSampler; // �����������

	uint32_t mipLevels;
	VkImage depthImage; // ���ͼ��
	VkImageView depthImageView; // ���ͼ����ͼ
	VkDeviceMemory depthMemory; // ���ͼ���ڴ�

	VkSampleCountFlagBits massSamples = VK_SAMPLE_COUNT_1_BIT; // ����������������
	// ���ö��ز������ͼ�����
	VkImage colorImage; // ͼ�����
	VkImageView colorImageView; // ͼ����ͼ����
	VkDeviceMemory colorImageMemory; // ʵ�ʵ��ڴ����
	void initWindow()
	{
		glfwInit(); // ��ʼ��glfw��
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // ���ô��ڵõ��ͻ���APIΪ��APIģʽ
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // ���ô����ɵ�����С
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr); // ��������
		glfwSetWindowUserPointer(window, this); // ͨ���ú����������뵱ǰ��ʵ����������
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}
	void initVulkan()
	{
		createInstance(); // ����ʵ��
		setupDebugMessenger(); // ���õ�����Ϣ����
		createSurface(); //����չʾ������Ĵ��ڱ���
		// ѡ����ʵ������豸��ѡ����ʵ�ͼ�Ρ���ʾ���������������豸��չ����
		pickPhysicalDevice();
		createLogicalDevice(); // �����߼��豸

		createSwapChain(); // ������������
		createImageView(); // ����ͼ����ͼ
		
		createRenderPass(); // ������Ⱦͨ��
		createDescriptorSetLayout(); // ����������������
		createGraphicsPipline(); // ����ͼ�ι���
		
		createCommandPool(); // ��������ض���

		createColorResources(); // �������ز�����Դ����
		createDepthResources(); // ���������Դ
		createFramebuffers(); //����֡������
		
		createTextureImage(); // ��������ͼ�񣬲�����ͼ�񻺴���
		createTextureImageView(); // ��������ͼ����ͼ
		createTextureSampler(); // ���������������
		loadModel(); // ����ģ������
		createVertexBuffer(); // �������㻺����
		createIndexBuffer(); // ��������������
		
		createUniformBuffer(); // ����ͳһ������
		
		createDescriptorPool(); // ������������
		createDescriptorSets(); // ������������
		createCommandBuffers(); // ���������������
		
		createSyncObject(); // ����ͬ������
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			drawFrame(); // ����֡
		}
		// ��Ϊ���Ʋ������첽�ģ���������ǰ�رմ��ں����豸���ܻ������ִ�С�
		// ����ʹ��vkDeviceWaitIdle���������豸�ȴ��������ִ������������˳�
		vkDeviceWaitIdle(device); 
	}
	// ���ٽ�������ز�������֡���桢ͼ����ͼ������������
	void cleanSwapChain()
	{	
		vkDestroyImageView(device, colorImageView, nullptr);
		vkDestroyImage(device, colorImage, nullptr);
		vkFreeMemory(device, colorImageMemory, nullptr);

		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthMemory, nullptr);

		for (auto framebuffer : swapChainFramebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		for (VkImageView imageView : swapChainImageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(device, swapchain, nullptr);
	}
	void cleanup()
	{	
		// �����������
		cleanSwapChain();
		// ���ٹܵ�
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		// ���ٹܵ�����
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		// ������Ⱦͨ��
		vkDestroyRenderPass(device, renderPass, nullptr);
		// ���ٻ��������ͷ�ӳ���ڴ�
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}
		// �����������أ�ͬʱ���������Զ��ͷ�
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		
		// ���ٲ�������
		vkDestroySampler(device, textureSampler, nullptr);
		// ��������ͼ����ͼ����
		vkDestroyImageView(device, textureImageView, nullptr);
		// ����ͼ�񻺳������ͷŶ�Ӧ������ڴ��ַ
		vkDestroyImage(device, textureImage, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);
		
		// ����������������
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		// ���ٻ�����
		vkDestroyBuffer(device, indicesBuffer, nullptr);
		vkFreeMemory(device, indicesBufferMemory, nullptr);
		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);


		// ɾ���ź�����դ������
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
		// ���������
		vkDestroyCommandPool(device, commandPool, nullptr);


		vkDestroyDevice(device, nullptr); // �����߼��豸
		// ֻ������������֤��֮�󣬲���Ҫ���ٴ����ĵ�����Ϣ����debugMessenger
		if(enableValidationLayers)
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr );

		vkDestroySurfaceKHR(instance, surface, nullptr); // ���ٴ��ڱ������
		vkDestroyInstance(instance, nullptr); // ������ʵ��
		glfwDestroyWindow(window); // ���ٴ���
		glfwTerminate(); // glfw�����ն�

	}

	// �ؽ�������
	void recreateSwapChain()
	{	
		// ��С������
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(device);
		cleanSwapChain(); // ������ʹ��֮ǰ�ľɶ���

		createSwapChain();
		createImageView();
		createColorResources();
		createDepthResources();
		createFramebuffers();
	}

	// ��鵱ǰϵͳ��Vulkan�Ƿ�֧����֤��
	bool checkValidationLayerSupport()
	{	// ������ȡʵ�������������
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> avaliableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, avaliableLayers.data());
		//std::cout << "avaliable layer:\n";
		//for (const auto& layer : avaliableLayers)
		//{
		//	std::cout << "\t" << layer.layerName << "\n";
		//}
		// ����Ƿ���õ�ʵ�����������Ƿ��������֤��
		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;
			for (const auto& layerProperties : avaliableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}
			if (!layerFound)
				return false;
		}

		return true;
	}
	// ��GLFW���л�ȡVulkan��չ��������չ�б�,�������Ƿ�������֤�㣬����������ʱ���յ�����Ϣ��
	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		// ʹ��glfwGetRequiredInstanceExtensions������ȡ��չ���б������
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		// ���캯������һ��������ַ�����һ��������ַ��
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		// ��������֤��ʱ����Ҫָ��������֤�������չ
		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	void createInstance() {

		// ��֤�����ò�����Ƿ�֧����֤��
		if (enableValidationLayers && !checkValidationLayerSupport())
			throw std::runtime_error("validation layers requested, but not avaliable!");

		// ����ʵ��ǰ����������Ӧ�ó�����Ϣ�Ľṹ�壺
		//	�����ýṹ�����ͣ�Ӧ�ó�������/�汾����������/�汾��vulkan api�汾
		// �Ա��Ż����ǵ��ض�Ӧ�ó���
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0; // ���ߺ���ʵ���㱾Ӧ�ó�������֧�ֵ����Vulkan API�汾
		
		// ����ʵ����˵�����ͣ���ֵ�õ�Ӧ�ó��򴴽��Ĳ�����Ϣ
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// ����ʵ����������Ϣ�ṹ�����չ�б����
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		// ����������Ϣ����׼��������֤��
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
			// �������Ե���Ϣ�ṹ��
			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		// ����ʵ����һ����˵Vulkan�Ĵ����������Ĳ�����ʽΪ(��������Ϣ�ṹ���ָ��, �ص�ָ��, ���������ָ��)
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create instance");

	}

	QueueFamilyIndices findQueueFimalies(VkPhysicalDevice physicalDevice)
	{
		// ����ͼ�ζ��е������ʹ洢���ж��е����Զ���
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		QueueFamilyIndices indices = {}; // ͼ�ζ��к�չʾ��������
		int i = 0;
		VkBool32 presentSupport = false;
		// ���������豸�Ƿ���֧��ͼ�β����ͱ��滺�����Ķ�����
		for (const auto& queue : queueFamilies)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
			if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.grahicsFamily = i;
			}
			if (presentSupport)
				indices.presentFamily = i;
			if (indices.isComplete())
				break;
			++i;
		}
		return indices;
	}
	// ��ȡ�����豸֧�ֵĹ��ܣ������ָ������չ�����Ƿ�֧�֡�
	bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
	{
		// ��������豸���õ���չ���ܣ����洢��������
		uint32_t avaliableExtensionsCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &avaliableExtensionsCount, nullptr);
		std::vector<VkExtensionProperties> avaliableExtensions(avaliableExtensionsCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &avaliableExtensionsCount, avaliableExtensions.data());
		// ���������չ�������õ�set�����У�������ͨ��ѭ������������չ�������飬
		// ������Ƿ��������������չ���ܡ�
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
		for (const auto& extension : avaliableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}


	// ����豸��Capabilities��SurfaceFormat��PresentModes
	SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice physicalDevice)
	{
		SwapChainSupportDetails details;
		// ��ѯ�����豸�Ļ�������
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
		// ��ѯ�����豸�ı����ʽ����
		uint32_t formatsCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, nullptr);
		if (formatsCount != 0)
		{
			details.formats.resize(formatsCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, details.formats.data());
		}
		// ��ѯ�����豸��չʾģʽ����
		uint32_t presentModesCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr);
		if (presentModesCount != 0)
		{
			details.presentModes.resize(presentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, details.presentModes.data());
		}
		return details;
	}

	// ���ѡ��ʹ�ú��ʵ������豸
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice)
	{
		// ��ѯ�豸�Ƿ����ָ�����ܶ��У�����ȡ��������
		QueueFamilyIndices indices = findQueueFimalies(physicalDevice);
		// ����豸����չ����֧�����
		bool deviceExtensionsSupported = checkDeviceExtensionSupport(physicalDevice);
		// ��ѯ�豸�Ƿ�֧�ָ������Բ���
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
		// ��ѯ�豸����������ǰ���������ԡ������ʽ������ģʽ�Ƿ�֧��
		bool deviceSwapChainAdequate = false;
		if (deviceExtensionsSupported)
		{
			SwapChainSupportDetails details = querySwapchainSupport(physicalDevice);
			deviceSwapChainAdequate = details.isFormatsAndPresentModesEmpty();

		}
		return indices.isComplete() && deviceExtensionsSupported && (!deviceSwapChainAdequate) && deviceFeatures.samplerAnisotropy;
	}

	// ʹ��isDeviceSuitable������ɸѡ��ѡ�������豸
	void pickPhysicalDevice()
	{
		// ���������豸
		// �õ�����������
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		if (physicalDeviceCount == 0)
			throw std::runtime_error("Not Found the physical device with Vulkan support!");
		// ��������豸�����������
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
		// ����Ƿ���ͼ�ζ��е������豸
		for (const auto& device : physicalDevices)
		{
			if (isDeviceSuitable(device))
			{
				physicalDevice = device;
				massSamples = getMaxUsableSampleCount(); // ��ȡ�����豸֧�ֵ���ɫ����Ȳ�����֮��
				break;
			}
		}
		if(physicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Failed to find a suitable GPU!");
	}

	// ��֤����Ϣ�Ļص�����
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		// �������س̶Ⱥ����;�����δ�����Ϣ
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			std::cerr << "validation layer warning:\t" << pCallbackData->pMessage << std::endl;
		}
		else
		{
			std::cerr << "validation layer info:\t" << pCallbackData->pMessage << std::endl;
		}

		return VK_FALSE;
	}
	// �����Ϣ���Զ������Ϣ�ṹ��
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}
	// ������Ϣ���Զ���
	void setupDebugMessenger()
	{
		if (!enableValidationLayers) return;
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		populateDebugMessengerCreateInfo(debugCreateInfo);
		if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
			throw std::runtime_error("Failed to create debuge messenger!");
	}
	void createLogicalDevice() 
	{	
		// ��ȡ����������
		QueueFamilyIndices indices = findQueueFimalies(physicalDevice);
		// �������������飬����ͼ�β�����չʾ�����Ķ���
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { // ��ͼ�ζ��к�չʾ���е�������������,˳��ȥ��
			indices.grahicsFamily.value() , indices.presentFamily.value() 
		};
		// ���øö��е����ȼ�
		float queuePriority = 1.0f;
		// ���������壬��ȡ���ж����壬��䵽����������
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; // ��ʶ�ýṹ������
			queueCreateInfo.queueFamilyIndex = queueFamily; // ����������
			queueCreateInfo.queueCount = 1; // ���и���
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}
		
		VkPhysicalDeviceFeatures deviceFatures{}; //��ѯ�����豸����
		deviceFatures.samplerAnisotropy = VK_TRUE;
		deviceFatures.sampleRateShading = VK_TRUE;
		// �����߼��豸����Ϣ�ṹ��
		VkDeviceCreateInfo deviceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()), // �趨ѡ��Ķ����������Ϣ
			.pQueueCreateInfos = queueCreateInfos.data(),
			.pEnabledFeatures = &deviceFatures // �߼��豸�����������豸��ͬ�Ĺ���
		};
		// ͨ�����pickuPphysicalDevice�����������ڽ���������ָ����չ���ܺ���֤��
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		
		// ������֤���־�����������߼��豸�Ƿ������֤����Ϣ
		if (enableValidationLayers)
		{
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			deviceCreateInfo.enabledLayerCount = 0;
		}
		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
			throw std::runtime_error("Failed to create logical device!");
		// ��ȡ���߼��豸һ�𴴽���ͼ�ζ��о�����Է��������ͼ�β����Ķ���
		vkGetDeviceQueue(device, indices.grahicsFamily.value(), 0, &graphicsQueue);
		// ��ȡ֧��չʾ�����Ķ��о�����Է������չʾ�����Ķ���
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	/*---- �˴���ʼչʾ�㹹�� ----*/


	void createSurface() // surfaceͬ�߼��豸����ʱ�Զ����أ��˴�ʹ��glfwCreateSurface��������
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create window surface!");
	}
	// ѡ������ʽ��format����ָ�����͵���ɫͨ����colorspace����ָ�����͵���ɫ�ռ�
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && 
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}
		return availableFormats[0];// һ����˵��������ָ������ʱ������ѡ���һ����ʽ����ʹ��
	}
	// ѡ��ͼ���������
	VkPresentModeKHR chooseSwapSurfacePresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availablePresentMode;
		}
		return VK_PRESENT_MODE_FIFO_KHR;// һ����˵��������ָ������ʱ������ѡ��FIFO����ͼ������
	}
	// ͼ��Ļ������ԣ���߶Ƚ���ѡ��
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilites)
	{
		if (capabilites.currentExtent.width != uint32_t(std::numeric_limits<uint32_t>::max ))
		{
			return capabilites.currentExtent;
		}
		else
		{
			int width = 0 , height = 0;
			glfwGetFramebufferSize(window, &width, &height);
			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};
			actualExtent.height = std::clamp(actualExtent.height, capabilites.minImageExtent.height, capabilites.maxImageExtent.height);
			actualExtent.width = std::clamp(actualExtent.width, capabilites.minImageExtent.width, capabilites.maxImageExtent.width);
			return actualExtent;
		}
	}

	// ����������
	void createSwapChain()
	{	
		// ȷ������������ԣ���ѡ�����ֵ
		SwapChainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice);
		VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapSurfacePresentMode(swapchainSupport.presentModes);
		// ���ý������е�ͼ����������ҷ�ֹ�趨ͼ���������
		uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
		if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
			imageCount = swapchainSupport.capabilities.maxImageCount;
		// ������������Ϣ�ṹ��
		VkSwapchainCreateInfoKHR swapchainCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		// ָ��ÿ��ͼ������Ĳ�����������3DӦ�ó��򿪷�������˴�ӦΪ1
		.imageArrayLayers = 1, 
		// �˴����ǽ�ֱ��ͼ����Ⱦ������ζ�����Ǳ�������ɫ����
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
		};
		// ���������������
		QueueFamilyIndices indices = findQueueFimalies(physicalDevice);
		uint32_t queueFamilyIndices[] = {indices.grahicsFamily.value(), indices.presentFamily.value()};
		// ���ͼ�κ�չʾ���в�ͬ����ô�˴����ݽ̳�ʹ�ò���ģʽ
		if (indices.grahicsFamily != indices.presentFamily)
		{
			// �˴�ָ��ͼ��һ����һ��������ӵ�У���������һ������ʹ����֮ǰ��Ҫ��ʽת������Ȩ
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			// ����ͼ����Կ�������ʹ�ã�������ʽת������Ȩ
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCreateInfo.queueFamilyIndexCount = 0;
			swapchainCreateInfo.pQueueFamilyIndices = nullptr;
		}
		// ָ�����ڽ������е�ͼ���ڳ��ֵ���Ļ֮ǰӦ��Ӧ�õ�Ԥ�任��
		swapchainCreateInfo.preTransform = swapchainSupport.capabilities.currentTransform;
		// ָ�� Alpha ͨ���Ƿ�Ӧ�����봰��ϵͳ�е��������ڻ�ϡ�
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// ָ���������ĳ���ģʽ
		swapchainCreateInfo.presentMode = presentMode;
		// ָ����ͼ�񳬹����ڱ߽�ʱ�Ƿ�Ӧ�ü�
		swapchainCreateInfo.clipped = VK_TRUE;
		// �������´���������ʱ�ĳ�ʼ�����˴�ΪNULL_HANDLE����Ϊ��һ�δ���
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS)
			throw std::runtime_error("failed to create swap chain!");
		
		// ��ȡ������ͼ����
		uint32_t imagesCount = 0;
		vkGetSwapchainImagesKHR(device, swapchain, &imagesCount, nullptr);
		if (imagesCount != 0)
		{
			swapChainImages.resize(imagesCount);
			vkGetSwapchainImagesKHR(device, swapchain, &imagesCount, swapChainImages.data());
		}
		// ����ͼ��ĸ�ʽ��ͼ���С
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}
	// ��������ͼ����ͼ
	void createTextureImageView()
	{
		textureImageView = createImageView(textureImage, mipLevels,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	}
	// ����ͼ����ͼ
	void createImageView()
	{
		swapChainImageViews.resize(swapChainImages.size());
		for (uint32_t i = 0; i < swapChainImages.size(); ++i)
		{
			swapChainImageViews[i] = createImageView(swapChainImages[i], 1,
				swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}
	// ����ͼ����ͼ��ͨ�ú���
	VkImageView createImageView(VkImage image,uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		// ָ�򽻻���ͼ��
		imageViewInfo.image = image;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // ����ͼ����
		imageViewInfo.format = format; // ��ͼ��ʽ��ͼ���ʽ��ͬ
		// ָ����ɫ���ͨ������
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		// ����ͼ�����Щ���汻������ͼ����ͼ�С������mipmap�㼶
		imageViewInfo.subresourceRange.aspectMask = aspectFlags;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = mipLevels;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		VkImageView imageView;
		if (vkCreateImageView(device, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image views!");
		return imageView;
	}
	/*---- �˴���ʼͼ�ι��ߵĹ��� ----*/
	// ��ɫ��ģ��
	// ��ȡ��ɫ��ģ��Ĵ����ļ����ݲ�����
	static std::vector<char> readFile(const std::string& filename) 
	{	
		// std::ios::ate:���ļ�ʹ�����ֲ�ļ�β��
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open())
			throw std::runtime_error("Failed to open the file!");
		size_t filesize = (size_t)file.tellg(); // ��ȡ��ǰ�ļ���ȡλ�õ�ƫ�������෴�������ʹ�õ�tellp()
		std::vector<char> buffer(filesize); // �����ļ����ݻ�����
		file.seekg(0); // �ƶ���������ݵĿ�ͷ
		file.read(buffer.data(), filesize); // ��ȡ�ļ�
		file.close();// �ر��ļ�
		return buffer;
	}
	// ���ݴ��ݵ���ɫ�����봴����ɫ��ģ�����VkShaderModule
	VkShaderModule createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = code.size();
		// ��Ҫע��reinterpret_cast�ǵײ��ڴ���������������ټ��Ǳ��Σ�ա�
		// ��ΪcodeΪvector���ͣ������ݽ���Ѿ����˶��������
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
			throw std::runtime_error("Failed to creat shader module!");
		return shaderModule;
	}
	// ������Ⱦͨ��
	void createRenderPass() 
	{
		// ��Ӹ��������������ø�����ʽ��������ʽ�����ڴ�ļ��غʹ洢��ʽ��ģ�建�����ļ��غʹ洢��ʽ�������������Ⱦͼ�񲼾ַ�ʽ
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat; // ��ɫ�����ĸ�ʽӦ�뽻����ͼ��ĸ�ʽƥ��
		colorAttachment.samples = massSamples;
		// ����Ⱦǰ����Ⱦ����δ������е�����
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // �ڿ�ʼʱ��ֵ���Ϊ����
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // ��Ⱦ���ݽ��洢���ڴ��У��Ժ��ȡ
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // ��ʹ��ģ�建��������������
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// ���ó�ʼͼ�񲼾ֺ���Ⱦ�����ͼ�񲼾�
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // ��ʼʱ������ͼ�񲼾�
		// ��Ϊʹ���˶��ز�������ʱ����ɫ������������ͨ����ɫ���������ǽ������ղ��ָ�Ϊ��ɫ����
		// �����ᴴ��������ɫ��������
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// ��ͨ���͸������ã����õĸ������������ø�����ͼ�񲼾�����
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; // �����������������Ϊ����ֻ������һ����������������Ϊ0
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // ��ʾ��������Ϊ��ɫ����������

		// �����ȸ���
		VkAttachmentDescription depthAttachment{
			.format = findDepthFormat(),
			.samples = massSamples,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // �����Ĵ洢��Ȳ������ڻ�����ɺ���ʹ��
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // ������֮ǰ����ȶ��������
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};
		VkAttachmentReference depthAttachmentRef{
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};


		// ������ɫ�����Ľ�������������ת��Ϊ���渽��
		VkAttachmentDescription colorAttachmentResolve{
			.format = swapChainImageFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, 
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR // �˴�ͼ�����ղ��ֱ���Ϊչʾ������Դ
		};
		VkAttachmentReference colorAttachmentResolveRef{
			.attachment = 2,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		// ������ͨ����ָ����ͨ�����ͣ����ø���������ָ����������
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // ����Ϊ����ͼ����ͨ��
		subpass.colorAttachmentCount = 1; // ��������
		subpass.pColorAttachments = &colorAttachmentRef; // ��ɫ��������
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		// ������ͬ��ͨ��֮��������ԣ�����֮�����Э����
		VkSubpassDependency dependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL, // ָ��������ϵ��Դ��ͨ������ʾ��������Ⱦͨ��֮���ĳ���ⲿ������
			.dstSubpass = 0, // ������ϵ��Ŀ����ͨ��
			// ָ��Դ��ͨ���е���Щ�ܵ��׶Σ�pipeline stages������˲�����
			// ָ�������ɫ��������׶κ�Ƭ�β��Ժ�׶�
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
				| VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT 
				| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT // ָ��Դ��ͨ���е���Щ������������˲�����
		};
		// ָ��Ŀ����ͨ���е���Щ�ܵ��׶���Ҫ����Դ��ͨ�������ݡ�
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 
			| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		// ָ��Ŀ����ͨ���е���Щ����������Ҫ����Դ��ͨ�������ݡ�
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT 
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		// �����и������Ϊ����
		std::array< VkAttachmentDescription, 3> attachments{ colorAttachment, depthAttachment, colorAttachmentResolve };

		// ��Ⱦͨ���Ĵ�����ָ����������ͨ��, ��ͨ��������
		VkRenderPassCreateInfo renderPassCreateInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = static_cast<uint32_t>(attachments.size()),
		.pAttachments = attachments.data(),
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency
		};
		if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
			throw std::runtime_error("Failed to create renderpass!");
		
	}

	// ����ͼ�ι��ߵ����庯��
	void createGraphicsPipline()
	{
		// ��ȡ�����Ƭ����ɫ����������
		auto vertShaderCode = readFile("src/VulkanTest/Shaders/vert.spv");
		auto fragShaderCode = readFile("src/VulkanTest/Shaders/frag.spv");
		// ��������Ϊ����������Ӧ��ɫ��ģ��
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
		// ����������ɫ��ģ�����Ϣ�ṹ��
		VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
		vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		// ָ������ɫ���ڹ�����ɫ�����ĸ�����׶�
		vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageCreateInfo.module = vertShaderModule;
		vertShaderStageCreateInfo.pName = "main"; // ָ����ɫ�����õĺ���(��ڵ�)
		// ����Ƭ����ɫ��ģ�����Ϣ�ṹ��
		VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
		fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		// ָ������ɫ���ڹ�����ɫ�����ĸ�����׶�
		fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageCreateInfo.module = fragShaderModule;
		fragShaderStageCreateInfo.pName = "main"; // ָ����ɫ�����õĺ���(��ڵ�)

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageCreateInfo, fragShaderStageCreateInfo };

		// �̶�����
		// ��̬״̬Dynamic State�������ӿںͲü�����
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR // ���޸Ĳü����δ�С
		};
		VkPipelineDynamicStateCreateInfo dynamicCreateInfo{};
		dynamicCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicCreateInfo.pDynamicStates = dynamicStates.data();

		// ��ȡ�������ݺ���δ�������(�õ����е�ʵ�ʴ����Ա�磺vertex&color)
		auto bindingDescription = Vertex::getBindingDescription();
		auto attriDescription = Vertex::getAttributeDescription();
		// ��������Vertex Input
		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
		vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attriDescription.size());
		vertexInputCreateInfo.pVertexAttributeDescriptions = attriDescription.data();
		
		// ������װInput Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
		inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
		// �ӿںͲü�Viewport & scissors����Ҫ�ڶ�̬״̬����
		// ��Ҫ�ڹܵ�����ʱ��ָ���ӿںͲü����εļ���
		VkPipelineViewportStateCreateInfo viewportCreateInfo{};
		viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportCreateInfo.viewportCount = 1;
		viewportCreateInfo.scissorCount = 1;
		// ��Ϊ��̬���ã�����Ҫָ���ӿںͲü����εĽṹ��
		//viewportCreateInfo.pViewports = &viewport;
		//viewportCreateInfo.pScissors = &scissor;
		// ��դ����Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizeCreateInfo{};
		rasterizeCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizeCreateInfo.depthClampEnable = VK_FALSE; // ������ƣ�����ʱ������nearplan��farplane��Ƭ�β���̶�
		rasterizeCreateInfo.rasterizerDiscardEnable = VK_FALSE; // ����ʱ������ͼ����Զ��ͨ����դ���׶�
		rasterizeCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // �������ɷ�ʽ
		rasterizeCreateInfo.lineWidth = 1.0f; // �߶ο��
		rasterizeCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; // ���޳���ʽ
		rasterizeCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // ���涥��˳��
		rasterizeCreateInfo.depthBiasEnable = VK_FALSE; // б��ƫ��

		// ���ز���multiSampling
		VkPipelineMultisampleStateCreateInfo multismapleCreateInfo{};
		multismapleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multismapleCreateInfo.sampleShadingEnable = VK_TRUE; // ����������ɫ?
		multismapleCreateInfo.rasterizationSamples = massSamples; // ÿ�����ص�������
		multismapleCreateInfo.minSampleShading = .2f;  // ÿ�����ص���С������ɫ����,1.0fָ���������ض���Ҫ��������ɫ
		multismapleCreateInfo.pSampleMask = nullptr; // ��Щ������Ч
		multismapleCreateInfo.alphaToCoverageEnable = VK_FALSE; // Alpha��Coverage
		multismapleCreateInfo.alphaToOneEnable = VK_FALSE; // Alpha��One
		// ���ģ��״̬
		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE, //������Ȳ��ԣ���Ƭ�ε����ֵ����Ȼ��������бȽ�
			.depthWriteEnable = VK_TRUE, // ��������Ȳ���ʱ��ȷ���Ƿ�ͨ�����Ե���Ƭ�����ֵд����Ȼ��������и���
			.depthCompareOp = VK_COMPARE_OP_LESS, // Ƭ�����ֵ����Ȼ�����ֵ��αȽ�
			.depthBoundsTestEnable = VK_FALSE, // �����ã���ֻ�ᱣ����min~max����֮���Ƭ��
			.stencilTestEnable = VK_FALSE, // ����ģ�����
			.front = {},
			.back = {},
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 1.0f,
		};

		// ��ɫ���Color blending
		// ������ɫ������ָ��ÿ����ɫ�����Ļ����Ϊ������ɫ/͸���Ȼ������,��ɫ/͸���Ȼ�Ϸ�ʽ
		VkPipelineColorBlendAttachmentState colorBlendAttachment{
			.blendEnable = VK_FALSE,// ��������ɫ���
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};
		// ��������֡�������Ľṹ����
		VkPipelineColorBlendStateCreateInfo colorBlending{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
			.blendConstants = 0.0f
		};

		// �ܵ����֣������ܵ����������������ν���
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1, // ������������
		.pSetLayouts = &descriptorSetLayout, // �������������������ָ��
		.pushConstantRangeCount = 0, // ���볣����Χ������
		.pPushConstantRanges = nullptr // ָ�����볣����Χ�����ָ��
		};
		if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create pipeline layout!");
		// ����ͼ�ι���
		VkGraphicsPipelineCreateInfo pipelineInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStages, // ��ɫ���׶�Ŀ��
		// �̶�����
		.pVertexInputState = &vertexInputCreateInfo,
		.pInputAssemblyState = &inputAssemblyCreateInfo,
		.pViewportState = &viewportCreateInfo,
		.pRasterizationState = &rasterizeCreateInfo,
		.pMultisampleState = &multismapleCreateInfo,
		.pDepthStencilState = &depthStencilCreateInfo,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicCreateInfo,
		// �ܵ�����
		.layout = pipelineLayout,
		.renderPass = renderPass, // ָ����Ⱦͨ������
		.subpass = 0, // ָ����ͨ������
		.basePipelineHandle = VK_NULL_HANDLE, // �����ܵ��������¹ܵ�ʱ��ָ�����йܵ��ľ��
		.basePipelineIndex = -1, // ͨ��������������һ���ܵ�
		};
		if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
			throw std::runtime_error("failed to create graphics pipeline!");
		// ��ʹ������ɫ������Ҫ�������
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
	}
	// ����ͳһ�����������Լ���صĻ�����������Ļ�����ӳ���豸�ڴ桢
	void createUniformBuffer()
	{
		VkDeviceSize size = sizeof(UniformBufferObject); // ����ͳһ����������
		// ����ض���ָ����С(ͬ֡���)
		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{	
			// �������������󣬲������豸�ڴ棬�������󶨷�����豸�ڴ����
			createBuffer(
				size, 
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[i], uniformBuffersMemory[i]);
			// ӳ���ڴ�
			vkMapMemory(device, uniformBuffersMemory[i], 0, size, 0, &uniformBuffersMapped[i]);
		}

	}
	// ����������������
	void createDescriptorSetLayout()
	{
		// ������������ָ����UBO����
		VkDescriptorSetLayoutBinding uboLayoutBinding{
			.binding = 0, // ָ������Դ��binding number
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // ������������
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT, // ָ����ɫ���׶α�־λ
			.pImmutableSamplers = nullptr // ͼ�������ص�������
		};
		// �������������ð󶨶���Ϊ��ϲ���ͼ��
		VkDescriptorSetLayoutBinding samplerLayoutBinding{
			.binding = 1, // ���binding��Ӧ��glsl������location(binding=value)ֵһ��
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // ���ý׶�ΪƬ����ɫ��
		};
		// �����õ����������ֶ��������������
		std::array < VkDescriptorSetLayoutBinding,2> bindings = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(bindings.size()),
			.pBindings = bindings.data()
		};
		if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create the descriptor set layout!");
	}
	// ������������
	void createDescriptorPool()
	{	
		// ָ���������ش�С��ָ�����������ͺ�����������
		std::array<VkDescriptorPoolSize, 2> poolSizes{
			poolSizes[0] = {
				.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = MAX_FRAMES_IN_FLIGHT
			},
			poolSizes[1] = {
				.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = MAX_FRAMES_IN_FLIGHT
			}
		};
		VkDescriptorPoolCreateInfo poolInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = 0,
			.maxSets = MAX_FRAMES_IN_FLIGHT, // ���ɷ�������������
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data() // ָ���������ش�С
		};

		if(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) 
			throw std::runtime_error("failed to create descriptor pool!");
	}
	// ��������������ָ�����յ����������ֺ�������������������֡����ͬ
	// ������������Ҫ��ʾ�ͷţ���������������ʱ���Զ��ͷ�
	void createDescriptorSets()
	{	
		// ����ͬ��������������������
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		// ��������������Ϣ��ָ�����õ��������ء�������������
		VkDescriptorSetAllocateInfo descriptorAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = MAX_FRAMES_IN_FLIGHT, //ͬ֡����ͬ
			.pSetLayouts = layouts.data()
		};
		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if(vkAllocateDescriptorSets(device, &descriptorAllocateInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		// ����֡����ʵ�ʸ���ÿ������������������ʵ����Ϣ
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{	
			// ����ͳһ��������Դ��Ϣ��ָ��ʵ�ʻ����������ݣ��Լ���С��Χ
			VkDescriptorBufferInfo descriptorUBOInfo{
				.buffer = uniformBuffers[i],
				.offset = 0,
				.range = sizeof(UniformBufferObject)
			};
			// ��������ͼ����Դ��Ϣ��ָ��������,����ͼ����ͼ��ͼ�񲼾�
			VkDescriptorImageInfo descriptorImageInfo{
				.sampler = textureSampler,
				.imageView = textureImageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
			// �����õ���Դ������Ϣ��׼��д�뵽����������
			std::array<VkWriteDescriptorSet, 2> writeDescriptorSets{
				writeDescriptorSets[0] =
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = descriptorSets[i],
					.dstBinding = 0, // �󶨵�Ŀ������
					.dstArrayElement = 0, // �������ṹλ����ʱʹ��
					.descriptorCount = 1, // ָ��Ҫ���µ�����Ԫ������
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pImageInfo = nullptr, //��������ͼ�����ݵ�������
					.pBufferInfo = &descriptorUBOInfo, // �������û��������ݵ�������
					.pTexelBufferView = nullptr // �������û�������ͼ��������
				},
				writeDescriptorSets[1] =
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = descriptorSets[i],
					.dstBinding = 1, // �󶨵�Ŀ������
					.dstArrayElement = 0, // �������ṹλ����ʱʹ��
					.descriptorCount = 1, // ָ��Ҫ���µ�����Ԫ������
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = &descriptorImageInfo, //��������ͼ�����ݵ�������
				}
			};
			// �������������е���������Ϣ
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}
	}
	// Drawing�׶�
	// ֡������
	void createFramebuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); ++i)
		{
			std::array<VkImageView,3> attachments = {
				colorImageView,
				depthImageView,
				swapChainImageViews[i]
			};
			VkFramebufferCreateInfo framebufferCreateInfo{ 
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = renderPass, // ָ������Ⱦͨ��
				.attachmentCount = static_cast<uint32_t>(attachments.size()),
				.pAttachments = attachments.data(), // ָ��ͼ����ͼ��Ϊ����
				// �˴���Ҫָ��֡�����Ӧ��ͼ����ͼ��һЩ��������
				.width = swapChainExtent.width, // ��ͼ����ͼ�Ŀ�߶���ͬ
				.height = swapChainExtent.height,
				.layers = 1 // ָ��ͼ������Ĳ���
			};
			if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create framebuffer!");
		}
	}
	// �����
	void createCommandPool()
	{	
		// ��ȡ�������Ķ�������ֵ
		QueueFamilyIndices indices = findQueueFimalies(physicalDevice);
		// ָ������ش�����ʽ��ָ��Ķ���������
		VkCommandPoolCreateInfo commandPoolInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = indices.grahicsFamily.value()
		};
		if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
			throw std::runtime_error("failed to create CommandPool!");
	}

	// �����������
	void createCommandBuffers()
	{	
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo commandBufferInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, // ָ�����������Ϊ���������
			.commandBufferCount = (uint32_t)commandBuffers.size()
		};
		if (vkAllocateCommandBuffers(device, &commandBufferInfo, commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to create CommandBuffer!");


	}
	// ����ָ���ڴ����ͺ���Ҫ���ڴ����Ե��ڴ�����
// typeFilter �ʺϻ��������ڴ����͵�λֵ��properties��ʾ�ڴ�����ֵ
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		// ��ȡ�����豸���ڴ�����
		VkPhysicalDeviceMemoryProperties memoryProperty;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperty);
		// ���������豸���ڴ����ԣ����鵽ָ�����ڴ�����
		for (uint32_t i = 0; i < memoryProperty.memoryTypeCount; ++i)
		{
			// ����
			if ((typeFilter & (1 << i)) && ((memoryProperty.memoryTypes[i].propertyFlags & properties) == properties))
				return i;
		}

		throw std::runtime_error("not found the suitable memory!");
	}

	// �������㻺�������󣬲������ڴ�
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		// �����������
		VkBufferCreateInfo bufferInfo{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.flags = 0,
			.size = size, // ָ����������С ��λ�ֽ�
			.usage = usage, // ��������ʹ��Ŀ��(����)
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE // ������ֻ�ܴ�ͼ�ζ����з��ʣ����ö�ռ����ģʽ
		};
		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			throw std::runtime_error("failed to create buffer!");
		// ��ȡ����ʵ��������ڴ��С
		VkMemoryRequirements memRequirement;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirement);
		// ����ʵ�������ڴ��С
		VkMemoryAllocateInfo allocateInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirement.size,
			.memoryTypeIndex = findMemoryType(memRequirement.memoryTypeBits, properties)
		};
		if (vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate buffer memory!");
		// ������Vkbuffer����������ڴ�VkDeviceMemory��
		vkBindBufferMemory(device, buffer, bufferMemory, 0); // ������������������㣬����Ҫ�ܹ���memRequirements.alignment����
	}
	// ��ʼ����ָ���ִ��
	VkCommandBuffer beginSingleTimeCommands()
	{
		// �ڴ洫����ʹ���������ִ�У����������Ҫ����һ����ʱ���������
		VkCommandBufferAllocateInfo commandBufferAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = commandPool,  // ���������
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, // ���ø���������ĵȼ�����
			.commandBufferCount = 1
		};
		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);
		VkCommandBufferBeginInfo commandBufferBeginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};
		// ��ʼ��¼�������
		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
		return commandBuffer;
	}
	// ��������ָ���ִ��
	void endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer); // ������¼
		// �����������¼�������ύ������
		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffer
		};
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		// ʹ�ö��еȴ�����������֤�������ݲ������ϴ����еĲ������
		vkQueueWaitIdle(graphicsQueue);
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}
	// ���ƻ���������
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{	
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		// ׼�����ƻ���������
		VkBufferCopy copyRegion{ 
			.srcOffset = 0,
			.dstOffset = 0,
			.size = size
		};
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		endSingleTimeCommands(commandBuffer);
	}
	// ����ͼ�񻺳���󣬲�����ʵ��ӳ���ڴ����
	void createImage(uint32_t  width, uint32_t  height, uint32_t mipLevels, 
		VkSampleCountFlagBits numSamples, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usages,  
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		// ����ͼ�����
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D, // ��Ϊƽ������
			.format = format,
			.extent = {width, height, 1},
			.mipLevels = mipLevels,  // mipmap�ȼ�
			.arrayLayers = 1, // ��ͼ��Ϊ��������û��ʹ������
			.samples = numSamples,
			.tiling = tiling, // ָ�����ص����з�ʽ������/����˳��
			.usage = usages,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};
		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
			throw std::runtime_error("failed to create image!");
		VkMemoryRequirements memRequirement;
		vkGetImageMemoryRequirements(device, image, &memRequirement);

		VkMemoryAllocateInfo memInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirement.size,
			.memoryTypeIndex = findMemoryType(memRequirement.memoryTypeBits, properties)
		};
		if (vkAllocateMemory(device, &memInfo, nullptr, &imageMemory) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate image memory!");
		vkBindImageMemory(device, image, imageMemory, 0);
	}
	// ת��ͼ�񲼾�, ʹ��ͼ�����Ͻ���ͬ��
	void transitionImageLayout(VkImage image, uint32_t mipLevels, VkFormat format,
		VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.oldLayout = oldLayout, // ��ǰ����
			.newLayout = newLayout, // Ŀ�겼��
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, // ���漰�����干��
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image, // ͼ����
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				// Ӱ���Mipӳ�������������
				.baseMipLevel = 0,
				.levelCount = mipLevels,
				.baseArrayLayer = 0,
				.layerCount = 1, 
			}
		};
		//// ��newLayoutΪģ�帽��ʱ
		//if (newLayout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL)
		//{	// ��Ҫ��ͼ���ض���Χ����ָ���ķ��汻ʹ�ã����λ��ģ��λ
		//	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		//	if (hasStencilComponent(format))
		//		barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		//}
		VkPipelineStageFlags srcStage; // ��ǰ�׶Σ����Ϸ���ǰ����ط������͡��׶�����
		VkPipelineStageFlags dstStage; // Ŀ��׶Σ����Ϸ��������ط������͡��׶�����
		// �ӳ�ʼ���׶ε�����׶�
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{	// src������֮ǰ��dst������֮��
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // ��ǰ�׶Σ�top_of_pipeָ����ˮ�߶����׶�
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Ŀ��׶�Ϊ����
			// �Ӵ���׶ε���ɫ����ȡ�׶�
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			// src������֮ǰ��dst������֮��
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;; // ����׶�д��
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // ��ɫ���׶ν���
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // ��ǰ�׶Σ�top_of_pipeָ����ˮ�߶����׶�
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Ŀ��׶�Ϊ����
		}
		//else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		//{
		//	barrier.srcAccessMask = 0;
		//	barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		//	
		//	srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		//	// reading������Ƭ�γ��ڽ׶Σ�write������Ƭ�κ��ڽ׶�
		//	dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		//}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}
		vkCmdPipelineBarrier( // �ܵ��������ϵ�ָ��
			commandBuffer,
			srcStage, dstStage,
			0,
			0,nullptr,
			0,nullptr,
			1,&barrier
			);
		endSingleTimeCommands(commandBuffer);
	}
	// �����������ݸ��Ƶ�ͼ�������
	void copyBufferToImage(VkBuffer buffer, VkImage image, 
		uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		VkBufferImageCopy region{ // ָ������������Ľṹ
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset = {0, 0, 0},
			.imageExtent = { width, height, 1}
		};
		vkCmdCopyBufferToImage(
			commandBuffer, // ����������
			buffer, // Դ������
			image, // Ŀ��ͼ��
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // ͼ��Ĳ���
			1, // �������������
			&region // �������������
		);
		endSingleTimeCommands(commandBuffer);
	}
	// ����ͼ�����ݣ�����ͼ�񻺴���
	void createTextureImage()
	{	
		// ���������ȸ߶ȡ���������ɫͨ����
		int texWidth, texHeight, texChannel;
		// uc == unsigned char��stbi_load���ĸ�����ָ��ʹ��ͼ��ԭʼ��ͼ��ͨ����
		stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannel, STBI_rgb_alpha);
		VkDeviceSize size = texWidth * texHeight * 4; // STBI_rgb_alphaǿ��ͼ��ʹ��alphaͨ��������һ������ռ��4���ֽ�
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(max(texWidth, texHeight)))) + 1;
		if (!pixels)
			throw std::runtime_error("failed to load texture image!");
		// ������ʱ����������ͼ������
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(size, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			stagingBuffer, stagingBufferMemory);
		// ��������
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, pixels, size);
		vkUnmapMemory(device, stagingBufferMemory);
		stbi_image_free(pixels); // �ͷż��ص�ͼ������
		// ��������ͼ�񻺴棬�豸�����ڴ���󣬲�������а�
		createImage(
			texWidth, texHeight, 
			mipLevels,
			VK_SAMPLE_COUNT_1_BIT,
			swapChainImageFormat, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			textureImage, textureImageMemory);
		// ����ת�� ��״̬->����׶�
		transitionImageLayout(
			textureImage, 
			mipLevels,
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		// ���ƻ��������ݵ�ͼ��
		copyBufferToImage(stagingBuffer, textureImage, texWidth, texHeight);
		// ��Ϊʹ��mipmap��ʽ����
		
		// ����ת�� �ܵ�����׶Ρ�>��ɫ�������׶�
		//transitionImageLayout(
		//	textureImage,
		//	mipLevels,
		//	VK_FORMAT_R8G8B8A8_SRGB,
		//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		// ������ʱ���������ͷŷ������ʱ�ڴ����
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
		generateMipmap(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
	}

	void generateMipmap(VkImage image,VkFormat format, int32_t width, int32_t height, uint32_t mipLevels)
	{
		// �ȼ������ͼ���ʽ�Ƿ�֧�����Թ���
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
			throw std::runtime_error("texture image format does not support linear blitting!");

		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		// ��ʼ�����Ͻṹ��
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};
		// ѭ����������mipmap
		int32_t mipWidth = width, mipHeight = height;
		for(uint32_t i = 1; i < mipLevels; ++i)
		{	// ���㵱ǰmip�ȼ�
			barrier.subresourceRange.baseMipLevel = i - 1;
			// ͼ�񲼾�ת������, ȷ��ִ��blitָ��ǰ��ͼ�񲼾��������ģ�ȷ��ͼ����ΪԴͼ��ɶ�
			// start image layout transition operator:
			// layout: src->dst : transfer_dst -> transfer_src
			// Access: src->dst : transfer_write-> transfer_read 
			// stage : transfer -> transfer
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr, // ���ڴ�����
				0, nullptr, // �����û������ڴ�����
				1, &barrier // ʹ��ͼ���ڴ�����
			);
			// ����blit�������
			// blit����������ͼ��ĸ��Ʋ�����
			// ������mipmapͼ������У�������Ҫ���µ�һ��ͼ��Ŀ�߶�/2
			VkImageBlit blit{
				.srcSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = i - 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
				.srcOffsets = {{0,0,0}, {mipWidth, mipHeight, 1}},
				.dstSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = i,
					.baseArrayLayer = 0,
					.layerCount = 1
				},
				.dstOffsets = {{0,0,0}, {mipWidth >1? mipWidth /2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}}
			};
			vkCmdBlitImage( // blitָ��Ĳ���ת��������ǰ�Ĳ���ת����������
				commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR // �������Թ���
			);


			// ͼ�񲼾�ת������,blitָ��ִ�к�i-1��mip��ͼ����Ϊ������ɫ����ȡ�����������޸�
			// end image layout transition operator:
			// layout: src->dst : transfer_src -> shader_read
			// Access: src->dst : transfer_read-> shader_read 
			// stage : transfer -> shader
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			vkCmdPipelineBarrier(
			commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr, // ���ڴ�����
				0, nullptr, // �����û������ڴ�����
				1, &barrier // ʹ��ͼ���ڴ�����
			);
			// Ϊ��һ��mipmapͼ���߶ȼ������
			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		};
		//�������һ���mipmap����,���һ���mipmapͼ���ܱ����䣬��������ɫ��ֻ��
		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer);
	}
	// ����������
	void createTextureSampler()
	{	
		// ׼����ȡ�豸֧�ֵ����������Ա���ֵ
		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		// ׼�������������Զ���
		VkSamplerCreateInfo samplerInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_LINEAR, // ָ�������Ŵ����ʱ�Ĺ��˷�ʽ
			.minFilter = VK_FILTER_LINEAR, // ָ��������С����ʱ�Ĺ��˷�ʽ
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, // ָ��Mipmap�ļ���ģʽ
			// ������ռ����ͼ���Сʱ������ָ���������ظ���ʽ
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT, // U����ѭ���ظ�
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT, // V����ѭ���ظ�
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT, // w����ѭ���ظ�
			.mipLodBias = 0.0f, // mipmap����LODʱ��ƫ��
			.anisotropyEnable = VK_TRUE, // �����������Թ���
			.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy, // ���������Ա���
			// ���رȽϲ�����������Ӱ��ͼ
			.compareEnable = VK_TRUE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			// mipmap LOD
			.minLod = 0.0f,
			.maxLod = VK_LOD_CLAMP_NONE, // == 1000.0f
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK, // �߽���ɫ
			.unnormalizedCoordinates = VK_FALSE, // �Ƿ�ʹ�÷ǹ�һ������
		};
		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
			throw std::runtime_error("failed to create texture sampler!");
	}

	// �������㻺�������󣬲������ڴ�
	void createVertexBuffer()
	{	
		VkDeviceSize size = sizeof(vertices[0]) * vertices.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// ׼����д��ʵ������
		void* data;
		// ӳ���ڴ棬����CPUֱ��д��ò����ڴ�����data��ʾָ��ӳ���ڴ�ĵ�ַָ��
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, vertices.data(), (size_t)size); // ��������
		vkUnmapMemory(device, stagingBufferMemory); // ȡ��ӳ��
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
		// ��������
		copyBuffer(stagingBuffer, vertexBuffer, size);
		// ���ٴ�������ʱ������
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		// �ͷ���ʱ���������ڴ�
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// �����������������󣬲������ڴ�
	void createIndexBuffer()
	{
		VkDeviceSize size = sizeof(indices[0]) * indices.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		
		// ׼����д��ʵ������
		void* data;
		// ӳ���ڴ棬����CPUֱ��д��ò����ڴ�����data��ʾָ��ӳ���ڴ�ĵ�ַָ��
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, indices.data(), (size_t)size); // ��������
		vkUnmapMemory(device, stagingBufferMemory); // ȡ��ӳ��
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indicesBuffer, indicesBufferMemory);
		// ��������
		copyBuffer(stagingBuffer, indicesBuffer, size);
		// ���ٴ�������ʱ������
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		// �ͷ���ʱ���������ڴ�
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// ��¼�������
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		// �������������ʼ��¼�Ľṹ��
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // ��ѡ�����ʹ�û���������
		beginInfo.pInheritanceInfo = nullptr;
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		// ��Ⱦͨ����ʼ�Ľṹ��
		VkRenderPassBeginInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = renderPass,
			.framebuffer = swapChainFramebuffers[imageIndex], // ָ������������
		};
		// ������Ⱦ����Ĵ�С
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = swapChainExtent;
		// ��ʾ��ɫ���������ģ�帽���ĳ�ʼ���ֵ
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {{0.0f,0.0f, 0.0f,1.0f}};
		clearValues[1].depthStencil = { 1.0, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		// ������Ⱦͨ��
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		// ��������������Ҫָ�����ĸ����͹���ʹ��
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
			0, 1, &descriptorSets[currentFrames], 0, nullptr);
		// ��ͼ�ι���
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		// ��Ϊ�ڹ̶�����һ��ָ�����ӿںͲü�����Ϊdynamic state���˴���Ҫ��ִ������ǰ����
		VkViewport viewport
		{
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(swapChainExtent.width),
			.height = static_cast<float>(swapChainExtent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor
		{
			.offset = {0,0},
			.extent = swapChainExtent
		};
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets );
		vkCmdBindIndexBuffer(commandBuffer, indicesBuffer, 0, VK_INDEX_TYPE_UINT32);
		// ��������������ָ��
		//vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		// ������Ⱦͨ��ִ��
		vkCmdEndRenderPass(commandBuffer);
		// ������¼�������
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to end the command buffer!");
	}
	void updateUniformBuffer(uint32_t currentFrame)
	{
		// ����ʱ��
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		// ���ͳһ�������
		UniformBufferObject ubo{
			.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			.view = glm::lookAt(glm::vec3(2.0f,2.0f,2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f)
		};
		ubo.proj[1][1] *= -1;
		memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
	}
	// ��Ⱦͼ��ͳ��ֽ��
	void drawFrame()
	{	
		// �ȴ�դ��
		vkWaitForFences(device, 1, &inFlightFences[currentFrames], VK_TRUE, UINT64_MAX);
		// ��ȡ����������һ��ͼ�񣬵���������ָ�����õĳ�ʱʱ�䣬������Ϊ��λ
		uint32_t imageIndex; // �ѿ��õĽ�����ͼ������
		VkResult result =  vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrames], VK_NULL_HANDLE, &imageIndex);
		// ��ʾ������������治���ݣ��޷�����Ⱦ��ͨ���������ڴ��ڵ�����С֮��
		if (result == VK_ERROR_OUT_OF_DATE_KHR ){
			recreateSwapChain();
			return;
		}
		// suboptimal��ʾ���ţ��������Կ��Գɹ������ڱ��棬���������Բ�һ����ȫƥ��
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
			}
		// ��������
		updateUniformBuffer(currentFrames);
		// ���ǽ����ӳ�����դ������ʹ��ǰ��Ҫ��դ������
		vkResetFences(device, 1, &inFlightFences[currentFrames]);

		// ʹ��ǰ�������������״̬��ʹ�����¼�¼����
		vkResetCommandBuffer(commandBuffers[currentFrames], 0);
		// ������Ⱦ��ͼ��������׼�����������ʼ��¼
		recordCommandBuffer(commandBuffers[currentFrames], imageIndex);
		// �ύ�������
		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
		};
		// �ȴ��ĸ��ź���
		VkSemaphore waitSemaphore[] = { imageAvailableSemaphores[currentFrames] };
		// �ڹܵ��ĸ��׶εȴ�
		VkPipelineStageFlags waitStage[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphore; // ָ���ȴ����ź���
		submitInfo.pWaitDstStageMask = waitStage; // ָ���ȴ����ĸ��׶�
		// ָ��ʵ��ִ�е��������
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrames];
		// ָ���������ִ����Ϻ�Ҫ�����źŵ��ź���
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrames] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// �ύ��ͼ�ζ���
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrames]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}
		// ����Ⱦ����ύ�ؽ�����׼��չʾ
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		// ���ô���ÿһ֡��ɵ��ź���
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores; // ȷ����ɻ�����ִ�з����źŵ��ź���
		// ���ý�������Ϣ
		VkSwapchainKHR swapChains[] = { swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains; // ָ������ͼ��Ľ�����
		presentInfo.pImageIndices = &imageIndex; // ÿ������������ͼ�������
		// ��ȡÿ���������ĳ��ֽ��
		presentInfo.pResults = nullptr;
		result = vkQueuePresentKHR(presentQueue, &presentInfo);
		// ����Ⱦ�����ύ��չʾ����ʱ��������ͼ���С�޸�
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
		currentFrames = (currentFrames + 1) % MAX_FRAMES_IN_FLIGHT;
	}
	// ����ͬ������
	void createSyncObject()
	{
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fenceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT // �ڵ�һ֡�ĵȴ�ʱ����ֹ��һ֡�ȴ�����ͬ���źţ�������������
		};
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
				|| vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
				|| vkCreateFence(device, &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create semaphore!");
			}
		}
	}
	// ���������Դ����
	void createDepthResources()
	{
		VkFormat depthFormat = findDepthFormat();
		createImage(
			swapChainExtent.width, swapChainExtent.height,
			1, massSamples,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			depthImage, depthMemory
		);
		depthImageView = createImageView(
			depthImage, 
			1,
			depthFormat,
			VK_IMAGE_ASPECT_DEPTH_BIT);
	}
	// ����Ƿ���ָ�����͵ĸ�ʽ
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{	// ��ȡ�����豸�������
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
			// ����Ƿ����ָ�����������з�ʽ��ָ����format���Ե�format��ʽ
			if (tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & features) == features)
				return format;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & features) == features)
				return format;
		}
		throw std::runtime_error("failed to find supported format!");
	}
	// �Ƿ����֧�����/ģ�帽���ĸ�ʽ
	VkFormat findDepthFormat()
	{
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	// �Ƿ����ģ�����
	bool hasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
	// ����ģ������
	void loadModel()
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
			throw std::runtime_error(warn + err);
		// ȥ�ض��㣺����Ԫ�ؽṹΪ��<����,����>
		std::unordered_map<Vertex, uint32_t> uniqueVertices;
		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2],
				};
				vertex.color = { 1.0,1.0, 1.0 };
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
				};
				// ��unordered_map�����в����ڸö��㣬����������У�
				if (!uniqueVertices.count(vertex))
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}

	}

	VkSampleCountFlagBits getMaxUsableSampleCount()
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
		if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
		if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
		if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
		if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
		if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
		return VK_SAMPLE_COUNT_1_BIT;
	}
	// �������ز�����Դ����
	void createColorResources()
	{
		VkFormat colorFormat = swapChainImageFormat;
		createImage(swapChainExtent.width, swapChainExtent.height, 1, massSamples,
			colorFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			colorImage, colorImageMemory);
		colorImageView = createImageView(colorImage, 1, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
};
int main()
{
	HelloTriangleApplication app{};
	try {
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return false;
	}
	return EXIT_SUCCESS;
}