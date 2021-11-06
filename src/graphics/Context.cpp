#include "graphics/Context.hpp"

#include "Engine.hpp"
#include "Logger.hpp"

#define VK_INIT(var, expr) { var = expr(); if(var == VK_NULL_HANDLE) { Logger::get()->error(#var " = " #expr "() FAILED!"); return false; } }
#define VK_INIT_ARGS(var, expr, args) { var = expr(args); if(var == VK_NULL_HANDLE) { Logger::get()->error(#var " = " #expr "(" #args ") FAILED!"); return false; } }
#define VK_CHECK(expr) { if(expr != VK_SUCCESS) { Logger::get()->error(#expr " Failed!"); return VK_NULL_HANDLE; } }

namespace NovaEngine::Graphics
{
	const char* Context::defaultAppName = "Missing App Name";

#ifdef NDEBUG
	std::vector<const char*> Context::validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
#endif

	bool Context::onInitialize(const char* name, GLFWwindow* window)
	{
		VK_INIT_ARGS(instance_, createVulkanInstance, name);

		ON_DEBUG(VK_INIT(debugMessenger_, createDebugMessenger));

		return true;
	}

	bool Context::onTerminate()
	{
		ON_DEBUG(destroyDebugMessenger());
		vkDestroyInstance(instance_, nullptr);
		return true;
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

#ifdef NDEBUG
		requiredExtensions.value().push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		return requiredExtensions.value();
	}

	VkInstance Context::createVulkanInstance(const char* appName)
	{
#ifdef NDEBUG
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
#ifdef NDEBUG
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif

		VkInstance instance;
		VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));
		return instance;
	}

#ifdef NDEBUG

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
		Logger::get()->info("validation layer: ", pCallbackData->pMessage);
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
