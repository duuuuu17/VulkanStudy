// 启用yingobj库的加载模型功能
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
// 指明stb的加载图像功能
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>
// 指明glm的深度值范围为vulkan的规范范围[0,1]
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
// 使用 glm的gtx功能需要声明启用glm的实验功能扩展
#define GLM_ENABLE_EXPERIMENTAL
#include<glm/gtx/hash.hpp>
#include<chrono>
#include<vulkan/vulkan.h>
#include<iostream>
#include<stdexcept>
#include<cstdlib>
// 指定 Vulkan 使用 Windows 平台扩展
#define VK_USE_PLATFORM_WIN32_KHR
// 告诉 GLFW 包含 Vulkan 的头文件
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// 告诉 GLFW 包含 Windows 平台特定的头文件
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

// 创建和销毁DebugUtilMesengerEXT扩展对象。
// 因为这两个功能属于扩展功能，是不会自动加载函数的，
// 所以都需要使用vkGetInstanceProcAddr来查找对应处理函数的地址来主动加载。
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
// 保存图形队列索引和展示层队列的结构体
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
	glm::vec3 pos; // 顶点分量
	glm::vec3 color; // 颜色分量
	glm::vec2 texCoord; // 纹理坐标
	// 顶点输入绑定描述符，描述如何从顶点缓冲区中读取顶点数据
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{
			.binding = 0, // 绑定索引
			.stride = sizeof(Vertex), // 绑定的顶点数据之间的步长
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX // 输入速率，此处为每个顶点都会更新一次数据
		};
		return bindingDescription;
	}
	// 顶点输入属性描述符,
	// 指明绑定顶点数据中的每个成员，每个成员在数据的起始地址和数据类型
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription() {
		std::array<VkVertexInputAttributeDescription, 3> attriDescription{
			attriDescription[0] = {
				.location = 0, // 与着色器中顶点的输入索引location = index相同
				.binding = 0, // 顶点数据索引，及数据处于那个顶点数据对象中
				.format = VK_FORMAT_R32G32B32_SFLOAT, // 描述属性的数据类型
				.offset = offsetof(Vertex, pos)
			},
			attriDescription[1] = {
				.location = 1,// 绑定的颜色坐标location索引号
				.binding = 0,// 与着色器中顶点的输入索引location = index相同
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, color) // 描述属性的数据类型
			},
			attriDescription[2] = {
				.location = 2, // 绑定的纹理坐标location索引
				.binding = 0, // 绑定的输入顶点数据索引
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = offsetof(Vertex, texCoord)
			}
		};
		return attriDescription;
	}
	// 在使用哈希表的容器，将Vertex作为时，需要Vertex结构能够键进行相等比较操作
	bool operator== (const Vertex & other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};
// 使用模板特化来使用glm的hash表，并且指定Vertex成员进行哈希计算
// 通过位移来减少哈希碰撞
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
// 使用索引缓冲区构建矩形
//const std::vector<Vertex> vertices{
//	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
//	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
//	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
//	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
//};

// 加入了采样纹理坐标的顶点数据对象，2D model
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
// 该统一缓冲区对象保存模型-视图-投影转换矩阵
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

	/*---- vulkan初始化：----*/
	/*---- 实例化：遍历扩展层、(可选)插入验证层、执行实例化 ----*/
	/*---- 设备初始化：选择物理设备、创建逻辑设备 ----*/
	GLFWwindow* window; // 窗口对象
	VkInstance instance; // 实例对象
	VkDebugUtilsMessengerEXT debugMessenger; // 验证层的调试工具的消息对象

	// 初始化物理设备对象，表示当前该对象没有分配资源句柄
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;// 逻辑设备
	VkQueue graphicsQueue; // 队列与逻辑设备一起自动创建，我们需要使用成员获取
	VkQueue presentQueue; // 展示层所需队列
	// 展示层所需参数:
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities; // 交换链的基本表面功能
		std::vector<VkSurfaceFormatKHR> formats; // 交换链的表面格式功能
		std::vector<VkPresentModeKHR> presentModes; // 交换链的展现模式
		bool isFormatsAndPresentModesEmpty() const
		{
			return formats.empty() && presentModes.empty();
		}
	};
	VkSurfaceKHR surface; // 窗口表面创建
	VkSwapchainKHR swapchain; // 交换链对象
	std::vector<VkImage> swapChainImages; // 交换链图像
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;// 交换链图像视图对象
	std::vector<VkFramebuffer> swapChainFramebuffers;

	// 渲染通道设置参数
	VkRenderPass renderPass; // 渲染通道对象
	VkDescriptorSetLayout descriptorSetLayout; // 描述符集布局对象
	VkPipelineLayout pipelineLayout; // 管道布局对象
	VkPipeline graphicsPipeline;	// 图形管道

	// 帧缓冲区参数
	// 命令缓冲区参数
	VkCommandPool commandPool;  // 命令池对象
	std::vector<VkCommandBuffer> commandBuffers; // 命令缓冲区对象
	// 同步参数
	std::vector<VkSemaphore> imageAvailableSemaphores; // 表示图像可用信号
	std::vector<VkSemaphore> renderFinishedSemaphores; // 表示渲染结束信号
	std::vector<VkFence> inFlightFences; // 用于控制渲染通道一次执行只渲染一帧

	uint32_t currentFrames = 0; // 当前渲染的帧
	bool framebufferResized = false; //标记已发生调整大小
	// 当使用模型加载时，我们不能使用固定的顶点和索引数据对象，而是动态创建
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer; // 顶点缓存区对象
	VkDeviceMemory vertexBufferMemory; // 实际顶点缓存区的内存对象
	VkBuffer indicesBuffer; // 索引缓存区对象
	VkDeviceMemory indicesBufferMemory; // 实际索引缓存区的内存对象

	// 创建统一缓冲区对象的缓冲区、以及相应的分配设备内存，以及统一缓冲区的数据
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	VkDescriptorPool descriptorPool; // 描述符池
	std::vector<VkDescriptorSet> descriptorSets; // 描述符集

	VkImage textureImage; // 纹理图像对象
	VkImageView textureImageView; // 纹理图像视图对象
	VkDeviceMemory textureImageMemory; // 实际的纹理图像的内存对象
	VkSampler textureSampler; // 纹理采样对象

	uint32_t mipLevels;
	VkImage depthImage; // 深度图像
	VkImageView depthImageView; // 深度图像视图
	VkDeviceMemory depthMemory; // 深度图像内存

	VkSampleCountFlagBits massSamples = VK_SAMPLE_COUNT_1_BIT; // 创建采样次数对象
	// 设置多重采样相关图像对象
	VkImage colorImage; // 图像对象
	VkImageView colorImageView; // 图像视图对象
	VkDeviceMemory colorImageMemory; // 实际的内存对象
	void initWindow()
	{
		glfwInit(); // 初始化glfw库
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // 设置窗口得到客户端API为无API模式
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // 设置床不可调整大小
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr); // 创建窗口
		glfwSetWindowUserPointer(window, this); // 通过该函数将窗口与当前类实例关联起来
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}
	void initVulkan()
	{
		createInstance(); // 创建实例
		setupDebugMessenger(); // 设置调试信息对象
		createSurface(); //创建展示层所需的窗口表面
		// 选择合适的物理设备，选择合适的图形、显示层队列族的索引和设备扩展功能
		pickPhysicalDevice();
		createLogicalDevice(); // 创建逻辑设备

		createSwapChain(); // 创建交换链表
		createImageView(); // 创建图像视图
		
		createRenderPass(); // 创建渲染通道
		createDescriptorSetLayout(); // 创建描述符集布局
		createGraphicsPipline(); // 创建图形管线
		
		createCommandPool(); // 创建命令池对象

		createColorResources(); // 创建多重采样资源对象
		createDepthResources(); // 创建深度资源
		createFramebuffers(); //创建帧缓冲区
		
		createTextureImage(); // 加载纹理图像，并创建图像缓存区
		createTextureImageView(); // 创建纹理图像视图
		createTextureSampler(); // 创建纹理采样对象
		loadModel(); // 加载模型数据
		createVertexBuffer(); // 创建顶点缓冲区
		createIndexBuffer(); // 创建索引缓冲区
		
		createUniformBuffer(); // 创建统一缓冲区
		
		createDescriptorPool(); // 创建描述符池
		createDescriptorSets(); // 创建描述符集
		createCommandBuffers(); // 分配命令缓冲区对象
		
		createSyncObject(); // 创建同步对象
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			drawFrame(); // 绘制帧
		}
		// 因为绘制操作是异步的，当我们提前关闭窗口后，其设备可能还会继续执行。
		// 我们使用vkDeviceWaitIdle函数，让设备等待命令队列执行完成来正常退出
		vkDeviceWaitIdle(device); 
	}
	// 销毁交换链相关参数对象：帧缓存、图像视图、交换链对象
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
		// 清除交换链表
		cleanSwapChain();
		// 销毁管道
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		// 销毁管道布局
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		// 销毁渲染通道
		vkDestroyRenderPass(device, renderPass, nullptr);
		// 销毁缓冲区和释放映射内存
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}
		// 销毁描述符池，同时描述符集自动释放
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		
		// 销毁采样对象
		vkDestroySampler(device, textureSampler, nullptr);
		// 销毁纹理图像视图对象
		vkDestroyImageView(device, textureImageView, nullptr);
		// 销毁图像缓冲对象和释放对应分配的内存地址
		vkDestroyImage(device, textureImage, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);
		
		// 销毁描述符集布局
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		// 销毁缓存区
		vkDestroyBuffer(device, indicesBuffer, nullptr);
		vkFreeMemory(device, indicesBufferMemory, nullptr);
		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);


		// 删除信号量和栅栏对象
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
		// 销毁命令池
		vkDestroyCommandPool(device, commandPool, nullptr);


		vkDestroyDevice(device, nullptr); // 销毁逻辑设备
		// 只有在启动了验证层之后，才需要销毁创建的调试消息工具debugMessenger
		if(enableValidationLayers)
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr );

		vkDestroySurfaceKHR(instance, surface, nullptr); // 销毁窗口表面对象
		vkDestroyInstance(instance, nullptr); // 销毁是实例
		glfwDestroyWindow(window); // 销毁窗口
		glfwTerminate(); // glfw结束终端

	}

	// 重建交换链
	void recreateSwapChain()
	{	
		// 最小化处理
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(device);
		cleanSwapChain(); // 不能再使用之前的旧对象

		createSwapChain();
		createImageView();
		createColorResources();
		createDepthResources();
		createFramebuffers();
	}

	// 检查当前系统的Vulkan是否支持验证层
	bool checkValidationLayerSupport()
	{	// 遍历获取实例层的所有属性
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> avaliableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, avaliableLayers.data());
		//std::cout << "avaliable layer:\n";
		//for (const auto& layer : avaliableLayers)
		//{
		//	std::cout << "\t" << layer.layerName << "\n";
		//}
		// 检查是否可用的实例层属性中是否包含了验证层
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
	// 从GLFW库中获取Vulkan扩展个数和扩展列表,并根据是否启用验证层，可以在运行时接收调试信息。
	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		// 使用glfwGetRequiredInstanceExtensions函数获取扩展的列表和数量
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		// 构造函数（第一个参数地址，最后一个参数地址）
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		// 当启用验证层时，需要指明启用验证层调试扩展
		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	void createInstance() {

		// 验证层启用并检查是否支持验证层
		if (enableValidationLayers && !checkValidationLayerSupport())
			throw std::runtime_error("validation layers requested, but not avaliable!");

		// 创建实例前，创建保存应用程序信息的结构体：
		//	声明该结构体类型，应用程序名称/版本，引擎名称/版本，vulkan api版本
		// 以便优化我们的特定应用程序
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0; // 告诉后续实例层本应用程序期望支持的最低Vulkan API版本
		
		// 创建实例，说明类型，赋值得到应用程序创建的部分信息
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// 设置实例创建的信息结构体的扩展列表对象
		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		// 创建调试消息对象，准备插入验证层
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
			// 构建调试的信息结构体
			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		// 创建实例，一般来说Vulkan的创建对象函数的参数格式为(创建的信息结构体的指针, 回调指针, 创建对象的指针)
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create instance");

	}

	QueueFamilyIndices findQueueFimalies(VkPhysicalDevice physicalDevice)
	{
		// 查找图形队列的数量和存储所有队列的属性对象
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		QueueFamilyIndices indices = {}; // 图形队列和展示队列索引
		int i = 0;
		VkBool32 presentSupport = false;
		// 检查该物理设备是否有支持图形操作和表面缓冲区的队列族
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
	// 获取物理设备支持的功能，并检查指定的扩展功能是否支持。
	bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
	{
		// 检查物理设备可用的扩展功能，并存储到数组中
		uint32_t avaliableExtensionsCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &avaliableExtensionsCount, nullptr);
		std::vector<VkExtensionProperties> avaliableExtensions(avaliableExtensionsCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &avaliableExtensionsCount, avaliableExtensions.data());
		// 将所需的扩展功能设置到set集合中，并后续通过循环遍历可用扩展功能数组，
		// 来检查是否包含所有所需扩展功能。
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
		for (const auto& extension : avaliableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}


	// 检查设备的Capabilities、SurfaceFormat、PresentModes
	SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice physicalDevice)
	{
		SwapChainSupportDetails details;
		// 查询物理设备的基本属性
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
		// 查询物理设备的表面格式属性
		uint32_t formatsCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, nullptr);
		if (formatsCount != 0)
		{
			details.formats.resize(formatsCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, details.formats.data());
		}
		// 查询物理设备的展示模式属性
		uint32_t presentModesCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr);
		if (presentModesCount != 0)
		{
			details.presentModes.resize(presentModesCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, details.presentModes.data());
		}
		return details;
	}

	// 检侧选择使用合适的物理设备
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice)
	{
		// 查询设备是否具有指定功能队列，并获取队列索引
		QueueFamilyIndices indices = findQueueFimalies(physicalDevice);
		// 检查设备的扩展功能支持情况
		bool deviceExtensionsSupported = checkDeviceExtensionSupport(physicalDevice);
		// 查询设备是否支持各向异性采样
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
		// 查询设备交换链创建前，基础属性、表面格式、呈现模式是否支持
		bool deviceSwapChainAdequate = false;
		if (deviceExtensionsSupported)
		{
			SwapChainSupportDetails details = querySwapchainSupport(physicalDevice);
			deviceSwapChainAdequate = details.isFormatsAndPresentModesEmpty();

		}
		return indices.isComplete() && deviceExtensionsSupported && (!deviceSwapChainAdequate) && deviceFeatures.samplerAnisotropy;
	}

	// 使用isDeviceSuitable函数来筛选并选择物理设备
	void pickPhysicalDevice()
	{
		// 遍历物理设备
		// 得到物理设别个数
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		if (physicalDeviceCount == 0)
			throw std::runtime_error("Not Found the physical device with Vulkan support!");
		// 存放物理设备句柄于数组中
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
		// 检查是否有图形队列的物理设备
		for (const auto& device : physicalDevices)
		{
			if (isDeviceSuitable(device))
			{
				physicalDevice = device;
				massSamples = getMaxUsableSampleCount(); // 获取物理设备支持的颜色和深度采样数之和
				break;
			}
		}
		if(physicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Failed to find a suitable GPU!");
	}

	// 验证层信息的回调函数
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		// 根据严重程度和类型决定如何处理消息
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
	// 填充消息调试对象的信息结构体
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}
	// 设置消息调试对象
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
		// 获取队列族索引
		QueueFamilyIndices indices = findQueueFimalies(physicalDevice);
		// 创建队列族数组，保存图形操作和展示操作的队列
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { // 以图形队列和展示队列的索引构建集合,顺便去重
			indices.grahicsFamily.value() , indices.presentFamily.value() 
		};
		// 设置该队列的优先级
		float queuePriority = 1.0f;
		// 遍历队列族，获取所有队列族，填充到队列族数组
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; // 标识该结构体类型
			queueCreateInfo.queueFamilyIndex = queueFamily; // 队列族索引
			queueCreateInfo.queueCount = 1; // 队列个数
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}
		
		VkPhysicalDeviceFeatures deviceFatures{}; //查询物理设备功能
		deviceFatures.samplerAnisotropy = VK_TRUE;
		deviceFatures.sampleRateShading = VK_TRUE;
		// 创建逻辑设备的信息结构体
		VkDeviceCreateInfo deviceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()), // 设定选择的队列族相关信息
			.pQueueCreateInfos = queueCreateInfos.data(),
			.pEnabledFeatures = &deviceFatures // 逻辑设备具有与物理设备相同的功能
		};
		// 通过添加pickuPphysicalDevice函数，都会在接下来启用指定扩展功能和验证层
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		
		// 根据验证层标志参数，决定逻辑设备是否添加验证层信息
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
		// 获取与逻辑设备一起创建的图形队列句柄，以方便后续的图形操作的队列
		vkGetDeviceQueue(device, indices.grahicsFamily.value(), 0, &graphicsQueue);
		// 获取支持展示操作的队列句柄，以方便后续展示操作的队列
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	/*---- 此处开始展示层构建 ----*/


	void createSurface() // surface同逻辑设备创建时自动加载，此处使用glfwCreateSurface函数即可
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create window surface!");
	}
	// 选择表面格式的format具有指定类型的颜色通道，colorspace具有指定类型的颜色空间
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && 
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}
		return availableFormats[0];// 一把来说当不存在指定类型时，我们选择第一个格式进行使用
	}
	// 选择图像呈现类型
	VkPresentModeKHR chooseSwapSurfacePresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availablePresentMode;
		}
		return VK_PRESENT_MODE_FIFO_KHR;// 一把来说当不存在指定类型时，我们选择FIFO呈现图像类型
	}
	// 图像的基本属性：宽高度进行选择
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

	// 创建交换链
	void createSwapChain()
	{	
		// 确认三大基本属性，并选择合适值
		SwapChainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice);
		VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapSurfacePresentMode(swapchainSupport.presentModes);
		// 设置交换链中的图像个数，并且防止设定图像个数过大
		uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
		if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
			imageCount = swapchainSupport.capabilities.maxImageCount;
		// 创建交换链信息结构体
		VkSwapchainCreateInfoKHR swapchainCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		// 指定每个图像包含的层数，除非是3D应用程序开发，否则此处应为1
		.imageArrayLayers = 1, 
		// 此处我们将直接图像渲染，这意味着它们被用作颜色附件
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
		};
		// 交换链队列族参数
		QueueFamilyIndices indices = findQueueFimalies(physicalDevice);
		uint32_t queueFamilyIndices[] = {indices.grahicsFamily.value(), indices.presentFamily.value()};
		// 如果图形和展示队列不同，那么此处根据教程使用并发模式
		if (indices.grahicsFamily != indices.presentFamily)
		{
			// 此处指明图像一次由一个队列族拥有，并且在另一个队列使用它之前需要显式转移所有权
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			// 表面图像可以跨多个队列使用，无需显式转移所有权
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCreateInfo.queueFamilyIndexCount = 0;
			swapchainCreateInfo.pQueueFamilyIndices = nullptr;
		}
		// 指定了在交换链中的图像在呈现到屏幕之前应该应用的预变换。
		swapchainCreateInfo.preTransform = swapchainSupport.capabilities.currentTransform;
		// 指定 Alpha 通道是否应用于与窗口系统中的其他窗口混合。
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// 指定交换链的呈现模式
		swapchainCreateInfo.presentMode = presentMode;
		// 指定当图像超过窗口边界时是否应裁剪
		swapchainCreateInfo.clipped = VK_TRUE;
		// 用于重新创建交换链时的初始化，此处为NULL_HANDLE表面为第一次创建
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS)
			throw std::runtime_error("failed to create swap chain!");
		
		// 获取交换链图像句柄
		uint32_t imagesCount = 0;
		vkGetSwapchainImagesKHR(device, swapchain, &imagesCount, nullptr);
		if (imagesCount != 0)
		{
			swapChainImages.resize(imagesCount);
			vkGetSwapchainImagesKHR(device, swapchain, &imagesCount, swapChainImages.data());
		}
		// 设置图像的格式和图像大小
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}
	// 创建纹理图像视图
	void createTextureImageView()
	{
		textureImageView = createImageView(textureImage, mipLevels,
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	}
	// 创建图像视图
	void createImageView()
	{
		swapChainImageViews.resize(swapChainImages.size());
		for (uint32_t i = 0; i < swapChainImages.size(); ++i)
		{
			swapChainImageViews[i] = createImageView(swapChainImages[i], 1,
				swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}
	// 创建图像视图的通用函数
	VkImageView createImageView(VkImage image,uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspectFlags)
	{
		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		// 指向交换链图像
		imageViewInfo.image = image;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // 该视图类型
		imageViewInfo.format = format; // 视图格式与图像格式相同
		// 指定颜色混合通道类型
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		// 设置图像的哪些方面被包含在图像视图中、纹理的mipmap层级
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
	/*---- 此处开始图形管线的构建 ----*/
	// 着色器模块
	// 读取着色器模块的代码文件内容并返回
	static std::vector<char> readFile(const std::string& filename) 
	{	
		// std::ios::ate:打开文件使光标移植文件尾部
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open())
			throw std::runtime_error("Failed to open the file!");
		size_t filesize = (size_t)file.tellg(); // 获取当前文件读取位置的偏移量，相反的输出流使用的tellp()
		std::vector<char> buffer(filesize); // 创建文件数据缓冲区
		file.seekg(0); // 移动光标至内容的开头
		file.read(buffer.data(), filesize); // 读取文件
		file.close();// 关闭文件
		return buffer;
	}
	// 根据传递的着色器代码创建着色器模块对象VkShaderModule
	VkShaderModule createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = code.size();
		// 需要注意reinterpret_cast是底层内存操作，编译器不再检查潜在危险。
		// 因为code为vector类型，该数据结果已经做了对齐操作。
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
			throw std::runtime_error("Failed to creat shader module!");
		return shaderModule;
	}
	// 创建渲染通道
	void createRenderPass() 
	{
		// 添加附件描述符：设置附件格式、采样方式、从内存的加载和存储方式，模板缓冲区的加载和存储方式、最初和最终渲染图像布局方式
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat; // 颜色附件的格式应与交换链图像的格式匹配
		colorAttachment.samples = massSamples;
		// 在渲染前和渲染后如何处理附件中的数据
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // 在开始时将值清除为常数
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // 渲染内容将存储在内存中，稍后读取
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // 不使用模板缓冲区，将不关心
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// 设置初始图像布局和渲染处理后图像布局
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // 开始时不关心图像布局
		// 因为使用了多重采样，此时的颜色附件不再是普通的颜色附件，我们将其最终布局改为颜色附件
		// 而最后会创建解析颜色附件对象
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// 子通道和附件引用：引用的附件索引，引用附件的图像布局类型
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; // 附件数组的索引，因为本处只是用了一个附件，所以索引为0
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // 表示将附件作为颜色缓冲区布局

		// 添加深度附件
		VkAttachmentDescription depthAttachment{
			.format = findDepthFormat(),
			.samples = massSamples,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // 不关心存储深度操作，在绘制完成后不再使用
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // 不关心之前的深度对象的内容
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};
		VkAttachmentReference depthAttachmentRef{
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};


		// 创建颜色附件的解析附件，将其转换为常规附件
		VkAttachmentDescription colorAttachmentResolve{
			.format = swapChainImageFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, 
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR // 此处图像最终布局必须为展示队列来源
		};
		VkAttachmentReference colorAttachmentResolveRef{
			.attachment = 2,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		// 创建子通道：指明子通道类型，引用附件个数和指出附件对象
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // 表面为用作图形子通道
		subpass.colorAttachmentCount = 1; // 附件个数
		subpass.pColorAttachments = &colorAttachmentRef; // 颜色附件对象
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		// 描述不同子通道之间的依赖性，它们之间如何协调的
		VkSubpassDependency dependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL, // 指定依赖关系的源子通道。表示依赖于渲染通道之外的某种外部操作。
			.dstSubpass = 0, // 依赖关系的目标子通道
			// 指定源子通道中的哪些管道阶段（pipeline stages）完成了操作。
			// 指定完成颜色附件输出阶段和片段测试后阶段
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
				| VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT 
				| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT // 指定源子通道中的哪些访问类型完成了操作。
		};
		// 指定目标子通道中的哪些管道阶段需要访问源子通道的数据。
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 
			| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		// 指定目标子通道中的哪些访问类型需要访问源子通道的数据。
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT 
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		// 将所有附件组合为数组
		std::array< VkAttachmentDescription, 3> attachments{ colorAttachment, depthAttachment, colorAttachmentResolve };

		// 渲染通道的创建：指明附件，子通道, 子通道依赖性
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

	// 创建图形管线的主体函数
	void createGraphicsPipline()
	{
		// 获取顶点和片段着色器编译后代码
		auto vertShaderCode = readFile("src/VulkanTest/Shaders/vert.spv");
		auto fragShaderCode = readFile("src/VulkanTest/Shaders/frag.spv");
		// 将代码作为参数创建对应着色器模块
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
		// 创建顶点着色器模块的信息结构体
		VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
		vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		// 指明该着色器在管线着色器的哪个处理阶段
		vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageCreateInfo.module = vertShaderModule;
		vertShaderStageCreateInfo.pName = "main"; // 指明着色器调用的函数(入口点)
		// 创建片段着色器模块的信息结构体
		VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
		fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		// 指明该着色器在管线着色器的哪个处理阶段
		fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageCreateInfo.module = fragShaderModule;
		fragShaderStageCreateInfo.pName = "main"; // 指明着色器调用的函数(入口点)

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageCreateInfo, fragShaderStageCreateInfo };

		// 固定功能
		// 动态状态Dynamic State：启用视口和裁剪矩形
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR // 可修改裁剪矩形大小
		};
		VkPipelineDynamicStateCreateInfo dynamicCreateInfo{};
		dynamicCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicCreateInfo.pDynamicStates = dynamicStates.data();

		// 获取顶点数据和如何处理顶点数(得到其中的实际处理成员如：vertex&color)
		auto bindingDescription = Vertex::getBindingDescription();
		auto attriDescription = Vertex::getAttributeDescription();
		// 顶点输入Vertex Input
		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
		vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attriDescription.size());
		vertexInputCreateInfo.pVertexAttributeDescriptions = attriDescription.data();
		
		// 输入组装Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
		inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
		// 视口和裁剪Viewport & scissors，需要在动态状态设置
		// 需要在管道创建时，指定视口和裁剪矩形的计数
		VkPipelineViewportStateCreateInfo viewportCreateInfo{};
		viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportCreateInfo.viewportCount = 1;
		viewportCreateInfo.scissorCount = 1;
		// 若为静态设置，则需要指明视口和裁剪矩形的结构体
		//viewportCreateInfo.pViewports = &viewport;
		//viewportCreateInfo.pScissors = &scissor;
		// 光栅化器Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizeCreateInfo{};
		rasterizeCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizeCreateInfo.depthClampEnable = VK_FALSE; // 深度限制，设置时，超出nearplan和farplane的片段不会固定
		rasterizeCreateInfo.rasterizerDiscardEnable = VK_FALSE; // 启用时，几何图形永远不通过光栅化阶段
		rasterizeCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // 几何生成方式
		rasterizeCreateInfo.lineWidth = 1.0f; // 线段宽度
		rasterizeCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; // 面剔除方式
		rasterizeCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // 正面顶点顺序
		rasterizeCreateInfo.depthBiasEnable = VK_FALSE; // 斜率偏置

		// 多重采样multiSampling
		VkPipelineMultisampleStateCreateInfo multismapleCreateInfo{};
		multismapleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multismapleCreateInfo.sampleShadingEnable = VK_TRUE; // 启用样本着色?
		multismapleCreateInfo.rasterizationSamples = massSamples; // 每个像素的样本数
		multismapleCreateInfo.minSampleShading = .2f;  // 每个像素的最小样本着色比例,1.0f指定所有像素都需要被采样着色
		multismapleCreateInfo.pSampleMask = nullptr; // 哪些样本有效
		multismapleCreateInfo.alphaToCoverageEnable = VK_FALSE; // Alpha到Coverage
		multismapleCreateInfo.alphaToOneEnable = VK_FALSE; // Alpha到One
		// 深度模板状态
		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE, //启用深度测试，新片段的深度值与深度缓冲区进行比较
			.depthWriteEnable = VK_TRUE, // 在启用深度测试时，确定是否将通过测试的新片段深度值写入深度缓冲区进行更新
			.depthCompareOp = VK_COMPARE_OP_LESS, // 片段深度值与深度缓冲区值如何比较
			.depthBoundsTestEnable = VK_FALSE, // 若启用，则只会保留在min~max参数之间的片段
			.stencilTestEnable = VK_FALSE, // 启用模板测试
			.front = {},
			.back = {},
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 1.0f,
		};

		// 颜色混合Color blending
		// 定义颜色附件，指定每个颜色附件的混合行为：即颜色/透明度混合因子,颜色/透明度混合方式
		VkPipelineColorBlendAttachmentState colorBlendAttachment{
			.blendEnable = VK_FALSE,// 不启用颜色混合
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};
		// 引用所有帧缓冲区的结构数组
		VkPipelineColorBlendStateCreateInfo colorBlending{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
			.blendConstants = 0.0f
		};

		// 管道布局：描述管道如何与描述符集如何交互
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1, // 描述集布局数
		.pSetLayouts = &descriptorSetLayout, // 设置描述集布局数组的指针
		.pushConstantRangeCount = 0, // 推入常量范围的数量
		.pPushConstantRanges = nullptr // 指向推入常量范围数组的指针
		};
		if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create pipeline layout!");
		// 创建图形管线
		VkGraphicsPipelineCreateInfo pipelineInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStages, // 着色器阶段目标
		// 固定处理
		.pVertexInputState = &vertexInputCreateInfo,
		.pInputAssemblyState = &inputAssemblyCreateInfo,
		.pViewportState = &viewportCreateInfo,
		.pRasterizationState = &rasterizeCreateInfo,
		.pMultisampleState = &multismapleCreateInfo,
		.pDepthStencilState = &depthStencilCreateInfo,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicCreateInfo,
		// 管道布局
		.layout = pipelineLayout,
		.renderPass = renderPass, // 指定渲染通道对象
		.subpass = 0, // 指定子通道索引
		.basePipelineHandle = VK_NULL_HANDLE, // 派生管道来创建新管道时，指定现有管道的句柄
		.basePipelineIndex = -1, // 通过索引来创建另一个管道
		};
		if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
			throw std::runtime_error("failed to create graphics pipeline!");
		// 在使用完着色器后，需要进行清除
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
	}
	// 创建统一缓冲区对象，以及相关的缓冲区、分配的缓冲区映射设备内存、
	void createUniformBuffer()
	{
		VkDeviceSize size = sizeof(UniformBufferObject); // 创建统一缓冲区对象
		// 将相关对象指定大小(同帧相关)
		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{	
			// 创建缓冲区对象，并分配设备内存，缓冲区绑定分配的设备内存对象
			createBuffer(
				size, 
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[i], uniformBuffersMemory[i]);
			// 映射内存
			vkMapMemory(device, uniformBuffersMemory[i], 0, size, 0, &uniformBuffersMapped[i]);
		}

	}
	// 创建描述符集布局
	void createDescriptorSetLayout()
	{
		// 描述符集布局指出绑定UBO对象
		VkDescriptorSetLayoutBinding uboLayoutBinding{
			.binding = 0, // 指定绑定资源的binding number
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // 描述符集类型
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT, // 指定着色器阶段标志位
			.pImmutableSamplers = nullptr // 图像采样相关的描述符
		};
		// 描述符布局设置绑定对象为组合采样图像
		VkDescriptorSetLayoutBinding samplerLayoutBinding{
			.binding = 1, // 这个binding号应与glsl代码中location(binding=value)值一致
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // 设置阶段为片段着色器
		};
		// 将设置的描述符布局对象存放至数组对象
		std::array < VkDescriptorSetLayoutBinding,2> bindings = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(bindings.size()),
			.pBindings = bindings.data()
		};
		if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create the descriptor set layout!");
	}
	// 创建描述符池
	void createDescriptorPool()
	{	
		// 指定描述符池大小，指明描述符类型和描述符数量
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
			.maxSets = MAX_FRAMES_IN_FLIGHT, // 最大可分配描述符个数
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data() // 指明描述符池大小
		};

		if(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) 
			throw std::runtime_error("failed to create descriptor pool!");
	}
	// 创建描述符集，指明参照的描述符布局和设置描述符集个数与帧数相同
	// 描述符集不需要显示释放，在销毁描述符池时会自动释放
	void createDescriptorSets()
	{	
		// 创建同样数量的描述符集布局
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		// 描述符集喷配信息，指明引用的描述符池、描述符集布局
		VkDescriptorSetAllocateInfo descriptorAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = MAX_FRAMES_IN_FLIGHT, //同帧数相同
			.pSetLayouts = layouts.data()
		};
		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if(vkAllocateDescriptorSets(device, &descriptorAllocateInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		// 根据帧数来实际更新每个描述符集的描述符实际信息
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{	
			// 设置统一缓冲区资源信息：指明实际缓冲区的数据，以及大小范围
			VkDescriptorBufferInfo descriptorUBOInfo{
				.buffer = uniformBuffers[i],
				.offset = 0,
				.range = sizeof(UniformBufferObject)
			};
			// 设置纹理图像资源信息：指明采样器,纹理图像视图和图像布局
			VkDescriptorImageInfo descriptorImageInfo{
				.sampler = textureSampler,
				.imageView = textureImageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
			// 将设置的资源对象信息，准备写入到描述符集中
			std::array<VkWriteDescriptorSet, 2> writeDescriptorSets{
				writeDescriptorSets[0] =
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = descriptorSets[i],
					.dstBinding = 0, // 绑定的目标数据
					.dstArrayElement = 0, // 描述符结构位数组时使用
					.descriptorCount = 1, // 指定要更新的数组元素数量
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pImageInfo = nullptr, //用于引用图像数据的描述符
					.pBufferInfo = &descriptorUBOInfo, // 用于引用缓冲区数据的描述符
					.pTexelBufferView = nullptr // 用于引用缓冲区视图的描述符
				},
				writeDescriptorSets[1] =
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = descriptorSets[i],
					.dstBinding = 1, // 绑定的目标数据
					.dstArrayElement = 0, // 描述符结构位数组时使用
					.descriptorCount = 1, // 指定要更新的数组元素数量
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = &descriptorImageInfo, //用于引用图像数据的描述符
				}
			};
			// 更新描述符集中的描述符信息
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}
	}
	// Drawing阶段
	// 帧缓冲区
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
				.renderPass = renderPass, // 指定的渲染通道
				.attachmentCount = static_cast<uint32_t>(attachments.size()),
				.pAttachments = attachments.data(), // 指定图像视图作为附件
				// 此处需要指明帧缓冲对应的图像视图的一些基本属性
				.width = swapChainExtent.width, // 于图像视图的宽高度相同
				.height = swapChainExtent.height,
				.layers = 1 // 指向图像数组的层数
			};
			if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create framebuffer!");
		}
	}
	// 命令池
	void createCommandPool()
	{	
		// 获取交换链的队列索引值
		QueueFamilyIndices indices = findQueueFimalies(physicalDevice);
		// 指定命令池创建方式、指向的队列族索引
		VkCommandPoolCreateInfo commandPoolInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = indices.grahicsFamily.value()
		};
		if (vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
			throw std::runtime_error("failed to create CommandPool!");
	}

	// 分配命令缓冲区
	void createCommandBuffers()
	{	
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo commandBufferInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, // 指定该命令缓冲区为主命令缓冲区
			.commandBufferCount = (uint32_t)commandBuffers.size()
		};
		if (vkAllocateCommandBuffers(device, &commandBufferInfo, commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to create CommandBuffer!");


	}
	// 查找指定内存类型和想要的内存属性的内存索引
// typeFilter 适合缓冲区的内存类型的位值，properties表示内存属性值
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		// 获取物理设备的内存属性
		VkPhysicalDeviceMemoryProperties memoryProperty;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperty);
		// 变量物理设备的内存属性，来查到指定的内存类型
		for (uint32_t i = 0; i < memoryProperty.memoryTypeCount; ++i)
		{
			// 首先
			if ((typeFilter & (1 << i)) && ((memoryProperty.memoryTypes[i].propertyFlags & properties) == properties))
				return i;
		}

		throw std::runtime_error("not found the suitable memory!");
	}

	// 创建顶点缓存区对象，并分配内存
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		// 创建缓存对象
		VkBufferCreateInfo bufferInfo{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.flags = 0,
			.size = size, // 指定缓冲区大小 单位字节
			.usage = usage, // 缓冲区的使用目的(类型)
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE // 缓冲区只能从图形队列中访问，设置独占访问模式
		};
		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			throw std::runtime_error("failed to create buffer!");
		// 获取缓存实际所需的内存大小
		VkMemoryRequirements memRequirement;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirement);
		// 分配实际所需内存大小
		VkMemoryAllocateInfo allocateInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirement.size,
			.memoryTypeIndex = findMemoryType(memRequirement.memoryTypeBits, properties)
		};
		if (vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate buffer memory!");
		// 将缓存Vkbuffer对象与分配内存VkDeviceMemory绑定
		vkBindBufferMemory(device, buffer, bufferMemory, 0); // 第三个参数，如果非零，则需要能够被memRequirements.alignment整除
	}
	// 开始单次指令缓冲执行
	VkCommandBuffer beginSingleTimeCommands()
	{
		// 内存传输是使用命令缓冲区执行，因此我们需要创建一个临时的命令缓冲区
		VkCommandBufferAllocateInfo commandBufferAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = commandPool,  // 设置命令池
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, // 设置该命令缓冲区的等级类型
			.commandBufferCount = 1
		};
		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);
		VkCommandBufferBeginInfo commandBufferBeginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};
		// 开始记录命令缓冲区
		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
		return commandBuffer;
	}
	// 结束单次指令缓冲执行
	void endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer); // 结束记录
		// 将命令缓冲区记录的命令提交至队列
		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffer
		};
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		// 使用队列等待至空闲来保证复制数据操作和上传队列的操作完成
		vkQueueWaitIdle(graphicsQueue);
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}
	// 复制缓冲区数据
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{	
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		// 准备复制缓存区数据
		VkBufferCopy copyRegion{ 
			.srcOffset = 0,
			.dstOffset = 0,
			.size = size
		};
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		endSingleTimeCommands(commandBuffer);
	}
	// 创建图像缓冲对象，并分配实际映射内存对象
	void createImage(uint32_t  width, uint32_t  height, uint32_t mipLevels, 
		VkSampleCountFlagBits numSamples, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usages,  
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		// 创建图像对象
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D, // 作为平面数据
			.format = format,
			.extent = {width, height, 1},
			.mipLevels = mipLevels,  // mipmap等级
			.arrayLayers = 1, // 本图像为单个对象没有使用数组
			.samples = numSamples,
			.tiling = tiling, // 指定纹素的排列方式：按行/定义顺序
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
	// 转换图像布局, 使用图像屏障进行同步
	void transitionImageLayout(VkImage image, uint32_t mipLevels, VkFormat format,
		VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.oldLayout = oldLayout, // 当前布局
			.newLayout = newLayout, // 目标布局
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, // 不涉及队列族共享
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image, // 图像句柄
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				// 影响的Mip映射层索引和数量
				.baseMipLevel = 0,
				.levelCount = mipLevels,
				.baseArrayLayer = 0,
				.layerCount = 1, 
			}
		};
		//// 当newLayout为模板附件时
		//if (newLayout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL)
		//{	// 需要在图像特定范围参数指定哪方面被使用：深度位和模板位
		//	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		//	if (hasStencilComponent(format))
		//		barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		//}
		VkPipelineStageFlags srcStage; // 当前阶段，屏障发生前的相关访问类型、阶段类型
		VkPipelineStageFlags dstStage; // 目标阶段，屏障发生后的相关访问类型、阶段类型
		// 从初始化阶段到传输阶段
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{	// src在屏障之前，dst在屏障之后
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // 当前阶段，top_of_pipe指明流水线顶部阶段
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // 目标阶段为传输
			// 从传输阶段到着色器读取阶段
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			// src在屏障之前，dst在屏障之后
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;; // 传输阶段写入
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // 着色器阶段仅读
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // 当前阶段，top_of_pipe指明流水线顶部阶段
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // 目标阶段为传输
		}
		//else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		//{
		//	barrier.srcAccessMask = 0;
		//	barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		//	
		//	srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		//	// reading发生在片段初期阶段，write发生在片段后期阶段
		//	dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		//}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}
		vkCmdPipelineBarrier( // 管道创建屏障的指令
			commandBuffer,
			srcStage, dstStage,
			0,
			0,nullptr,
			0,nullptr,
			1,&barrier
			);
		endSingleTimeCommands(commandBuffer);
	}
	// 将缓冲区数据复制到图像对象中
	void copyBufferToImage(VkBuffer buffer, VkImage image, 
		uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		VkBufferImageCopy region{ // 指明待复制区域的结构
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
			commandBuffer, // 命令缓冲区句柄
			buffer, // 源缓冲区
			image, // 目标图像
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // 图像的布局
			1, // 拷贝区域的数量
			&region // 拷贝区域的数组
		);
		endSingleTimeCommands(commandBuffer);
	}
	// 加载图像数据，创建图像缓存区
	void createTextureImage()
	{	
		// 设置纹理宽度高度、和纹理颜色通道数
		int texWidth, texHeight, texChannel;
		// uc == unsigned char，stbi_load第四个参数指，使用图像原始的图像通道数
		stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannel, STBI_rgb_alpha);
		VkDeviceSize size = texWidth * texHeight * 4; // STBI_rgb_alpha强制图像使用alpha通道，所以一个像素占用4个字节
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(max(texWidth, texHeight)))) + 1;
		if (!pixels)
			throw std::runtime_error("failed to load texture image!");
		// 创建临时缓冲区保存图像数据
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(size, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			stagingBuffer, stagingBufferMemory);
		// 复制数据
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, pixels, size);
		vkUnmapMemory(device, stagingBufferMemory);
		stbi_image_free(pixels); // 释放加载的图像数据
		// 创建纹理图像缓存，设备分配内存对象，并将其进行绑定
		createImage(
			texWidth, texHeight, 
			mipLevels,
			VK_SAMPLE_COUNT_1_BIT,
			swapChainImageFormat, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			textureImage, textureImageMemory);
		// 布局转换 无状态->传输阶段
		transitionImageLayout(
			textureImage, 
			mipLevels,
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		// 复制缓冲区数据到图像
		copyBufferToImage(stagingBuffer, textureImage, texWidth, texHeight);
		// 改为使用mipmap方式生成
		
		// 布局转换 管道传输阶段—>着色器仅读阶段
		//transitionImageLayout(
		//	textureImage,
		//	mipLevels,
		//	VK_FORMAT_R8G8B8A8_SRGB,
		//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		// 销毁临时缓冲区和释放分配的临时内存对象
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
		generateMipmap(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
	}

	void generateMipmap(VkImage image,VkFormat format, int32_t width, int32_t height, uint32_t mipLevels)
	{
		// 先检查纹理图像格式是否支持线性过滤
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
			throw std::runtime_error("texture image format does not support linear blitting!");

		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		// 初始化屏障结构体
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
		// 循环遍历生成mipmap
		int32_t mipWidth = width, mipHeight = height;
		for(uint32_t i = 1; i < mipLevels; ++i)
		{	// 计算当前mip等级
			barrier.subresourceRange.baseMipLevel = i - 1;
			// 图像布局转换设置, 确保执行blit指令前的图像布局是连续的，确保图像作为源图像可读
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
				0, nullptr, // 不内存屏障
				0, nullptr, // 不适用缓冲区内存屏障
				1, &barrier // 使用图像内存屏障
			);
			// 设置blit命令参数
			// blit操作：描述图像的复制操作。
			// 在生成mipmap图像操作中，我们需要将新的一级图像的宽高度/2
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
			vkCmdBlitImage( // blit指令的布局转换跟屏障前的布局转换是连续的
				commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR // 采用线性过滤
			);


			// 图像布局转换设置,blit指令执行后，i-1的mip层图像作为纹理着色器读取，而不会再修改
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
				0, nullptr, // 不内存屏障
				0, nullptr, // 不适用缓冲区内存屏障
				1, &barrier // 使用图像内存屏障
			);
			// 为下一轮mipmap图像宽高度计算更新
			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		};
		//设置最后一层的mipmap布局,最后一层的mipmap图像不能被传输，而仅供着色器只读
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
	// 创建采样器
	void createTextureSampler()
	{	
		// 准备获取设备支持的最大各向异性比例值
		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
		// 准备创建各向异性对象
		VkSamplerCreateInfo samplerInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_LINEAR, // 指定采样放大操作时的过滤方式
			.minFilter = VK_FILTER_LINEAR, // 指定采样缩小操作时的过滤方式
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, // 指定Mipmap的计算模式
			// 当纹理空间大于图像大小时，我们指定其纹理重复方式
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT, // U方向循环重复
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT, // V方向循环重复
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT, // w方向循环重复
			.mipLodBias = 0.0f, // mipmap计算LOD时的偏差
			.anisotropyEnable = VK_TRUE, // 开启各向异性功能
			.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy, // 最大各向异性比例
			// 纹素比较操作，用于阴影贴图
			.compareEnable = VK_TRUE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			// mipmap LOD
			.minLod = 0.0f,
			.maxLod = VK_LOD_CLAMP_NONE, // == 1000.0f
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK, // 边界颜色
			.unnormalizedCoordinates = VK_FALSE, // 是否使用非归一化坐标
		};
		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
			throw std::runtime_error("failed to create texture sampler!");
	}

	// 创建顶点缓存区对象，并分配内存
	void createVertexBuffer()
	{	
		VkDeviceSize size = sizeof(vertices[0]) * vertices.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// 准备填写入实际内容
		void* data;
		// 映射内存，运行CPU直接写入该部分内存区域，data表示指向映射内存的地址指针
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, vertices.data(), (size_t)size); // 复制内容
		vkUnmapMemory(device, stagingBufferMemory); // 取消映射
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
		// 复制数据
		copyBuffer(stagingBuffer, vertexBuffer, size);
		// 销毁创建的临时缓冲区
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		// 释放临时缓冲区的内存
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// 创建索引缓存区对象，并分配内存
	void createIndexBuffer()
	{
		VkDeviceSize size = sizeof(indices[0]) * indices.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		
		// 准备填写入实际内容
		void* data;
		// 映射内存，运行CPU直接写入该部分内存区域，data表示指向映射内存的地址指针
		vkMapMemory(device, stagingBufferMemory, 0, size, 0, &data);
		memcpy(data, indices.data(), (size_t)size); // 复制内容
		vkUnmapMemory(device, stagingBufferMemory); // 取消映射
		createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indicesBuffer, indicesBufferMemory);
		// 复制数据
		copyBuffer(stagingBuffer, indicesBuffer, size);
		// 销毁创建的临时缓冲区
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		// 释放临时缓冲区的内存
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	// 记录命令缓冲区
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		// 设置命令缓冲区开始记录的结构体
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // 不选择如何使用缓冲区命令
		beginInfo.pInheritanceInfo = nullptr;
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		// 渲染通道开始的结构体
		VkRenderPassBeginInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = renderPass,
			.framebuffer = swapChainFramebuffers[imageIndex], // 指定缓冲区对象
		};
		// 设置渲染区域的大小
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = swapChainExtent;
		// 表示颜色附件和深度模板附件的初始清除值
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {{0.0f,0.0f, 0.0f,1.0f}};
		clearValues[1].depthStencil = { 1.0, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		// 开启渲染通道
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		// 绑定描述符集，需要指明在哪个类型管线使用
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
			0, 1, &descriptorSets[currentFrames], 0, nullptr);
		// 绑定图形管线
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		// 因为在固定功能一节指定了视口和裁剪矩形为dynamic state，此处需要在执行命令前设置
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
		// 发出绘制三角形指令
		//vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		// 结束渲染通道执行
		vkCmdEndRenderPass(commandBuffer);
		// 结束记录命令缓冲区
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to end the command buffer!");
	}
	void updateUniformBuffer(uint32_t currentFrame)
	{
		// 计算时间
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		// 填充统一缓冲对象
		UniformBufferObject ubo{
			.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			.view = glm::lookAt(glm::vec3(2.0f,2.0f,2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f)
		};
		ubo.proj[1][1] *= -1;
		memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
	}
	// 渲染图像和呈现结果
	void drawFrame()
	{	
		// 等待栅栏
		vkWaitForFences(device, 1, &inFlightFences[currentFrames], VK_TRUE, UINT64_MAX);
		// 获取交换链的下一张图像，第三个参数指定可用的超时时间，以纳秒为单位
		uint32_t imageIndex; // 已可用的交换链图像索引
		VkResult result =  vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrames], VK_NULL_HANDLE, &imageIndex);
		// 表示交换链已与表面不兼容，无法再渲染。通常发生于在窗口调整大小之后
		if (result == VK_ERROR_OUT_OF_DATE_KHR ){
			recreateSwapChain();
			return;
		}
		// suboptimal表示次优，交换链仍可以成功呈现在表面，但表面属性不一定完全匹配
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
			}
		// 更新数据
		updateUniformBuffer(currentFrames);
		// 我们进行延迟重置栅栏，在使用前需要把栅栏重置
		vkResetFences(device, 1, &inFlightFences[currentFrames]);

		// 使用前重置命令缓冲区的状态，使其重新记录命令
		vkResetCommandBuffer(commandBuffers[currentFrames], 0);
		// 传递渲染的图像索引，准备命令缓冲区开始记录
		recordCommandBuffer(commandBuffers[currentFrames], imageIndex);
		// 提交命令缓冲区
		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
		};
		// 等待哪个信号量
		VkSemaphore waitSemaphore[] = { imageAvailableSemaphores[currentFrames] };
		// 在管道哪个阶段等待
		VkPipelineStageFlags waitStage[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphore; // 指定等待的信号量
		submitInfo.pWaitDstStageMask = waitStage; // 指定等待的哪个阶段
		// 指定实际执行的命令缓冲区
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrames];
		// 指定命令缓冲区执行完毕后要发出信号的信号量
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrames] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// 提交至图形队列
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrames]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}
		// 将渲染结果提交回交换链准备展示
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		// 设置代表每一帧完成的信号量
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores; // 确定完成缓冲区执行发出信号的信号量
		// 设置交换链信息
		VkSwapchainKHR swapChains[] = { swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains; // 指定呈现图像的交换链
		presentInfo.pImageIndices = &imageIndex; // 每个交换链呈现图像的索引
		// 获取每个交换链的呈现结果
		presentInfo.pResults = nullptr;
		result = vkQueuePresentKHR(presentQueue, &presentInfo);
		// 当渲染都已提交至展示队列时，发生的图像大小修改
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
	// 创建同步对象
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
			.flags = VK_FENCE_CREATE_SIGNALED_BIT // 在第一帧的等待时，防止第一帧等待虚无同步信号，而无限期阻塞
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
	// 创建深度资源对象
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
	// 检测是否有指定类型的格式
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{	// 获取物理设备表格属性
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
			// 检测是否具有指定的像素排列方式和指定的format属性的format格式
			if (tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & features) == features)
				return format;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & features) == features)
				return format;
		}
		throw std::runtime_error("failed to find supported format!");
	}
	// 是否具有支持深度/模板附件的格式
	VkFormat findDepthFormat()
	{
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	// 是否具有模板分量
	bool hasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
	// 加载模型数据
	void loadModel()
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
			throw std::runtime_error(warn + err);
		// 去重顶点：容易元素结构为：<顶点,索引>
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
				// 若unordered_map容器中不存在该顶点，则加入容器中，
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
	// 创建多重采样资源对象
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