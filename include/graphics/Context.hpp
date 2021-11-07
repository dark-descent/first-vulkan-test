#ifndef ENGINE_GRAPHICS_CONTEXT_HPP
#define ENGINE_GRAPHICS_CONTEXT_HPP

#include "AbstractObject.hpp"
#include "framework.hpp"
#include "graphics/PhysicalDevice.hpp"
#include "graphics/Device.hpp"
#include "graphics/SwapChain.hpp"
#include "graphics/Pipeline.hpp"

namespace NovaEngine
{
	class Engine;
};

namespace NovaEngine::Graphics
{
	class GraphicsManager;

	class Context : public AbstractObject<Engine*, const char*, GLFWwindow*>
	{
	private:
		static const char* defaultAppName;

		Engine* engine_;

		GLFWwindow* window_;
		VkInstance instance_;
		PhysicalDevice physicalDevice_;
		Device device_;
		VkSurfaceKHR surface_;
		SwapChain swapChain_;
		VkRenderPass renderPass_;
		Pipeline pipeline_;

		Context() : AbstractObject(),
			engine_(nullptr),
			window_(nullptr),
			instance_(VK_NULL_HANDLE),
			physicalDevice_(this),
			device_(this),
			surface_(VK_NULL_HANDLE),
			swapChain_(this),
			renderPass_(VK_NULL_HANDLE),
			pipeline_(this),
			debugMessenger_(VK_NULL_HANDLE)
		{};

	public:
		Engine* engine();
		GLFWwindow* window();
		VkInstance& instance();
		PhysicalDevice& physicalDevice();
		Device& device();
		VkSurfaceKHR& surface();
		SwapChain& swapChain();
		VkRenderPass& renderPass();
		Pipeline& pipeline();

	protected:
		bool onInitialize(Engine*, const char*, GLFWwindow* window);
		bool onTerminate();
		bool isVulkanSupported();
		VkRenderPass createRenderPass();

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
