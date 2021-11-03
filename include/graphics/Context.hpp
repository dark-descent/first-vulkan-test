#ifndef ENGINE_GRAPHICS_CONTEXT_HPP
#define ENGINE_GRAPHICS_CONTEXT_HPP

#include "framework.hpp"
#include "AbstractObject.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/SwapChain.hpp"
#include "graphics/Pipeline.hpp"

#include "Engine.hpp"

namespace NovaEngine
{
	class GraphicsManager;

	namespace Graphics
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

		typedef const std::vector<const char*>& ValidationLayers;
		typedef const std::vector<const char*>& DeviceExtensions;

		class Context : public AbstractObject<const char*, GLFWwindow*, ValidationLayers, DeviceExtensions>
		{
		public:
			static void getQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, QueueFamilyIndices* indices);

		private:
			static std::vector<VkLayerProperties> getAvailableLayers();
			static std::vector<VkExtensionProperties> getAvailableExtensions();
			static bool checkValidationLayerSuppport(const std::vector<const char*> validationLayers);
			static std::vector<const char*> getRequiredExtensions();

			VkInstance instance_;
			VkSurfaceKHR surface_;
			PhysicalDevice physicalDevice_;
			VkDevice logicalDevice_;
			VkQueue graphicsQueue_;
			VkQueue presentQueue_;
			SwapChain swapChain_;
			VkRenderPass renderPass_;
			Pipeline pipeline_;
			GraphicsManager* graphicsManager_;

#ifdef DEBUG
			static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType,
				const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData
			);

			VkDebugUtilsMessengerEXT debugMessenger_ = nullptr;
#endif

		public:
			Context(GraphicsManager*);
			GraphicsManager* graphicsManager();

		protected:
			bool onInitialize(const char* gamenName, GLFWwindow* window, ValidationLayers validationLayers, DeviceExtensions deviceExtensions);
			bool onTerminate();
			bool createInstance(const char* gameName, ValidationLayers validationLayers);
			bool isVulkanSupported();
			bool createLogicalDeviceAndQueues(const std::vector<const char*>& validationLayers, const std::vector<const char*>& deviceExtensions);
			VkSurfaceFormatKHR getSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR getPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
			VkExtent2D getSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
			bool createRenderPass();
			bool createSwapChain();

#ifdef DEBUG
			bool setupDebugEnvironment();
			bool destroyDebugEnvironment();
#endif

			friend class Pipeline;
		};
	};

};


#endif

