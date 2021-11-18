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

		QueueFamilies queueFamilies_;

		std::vector<Context> contexts_;
		bool isDeviceInitialized_;

		size_t graphicsQueuePtr_;
		size_t presentationQueuePtr_;

		std::vector<VkQueue> graphicsQueues_;
		std::vector<VkQueue> presentationQueues_;

		ENGINE_SUB_SYSTEM_CTOR(GraphicsManager),
			debugExt(VK_NULL_HANDLE),
			instance_(VK_NULL_HANDLE),
			physicalDevice_(VK_NULL_HANDLE),
			device_(VK_NULL_HANDLE),
			queueFamilies_(),
			contexts_(),
			isDeviceInitialized_(false),
			graphicsQueuePtr_(0),
			presentationQueuePtr_(0),
			graphicsQueues_(),
			presentationQueues_()
		{};

		bool initializeDevice(const VkSurfaceKHR& surface);
		VkQueue getGraphicsQueue();
		VkQueue getPresentationQueue();

	protected:
		bool onInitialize(GraphicsConfig* config);
		bool onTerminate();

		Context* createContext(GLFWwindow* window);

		friend class Context;
		friend class SwapChain;

	};
};

#endif
