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
	VkSurfaceKHR surface; // ���ڱ��洴��
	VkSwapchainKHR swapchain; // ����������
	std::vector<VkImage> swapChainImages; // ������ͼ��
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;// ������ͼ����ͼ����
	// ��Ⱦͨ�����ò���
	VkRenderPass renderPass; // ��Ⱦͨ������
	VkPipelineLayout pipelineLayout; // �ܵ����ֶ���
	VkPipeline graphicsPipeline;	// ͼ�ιܵ�
	// ֡����������
	std::vector<VkFramebuffer> swapChainFramebuffers;
	// �����������
	VkCommandPool commandPool;  // ����ض���
	std::vector<VkCommandBuffer> commandBuffers; // �����������
	// ͬ������
	std::vector<VkSemaphore> imageAvailableSemaphores; // ��ʾͼ������ź�
	std::vector<VkSemaphore> renderFinishedSemaphores; // ��ʾ��Ⱦ�����ź�
	std::vector<VkFence> inFlightFences; // ���ڿ�����Ⱦͨ��һ��ִ��ֻ��Ⱦһ֡

	uint32_t currentFrames = 0; // ��ǰ��Ⱦ��֡
	bool framebufferResized = false; //����ѷ���������С
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
		createGraphicsPipline(); // ����ͼ�ι���
		createFramebuffers(); //����֡������
		createCommandPool(); // ��������ض���
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
		// ���ٹܵ�
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		// ���ٹܵ�����
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		// ������Ⱦͨ��
		vkDestroyRenderPass(device, renderPass, nullptr);
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
		createFramebuffers();
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

	// ���ѡ��ʹ�ú��ʵ������豸
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

	// ʹ�ø����Ƽ��㣬������ʹ���ĸ������豸
	int rateDeviceSuitiability(VkPhysicalDevice physicalDeivce)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(physicalDeivce, &deviceProperties);
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDeivce, &deviceFeatures);
		int score = 0;
		// ���������豸�Ƿ�λ�����Կ�
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000;
		// ����ͼ����������ֱ��Ӱ��ͼƬ����
		score += deviceProperties.limits.maxImageDimension2D;
		if (!deviceFeatures.geometryShader) // �����м�����ɫ���ܵ�ֱ��pass
			return 0;
		return score;
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
				break;
			}
		}
		if(physicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Failed to find a suitable GPU!");
		// -- ʹ�ø��ַ�ʽѡ�������豸 --
		//std::multimap<int, VkPhysicalDevice> ratePhysicalDevices;
		//for (const auto& device : physicalDevices)
		//{
			// ��map��Ԫ�ش洢��ʽ��increment ordered
			//ratePhysicalDevices.insert(std::make_pair(rateDeviceSuitiability(device), device) );
		//}
		//if (ratePhysicalDevices.rbegin()->first > 0) {
		//	physicalDevice = ratePhysicalDevices.rbegin()->second;
		//}
		//else
			//throw std::runtime_error("Failed to find a suitable GPU!");
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
			std::cerr << "validation layer:\t" << pCallbackData->pMessage << std::endl;
		}
		else
		{
			std::cerr << "validation layer:\t" << pCallbackData->pMessage << std::endl;
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
		// �����߼��豸����Ϣ�ṹ��
		VkDeviceCreateInfo deviceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()), // �趨ѡ��Ķ����������Ϣ
		.pQueueCreateInfos = queueCreateInfos.data(),
		.pEnabledFeatures = &deviceFatures // �߼��豸�����������豸��ͬ�Ĺ���
		};
		// ͨ�����pickuPphysicalDevice�����������ڽ���������ָ����չ�����֤��
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

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities; // �������Ļ������湦��
		std::vector<VkSurfaceFormatKHR> formats; // �������ı����ʽ����
		std::vector<VkPresentModeKHR> presentModes; // ��������չ��ģʽ
		bool isFormatsAndPresentModesEmpty() const
		{
			return formats.empty() && presentModes.empty();
		}
	};
	void createSurface() // surfaceͬ�߼��豸����ʱ�Զ����أ��˴�ʹ��glfwCreateSurface��������
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create window surface!");
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

	// ѡ������ʽ��format����ָ�����͵���ɫͨ����colorspace����ָ�����͵���ɫ�ռ�
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
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
	void createImageView()
	{
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); ++i)
		{
			VkImageViewCreateInfo swapchainCreateInfo{};
			swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			// ָ�򽻻���ͼ��
			swapchainCreateInfo.image = swapChainImages[i];
			swapchainCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // ����ͼ����
			swapchainCreateInfo.format = swapChainImageFormat; // ��ͼ��ʽ��ͼ���ʽ��ͬ
			// ָ����ɫ���ͨ������
			swapchainCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			swapchainCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			swapchainCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			swapchainCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			// ��������Ŀ�ꡢ�����mipmap�㼶
			swapchainCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			swapchainCreateInfo.subresourceRange.baseMipLevel = 0;
			swapchainCreateInfo.subresourceRange.levelCount = 1;
			swapchainCreateInfo.subresourceRange.baseArrayLayer = 0;
			swapchainCreateInfo.subresourceRange.layerCount = 1;
			if (vkCreateImageView(device, &swapchainCreateInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create image views!");
		}
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
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		// ����Ⱦǰ��ѡȻ����δ������е�����
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // �ڿ�ʼʱ��ֵ���Ϊ����
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // ��Ⱦ���ݽ��洢���ڴ��У��Ժ��ȡ
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // ��ʹ��ģ�建��������������
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// ���ó�ʼͼ�񲼾ֺ���Ⱦ�����ͼ�񲼾�
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // ��ʼʱ������ͼ�񲼾�
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // ѡȻ��׼����ʹ�ý�����
		// ��ͨ���͸������ã����õĸ������������ø�����ͼ�񲼾�����
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; // �����������������Ϊ����ֻ������һ����������������Ϊ0
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // ��ʾ��������Ϊ��ɫ����������
		// ������ͨ����ָ����ͨ�����ͣ����ø�������
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // ����Ϊ����ͼ����ͨ��
		subpass.colorAttachmentCount = 1; // ��������
		subpass.pColorAttachments = &colorAttachmentRef; // ��ɫ��������
		// ��ͨ��������
		VkSubpassDependency dependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL, // ��ֵָ��Ⱦͨ��֮ǰ��֮�����ʽ��ͨ��
			.dstSubpass = 0, // ��ͨ������
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0
		};
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		// ��Ⱦͨ���Ĵ�����ָ����������ͨ��, ��ͨ��������
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

		// ��̬״̬Dynamic State�������ӿںͲü�����
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR // ���޸Ĳü����δ�С
		};
		VkPipelineDynamicStateCreateInfo dynamicCreateInfo{};
		dynamicCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicCreateInfo.pDynamicStates = dynamicStates.data();
		// ��������Vertex Input
		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
		vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;
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
		rasterizeCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // ���涥��˳��
		rasterizeCreateInfo.depthBiasEnable = VK_FALSE; // б��ƫ��
		// ���ز���multiSampling
		VkPipelineMultisampleStateCreateInfo multismapleCreateInfo{};
		multismapleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multismapleCreateInfo.sampleShadingEnable = VK_FALSE; // ������ɫ
		multismapleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // ÿ�����ص�������
		multismapleCreateInfo.minSampleShading = 1.0f;  // ÿ�����ص���С������ɫ����
		multismapleCreateInfo.pSampleMask = nullptr; // ��Щ������Ч
		multismapleCreateInfo.alphaToCoverageEnable = VK_FALSE; // Alpha��Coverage
		multismapleCreateInfo.alphaToOneEnable = VK_FALSE; // Alpha��One
		// ��ɫ���Color blending
		// ������ɫ������ָ��ÿ����ɫ�����Ļ����Ϊ������ɫ/͸���Ȼ������,��ɫ/͸���Ȼ�Ϸ�ʽ
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE; // ��������ɫ���
		// ��������֡�������Ľṹ����
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
		// �ܵ�����
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 0, // ������������
		.pSetLayouts = nullptr, // �������������������ָ��
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
		.pDepthStencilState = nullptr,
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
	// Drawing�׶�
	// ֡������
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
				.renderPass = renderPass, // ָ������Ⱦͨ��
				.attachmentCount = 1,
				.pAttachments = attachment, // ָ��ͼ����ͼ��Ϊ����
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
		// ��ʾ��ɫ�����ĳ�ʼ���ֵ
		VkClearValue clearColor = { {{0.0f,0.0f, 0.0f,1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		// ������Ⱦͨ��
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
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
		// ��������������ָ��
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		// ������Ⱦͨ��ִ��
		vkCmdEndRenderPass(commandBuffer);
		// ������¼�������
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to end the command buffer!");
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
		// ����Ⱦ����ύ�ؽ�����
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