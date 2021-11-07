#include "graphics/Context.hpp"

#include "Engine.hpp"
#include "Logger.hpp"

#define VK_INIT(var, expr) { var = expr(); if(var == VK_NULL_HANDLE) { Logger::get()->error(#var " = " #expr "() FAILED!"); return false; } }
#define VK_INIT_ARGS(var, expr, args) { var = expr(args); if(var == VK_NULL_HANDLE) { Logger::get()->error(#var " = " #expr "(" #args ") FAILED!"); return false; } }
#define VK_CHECK(expr) { if(expr != VK_SUCCESS) { Logger::get()->error(#expr " Failed!"); return VK_NULL_HANDLE; } }
#define INIT_GFX_OBJ(obj) { if(!obj.initialize()) { Logger::get()->error(#obj ".initialize() Failed!"); return false; } }
#define INIT_GFX_OBJ_ARGS(obj, args) { if(!obj.initialize(args)) { Logger::get()->error(#obj ".initialize("#args") Failed!"); return false; } }

namespace NovaEngine::Graphics
{
	const char* Context::defaultAppName = "Missing App Name";

#ifdef DEBUG
	std::vector<const char*> Context::validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
#endif

	bool Context::onInitialize(const char* name, GLFWwindow* window)
	{
		VK_INIT_ARGS(instance_, createVulkanInstance, name);

		if (!isVulkanSupported())
			return false;

		ON_DEBUG(VK_INIT(debugMessenger_, createDebugMessenger));

		surface_ = createSurface(instance_, window);

		if (surface_ == VK_NULL_HANDLE)
		{
			Logger::get()->error("Failed to create vulkan surface!");
			return false;
		}

		INIT_GFX_OBJ_ARGS(physicalDevice_, nullptr);
		INIT_GFX_OBJ_ARGS(device_, nullptr);
		INIT_GFX_OBJ(swapChain_);

		return true;
	}

	bool Context::onTerminate()
	{
		swapChain_.terminate();
		device_.terminate();
		physicalDevice_.terminate();
		vkDestroySurfaceKHR(instance_, surface_, nullptr);
		ON_DEBUG(destroyDebugMessenger());
		vkDestroyInstance(instance_, nullptr);
		return true;
	}


	GLFWwindow* Context::window() { return window_; }
	VkInstance& Context::instance() { return instance_; }
	PhysicalDevice& Context::physicalDevice() { return physicalDevice_; }
	Device& Context::device() { return device_; }
	VkSurfaceKHR& Context::surface() { return surface_; }
	SwapChain& Context::swapChain() { return swapChain_; }

	bool Context::isVulkanSupported()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
		return deviceCount != 0;
	}

#pragma region Static Create Methods 

	std::vector<const char*>& Context::getRequiredExtensions()
	{
		static std::optional<std::vector<const char*>> requiredExtensions;

		if (requiredExtensions.has_value())
			return requiredExtensions.value();

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		requiredExtensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef DEBUG
		requiredExtensions.value().push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		return requiredExtensions.value();
	}

	VkInstance Context::createVulkanInstance(const char* appName)
	{
#ifdef DEBUG
		if (!hasValidationLayerSupport())
			return VK_NULL_HANDLE;
#endif

		uint32_t glfwExtensionCount = 0;

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = getRequiredExtensions().size();
		createInfo.ppEnabledExtensionNames = getRequiredExtensions().data();
#ifdef DEBUG
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif

		VkInstance instance;
		VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));
		return instance;
	}


	VkSurfaceKHR Context::createSurface(VkInstance instance, GLFWwindow* window)
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) == VK_SUCCESS)
			return surface;
		return VK_NULL_HANDLE;

	}

#ifdef DEBUG

	bool Context::hasValidationLayerSupport()
	{
		static std::optional<bool> hasLayerSupport;

		if (hasLayerSupport.has_value())
			return hasLayerSupport.value();

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const VkLayerProperties& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				hasLayerSupport = false;
				return false;
			}
		}

		hasLayerSupport = true;

		return true;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Context::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		if(strncmp(pCallbackData->pMessage, "Device Extension: ", 18) == 0)
			return VK_FALSE;

		if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT || messageSeverity == messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			Logger::get()->info(pCallbackData->pMessage);
		if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			Logger::get()->warn(pCallbackData->pMessage);
		if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			Logger::get()->error(pCallbackData->pMessage);

		return VK_FALSE;
	}

	VkDebugUtilsMessengerEXT Context::createDebugMessenger()
	{
		static PFN_vkCreateDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT"));

		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;

		VkDebugUtilsMessengerEXT debugMesenger;

		VK_CHECK(func(instance_, &createInfo, nullptr, &debugMesenger));

		return debugMesenger;
	}

	void Context::destroyDebugMessenger()
	{
		PFN_vkDestroyDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT"));

		if (func != nullptr)
			func(instance_, debugMessenger_, nullptr);
	}

#endif

#pragma endregion

};
