#ifndef ENGINE_GRAPHICS_GRAPHICS_MANAGER_HPP
#define ENGINE_GRAPHICS_GRAPHICS_MANAGER_HPP

#include "SubSystem.hpp"
#include "framework.hpp"
#include "graphics/VkFactory.hpp"
#include "graphics/Context.hpp"

namespace NovaEngine::Graphics
{
	struct GraphicsConfig
	{

	};

	class GraphicsManager : public SubSystem<GraphicsConfig*>
	{
	private:
		static std::vector<const char*> defaultLayers;
		static const std::vector<const char*> defaultExtensions;

		VkDebugUtilsMessengerEXT debugExt;
		VkInstance instance_;
		VkPhysicalDevice physicalDevice_;
		VkDevice device_;

		QueueFamilyIndices queueFamilyIndices_;
		SwapChainSupportDetails swapChainSupportDetails_;
		VkSurfaceFormatKHR surfaceFormat_;

		std::vector<Context> contexts_;
		bool isDeviceInitialized_;

		ENGINE_SUB_SYSTEM_CTOR(GraphicsManager),
			debugExt(VK_NULL_HANDLE),
			instance_(VK_NULL_HANDLE),
			physicalDevice_(VK_NULL_HANDLE),
			device_(VK_NULL_HANDLE),
			queueFamilyIndices_(),
			swapChainSupportDetails_(),
			surfaceFormat_(),
			contexts_(),
			isDeviceInitialized_(false)
		{};

		bool initializeDevice(const VkSurfaceKHR& surface);

	protected:
		bool onInitialize(GraphicsConfig* config);
		bool onTerminate();

		Context* createContext(GLFWwindow* window);

		friend class Context;
		friend class SwapChain;

	};
};

#endif
