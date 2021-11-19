#include "graphics/GraphicsManager.hpp"

#include "Engine.hpp"

namespace NovaEngine::Graphics
{
	std::vector<const char*> GraphicsManager::defaultLayers = {
#ifdef DEBUG
			"VK_LAYER_KHRONOS_validation",
#endif
	};

	const std::vector<const char*> GraphicsManager::defaultExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	bool GraphicsManager::onInitialize(GraphicsConfig* config)
	{
		instance_ = VkFactory::createInstance(defaultLayers, &debugExt);

		return true;
	}

	bool GraphicsManager::onTerminate()
	{
		for (auto& ctx : contexts_)
			ctx.destroy();
		return true;
	}

	bool GraphicsManager::initializeDevice(const VkSurfaceKHR& surface)
	{
		if (!isDeviceInitialized_ && (surface != VK_NULL_HANDLE))
		{
			printf("initialize device...\n");
			physicalDevice_ = VkFactory::pickPhysicalDevice(instance_, surface, defaultExtensions);
			printf("find queue families...\n");
			queueFamilies_ = VkUtils::findQueueFamilies(physicalDevice_, surface);
			printf("create device...\n");
			device_ = VkFactory::createDevice(physicalDevice_, queueFamilies_, defaultExtensions, defaultLayers);
			printf("device created...\n");
			isDeviceInitialized_ = true;

			return true;
		}
		return false;
	}

	VkQueue GraphicsManager::getGraphicsQueue()
	{
		QueueFamilies::QueueInfo& info = queueFamilies_.graphics.value();
		VkQueue queue;
		vkGetDeviceQueue(device_, info.index, graphicsQueuePtr_, &queue);
		graphicsQueuePtr_ = (graphicsQueuePtr_ + 1) % info.maxQueues;
		printf("GraphicsManager::getGraphicsQueue()\n");
		return queue;
	}

	VkQueue GraphicsManager::getPresentationQueue()
	{
		QueueFamilies::QueueInfo& info = queueFamilies_.present.value();
		VkQueue queue;
		vkGetDeviceQueue(device_, info.index, presentationQueuePtr_, &queue);
		presentationQueuePtr_ = (presentationQueuePtr_ + 1) % info.maxQueues;
		printf("GraphicsManager::getPresentationQueue()\n");
		return queue;
	}


	Context* GraphicsManager::createContext(GLFWwindow* window, NovaEngine::Graphics::ContextOptions *options)
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		if (!isDeviceInitialized_)
		{
			surface = VkFactory::createSurface(instance_, window);
			if (!initializeDevice(surface))
				throw std::runtime_error("Could not create context!");
		}

		size_t index = contexts_.size();
		contexts_.push_back(Context());
		contexts_[index].initialize(this, window, getGraphicsQueue(), getPresentationQueue(), surface, options);
		return &contexts_[index];
	}
};
