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
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

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
	std::optional<uint32_t> presentFamily;

	bool isComplete() const
	{
		return grahicsFamily.has_value() && presentFamily.has_value();
	}
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
	VkSurfaceKHR surface; // 窗口表面创建
	VkSwapchainKHR swapchain; // 交换链对象
	std::vector<VkImage> swapChainImages; // 交换链图像
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;// 交换链图像视图对象
	// 渲染通道设置参数
	VkRenderPass renderPass; // 渲染通道对象
	VkPipelineLayout pipelineLayout; // 管道布局对象
	VkPipeline graphicsPipeline;	// 图形管道
	// 帧缓冲区参数
	std::vector<VkFramebuffer> swapChainFramebuffers;
	// 命令缓冲区参数
	VkCommandPool commandPool;  // 命令池对象
	std::vector<VkCommandBuffer> commandBuffers; // 命令缓冲区对象
	// 同步参数
	std::vector<VkSemaphore> imageAvailableSemaphores; // 表示图像可用信号
	std::vector<VkSemaphore> renderFinishedSemaphores; // 表示渲染结束信号
	std::vector<VkFence> inFlightFences; // 用于控制渲染通道一次执行只渲染一帧

	uint32_t currentFrames = 0; // 当前渲染的帧
	bool framebufferResized = false; //标记已发生调整大小
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
		createGraphicsPipline(); // 创建图形管线
		createFramebuffers(); //创建帧缓冲区
		createCommandPool(); // 创建命令池对象
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
		cleanSwapChain();
		// 销毁管道
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		// 销毁管道布局
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		// 销毁渲染通道
		vkDestroyRenderPass(device, renderPass, nullptr);
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
		createFramebuffers();
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

	// 检侧选择使用合适的物理设备
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice)
	{
		QueueFamilyIndices indices = findQueueFimalies(physicalDevice);
		bool deviceExtensionsSupported = checkDeviceExtensionSupport(physicalDevice);
		bool deviceSwapChainAdequate = false;
		if (deviceExtensionsSupported)
		{
			SwapChainSupportDetails details = querySwapchainSupport(physicalDevice);
			deviceSwapChainAdequate = details.isFormatsAndPresentModesEmpty();

		}
		return indices.isComplete() && deviceExtensionsSupported && (!deviceSwapChainAdequate);
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

	// 使用赋分制计算，来决定使用哪个物理设备
	int rateDeviceSuitiability(VkPhysicalDevice physicalDeivce)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDeivce, &deviceProperties);
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDeivce, &deviceFeatures);
		int score = 0;
		// 检测该物理设备是否位独立显卡
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000;
		// 纹理图像的最大数量直接影响图片质量
		score += deviceProperties.limits.maxImageDimension2D;
		if (!deviceFeatures.geometryShader) // 不具有几何着色功能的直接pass
			return 0;
		return score;
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
				break;
			}
		}
		if(physicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Failed to find a suitable GPU!");
		// -- 使用赋分方式选择物理设备 --
		//std::multimap<int, VkPhysicalDevice> ratePhysicalDevices;
		//for (const auto& device : physicalDevices)
		//{
			// 该map的元素存储方式：increment ordered
			//ratePhysicalDevices.insert(std::make_pair(rateDeviceSuitiability(device), device) );
		//}
		//if (ratePhysicalDevices.rbegin()->first > 0) {
		//	physicalDevice = ratePhysicalDevices.rbegin()->second;
		//}
		//else
			//throw std::runtime_error("Failed to find a suitable GPU!");
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
			std::cerr << "validation layer:\t" << pCallbackData->pMessage << std::endl;
		}
		else
		{
			std::cerr << "validation layer:\t" << pCallbackData->pMessage << std::endl;
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
		// 创建逻辑设备的信息结构体
		VkDeviceCreateInfo deviceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()), // 设定选择的队列族相关信息
		.pQueueCreateInfos = queueCreateInfos.data(),
		.pEnabledFeatures = &deviceFatures // 逻辑设备具有与物理设备相同的功能
		};
		// 通过添加pickuPphysicalDevice函数，都会在接下来启用指定扩展层和验证层
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

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities; // 交换链的基本表面功能
		std::vector<VkSurfaceFormatKHR> formats; // 交换链的表面格式功能
		std::vector<VkPresentModeKHR> presentModes; // 交换链的展现模式
		bool isFormatsAndPresentModesEmpty() const
		{
			return formats.empty() && presentModes.empty();
		}
	};
	void createSurface() // surface同逻辑设备创建时自动加载，此处使用glfwCreateSurface函数即可
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create window surface!");
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

	// 选择表面格式的format具有指定类型的颜色通道，colorspace具有指定类型的颜色空间
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
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
	void createImageView()
	{
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); ++i)
		{
			VkImageViewCreateInfo swapchainCreateInfo{};
			swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			// 指向交换链图像
			swapchainCreateInfo.image = swapChainImages[i];
			swapchainCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // 该视图类型
			swapchainCreateInfo.format = swapChainImageFormat; // 视图格式与图像格式相同
			// 指定颜色混合通道类型
			swapchainCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			swapchainCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			swapchainCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			swapchainCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			// 设置颜射目标、纹理的mipmap层级
			swapchainCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			swapchainCreateInfo.subresourceRange.baseMipLevel = 0;
			swapchainCreateInfo.subresourceRange.levelCount = 1;
			swapchainCreateInfo.subresourceRange.baseArrayLayer = 0;
			swapchainCreateInfo.subresourceRange.layerCount = 1;
			if (vkCreateImageView(device, &swapchainCreateInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create image views!");
		}
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
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		// 在渲染前和选然后如何处理附件中的数据
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // 在开始时将值清除为常数
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // 渲染内容将存储在内存中，稍后读取
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // 不使用模板缓冲区，将不关心
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// 设置初始图像布局和渲染处理后图像布局
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // 开始时不关心图像布局
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 选然后准备好使用交换链
		// 子通道和附件引用：引用的附件索引，引用附件的图像布局类型
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; // 附件数组的索引，因为本处只是用了一个附件，所以索引为0
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // 表示将附件作为颜色缓冲区布局
		// 创建子通道：指明子通道类型，引用附件个数
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // 表面为用作图形子通道
		subpass.colorAttachmentCount = 1; // 附件个数
		subpass.pColorAttachments = &colorAttachmentRef; // 颜色附件对象
		// 子通道依赖性
		VkSubpassDependency dependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL, // 该值指渲染通道之前或之后的隐式子通道
			.dstSubpass = 0, // 子通道索引
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0
		};
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		// 渲染通道的创建：指明附件，子通道, 子通道依赖性
		VkRenderPassCreateInfo renderPassCreateInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorAttachment,
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

		// 动态状态Dynamic State：启用视口和裁剪矩形
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR // 可修改裁剪矩形大小
		};
		VkPipelineDynamicStateCreateInfo dynamicCreateInfo{};
		dynamicCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicCreateInfo.pDynamicStates = dynamicStates.data();
		// 顶点输入Vertex Input
		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
		vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;
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
		rasterizeCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // 正面顶点顺序
		rasterizeCreateInfo.depthBiasEnable = VK_FALSE; // 斜率偏置
		// 多重采样multiSampling
		VkPipelineMultisampleStateCreateInfo multismapleCreateInfo{};
		multismapleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multismapleCreateInfo.sampleShadingEnable = VK_FALSE; // 样本着色
		multismapleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 每个像素的样本数
		multismapleCreateInfo.minSampleShading = 1.0f;  // 每个像素的最小样本着色比例
		multismapleCreateInfo.pSampleMask = nullptr; // 哪些样本有效
		multismapleCreateInfo.alphaToCoverageEnable = VK_FALSE; // Alpha到Coverage
		multismapleCreateInfo.alphaToOneEnable = VK_FALSE; // Alpha到One
		// 颜色混合Color blending
		// 定义颜色附件，指定每个颜色附件的混合行为：即颜色/透明度混合因子,颜色/透明度混合方式
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE; // 不启用颜色混合
		// 引用所有帧缓冲区的结构数组
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.attachmentCount = 1;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;
		// 管道布局
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 0, // 描述集布局数
		.pSetLayouts = nullptr, // 设置描述集布局数组的指针
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
		.pDepthStencilState = nullptr,
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
	// Drawing阶段
	// 帧缓冲区
	void createFramebuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); ++i)
		{
			VkImageView attachment[] = {
				swapChainImageViews[i]
			};
			VkFramebufferCreateInfo framebufferCreateInfo{ 
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = renderPass, // 指定的渲染通道
				.attachmentCount = 1,
				.pAttachments = attachment, // 指定图像视图作为附件
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
		// 表示颜色附件的初始清除值
		VkClearValue clearColor = { {{0.0f,0.0f, 0.0f,1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		// 开启渲染通道
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
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
		// 发出绘制三角形指令
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		// 结束渲染通道执行
		vkCmdEndRenderPass(commandBuffer);
		// 结束记录命令缓冲区
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to end the command buffer!");
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
		// 将渲染结果提交回交换链
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