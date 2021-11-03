#include "graphics/Context.hpp"
#include "ConfigManager.hpp"
#include "framework.hpp"
#include "GraphicsManager.hpp"

namespace NovaEngine::Graphics
{
	std::vector<VkLayerProperties> Context::getAvailableLayers()
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, VK_NULL_HANDLE);
		std::vector<VkLayerProperties> layers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
		return layers;
	}

	std::vector<VkExtensionProperties> Context::getAvailableExtensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, VK_NULL_HANDLE);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
		return extensions;
	}

	bool Context::checkValidationLayerSuppport(const std::vector<const char*> validationLayers)
	{
		std::vector<VkLayerProperties> layers = getAvailableLayers();

		for (const char* name : validationLayers)
		{
			bool layerFound = false;

			for (const VkLayerProperties& props : layers)
			{
				if (strcmp(props.layerName, name) == 0)
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

	std::vector<const char*> Context::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef DEBUG
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		return extensions;
	}

	void Context::getQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, QueueFamilyIndices* indices)
	{
		if (device == nullptr || surface == nullptr)
			return;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, VK_NULL_HANDLE);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices->graphicsFamily = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
				indices->presentFamily = i;

			if (indices->isComplete())
				return;

			i++;
		}
	}


	Context::Context(GraphicsManager* graphicsManager) :
		instance_(VK_NULL_HANDLE),
		surface_(VK_NULL_HANDLE),
		physicalDevice_(),
		logicalDevice_(VK_NULL_HANDLE),
		graphicsQueue_(VK_NULL_HANDLE),
		presentQueue_(VK_NULL_HANDLE),
		swapChain_(),
		renderPass_(VK_NULL_HANDLE),
		pipeline_(this),
		graphicsManager_(graphicsManager)
	{

	}

	GraphicsManager* Context::graphicsManager() { return graphicsManager_; }

	bool Context::onInitialize(const char* gameName, GLFWwindow* window, ValidationLayers validationLayers, DeviceExtensions deviceExtensions)
	{
		CHECK_OR_RETURN(window, "No window passed!");
		CHECK_OR_RETURN(createInstance(gameName, validationLayers), "Failed to create a Vulkan Instance!");
		CHECK_OR_RETURN(glfwCreateWindowSurface(instance_, window, VK_NULL_HANDLE, &surface_) == VK_SUCCESS, "Failed to create a render surface!");
		CHECK_OR_RETURN(isVulkanSupported(), "Vulkan is not supported!");
		ON_DEBUG(setupDebugEnvironment());
		CHECK_OR_RETURN(physicalDevice_.initialize(instance_, surface_, deviceExtensions), "Could not create physical device!");
		CHECK_OR_RETURN(createLogicalDeviceAndQueues(validationLayers, deviceExtensions), "Failed to create logical device!");
		CHECK_OR_RETURN(swapChain_.initialize(window, surface_, *physicalDevice_, logicalDevice_), "Failed to create swapchain!");
		CHECK_OR_RETURN(createRenderPass(), "Fauled to create the render pass!");
		CHECK_OR_RETURN(pipeline_.initialize(), "Failed to create the pipeline!");
		return true;
	}

	bool Context::onTerminate()
	{
		ON_DEBUG(destroyDebugEnvironment());

		pipeline_.terminate();
		vkDestroyRenderPass(logicalDevice_, renderPass_, nullptr);
		swapChain_.terminate();
		physicalDevice_.terminate();
		vkDestroyDevice(logicalDevice_, VK_NULL_HANDLE);
		vkDestroySurfaceKHR(instance_, surface_, VK_NULL_HANDLE);
		vkDestroyInstance(instance_, VK_NULL_HANDLE);

		return true;
	}

	bool Context::createInstance(const char* gameName, ValidationLayers validationLayers)
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = gameName == nullptr ? "Game" : gameName;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Nova Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> requiredExtensions = getRequiredExtensions();

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();
#ifdef DEBUG
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = 0;
#endif

		return vkCreateInstance(&createInfo, VK_NULL_HANDLE, &instance_) == VK_SUCCESS;
	}

	bool Context::isVulkanSupported()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance_, &deviceCount, VK_NULL_HANDLE);
		return deviceCount != 0;
	}

	bool Context::createLogicalDeviceAndQueues(const std::vector<const char*>& validationLayers, const std::vector<const char*>& deviceExtensions)
	{
		QueueFamilyIndices indices;
		getQueueFamilies(*physicalDevice_, surface_, &indices);

		float queuePriority = 1.0f;

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifdef DEBUG
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif

		if (vkCreateDevice(*physicalDevice_, &createInfo, nullptr, &logicalDevice_) != VK_SUCCESS)
			return false;

		if (indices.isComplete())
		{
			vkGetDeviceQueue(logicalDevice_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
			vkGetDeviceQueue(logicalDevice_, indices.presentFamily.value(), 0, &presentQueue_);
		}

		return true;
	}

#ifdef DEBUG
	VKAPI_ATTR VkBool32 VKAPI_CALL Context::vulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	) {
		// std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	bool Context::setupDebugEnvironment()
	{
		std::vector<VkLayerProperties> layers = getAvailableLayers();
		printf("Available vulkan layers:\n");
		for (const VkLayerProperties& l : layers)
			printf("    %s\n", l.layerName);

		std::vector<VkExtensionProperties> extensions = getAvailableExtensions();
		printf("vulkan extensions:\n");
		for (const VkExtensionProperties& ext : extensions)
			printf("    %s\n", ext.extensionName);

		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = vulkanDebugCallback;
		createInfo.pUserData = nullptr;

		PFN_vkCreateDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT"));
		if (func != nullptr)
			return func(instance_, &createInfo, nullptr, &debugMessenger_) == VK_SUCCESS;
		return false;
	}

	bool Context::destroyDebugEnvironment()
	{
		PFN_vkDestroyDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT"));

		if (func != nullptr && debugMessenger_ != nullptr)
		{
			func(instance_, debugMessenger_, nullptr);
			return true;
		}

		return false;
	}
#endif

	bool Context::createRenderPass()
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapChain_.imageFormat_;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		return vkCreateRenderPass(logicalDevice_, &renderPassInfo, nullptr, &renderPass_) == VK_SUCCESS;
	}
};
