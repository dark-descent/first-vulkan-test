#ifndef ENGINE_GRAPHICS_CONTEXT_HPP
#define ENGINE_GRAPHICS_CONTEXT_HPP

#include "AbstractObject.hpp"
#include "framework.hpp"
#include "PhysicalDevice.hpp"
#include "Device.hpp"

namespace NovaEngine::Graphics
{
	class GraphicsManager;

	class Context : public AbstractObject<const char*, GLFWwindow*>
	{
	private:
		static const char* defaultAppName;

		GLFWwindow* window_;
		VkInstance instance_;
		PhysicalDevice physicalDevice_;
		Device device_;
		VkSurfaceKHR surface_;

		Context() : AbstractObject(),
			window_(nullptr),
			instance_(VK_NULL_HANDLE),
			physicalDevice_(this),
			device_(this),
			surface_(VK_NULL_HANDLE),
			debugMessenger_(VK_NULL_HANDLE)
		{};

	public:
		GLFWwindow* window();
		VkInstance& instance();
		PhysicalDevice& physicalDevice();
		Device& device();
		VkSurfaceKHR& surface();

	protected:
		bool onInitialize(const char*, GLFWwindow* window);
		bool onTerminate();
		bool isVulkanSupported();

		static VkInstance createVulkanInstance(const char* appName = Context::defaultAppName);
		static std::vector<const char*>& getRequiredExtensions();
		static VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window);

	private:
#ifdef DEBUG
		VkDebugUtilsMessengerEXT debugMessenger_;

		VkDebugUtilsMessengerEXT createDebugMessenger();
		void destroyDebugMessenger();

		static std::vector<const char*> validationLayers;
		static bool hasValidationLayerSupport();
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
#endif

		friend class NovaEngine::Graphics::GraphicsManager;
		friend class NovaEngine::Graphics::PhysicalDevice;
	};
};

#endif
