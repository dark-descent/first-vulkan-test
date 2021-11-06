#ifndef ENGINE_GRAPHICS_CONTEXT_HPP
#define ENGINE_GRAPHICS_CONTEXT_HPP

#include "AbstractObject.hpp"
#include "framework.hpp"

namespace NovaEngine::Graphics
{
	class GraphicsManager;

	class Context : public AbstractObject<const char*, GLFWwindow*>
	{
	private:
		static const char* defaultAppName;

		GLFWwindow* window_;
		VkInstance instance_;

		Context() : AbstractObject(),
			window_(nullptr),
			instance_(VK_NULL_HANDLE),
			debugMessenger_(VK_NULL_HANDLE)
		{};

	protected:
		bool onInitialize(const char*, GLFWwindow* window);
		bool onTerminate();

		static VkInstance createVulkanInstance(const char* appName = Context::defaultAppName);
		static std::vector<const char*>& getRequiredExtensions();

private:
#ifdef NDEBUG
		VkDebugUtilsMessengerEXT debugMessenger_;

		VkDebugUtilsMessengerEXT createDebugMessenger();
		void destroyDebugMessenger();
		
		static std::vector<const char*> validationLayers;
		static bool hasValidationLayerSupport();
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
#endif

		friend class NovaEngine::Graphics::GraphicsManager;
	};
};

#endif
