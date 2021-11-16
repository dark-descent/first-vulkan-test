#ifndef NOVA_ENGINE_GRAPHICS_VK_UTILS_HPP
#define NOVA_ENGINE_GRAPHICS_VK_UTILS_HPP

#include "framework.hpp"

namespace NovaEngine::Graphics
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	namespace VkUtils
	{
		bool supportLayers(const std::vector<const char*>& layers);
		std::vector<const char*> getRequiredExtensions();

		VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator = nullptr);

		bool checkDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char*>& extensions);
		bool isDeviceSuitable(const VkPhysicalDevice& dev, const VkSurfaceKHR& surface, const std::vector<const char*>& extensions);
		QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
		VkQueue getGraphicsQueue(const VkDevice& device, const QueueFamilyIndices& indices);
		VkQueue getPresentationQueue(const VkDevice& device, const QueueFamilyIndices& indices);

		SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow* window);		
	
		VkShaderModule createShaderModule(const VkDevice& device, const std::vector<char>& code);

		uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};
};

#endif
