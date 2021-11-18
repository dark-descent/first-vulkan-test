#ifndef NOVA_ENGINE_GRAPHICS_VK_UTILS_HPP
#define NOVA_ENGINE_GRAPHICS_VK_UTILS_HPP

#include "framework.hpp"

namespace NovaEngine::Graphics
{
	struct QueueFamilies
	{
		struct QueueInfo
		{
			uint32_t index;
			uint32_t maxQueues;

			QueueInfo(uint32_t index = 0, uint32_t maxQueues = 0) : index(index), maxQueues(maxQueues) {}
		};

		std::optional<QueueInfo> graphics;
		std::optional<QueueInfo> present;

		QueueFamilies() : graphics(), present() {}

		bool isComplete()
		{
			return graphics.has_value() && present.has_value();
		}
	};
	
	struct SwapChainSupportDetails;

	namespace VkUtils
	{
		bool supportLayers(const std::vector<const char*>& layers);
		std::vector<const char*> getRequiredExtensions();

		VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator = nullptr);

		bool checkDeviceExtensionSupport(const VkPhysicalDevice& device, const std::vector<const char*>& extensions);
		bool isDeviceSuitable(const VkPhysicalDevice& dev, const VkSurfaceKHR& surface, const std::vector<const char*>& extensions);
		QueueFamilies findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
		
		SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow* window);		
	
		VkShaderModule createShaderModule(const VkDevice& device, const std::vector<char>& code);

		uint32_t findMemoryType(const VkPhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};
};

#endif
