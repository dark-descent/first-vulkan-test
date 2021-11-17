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
		for(auto& ctx : contexts_)
			ctx.destroy();
		return true;
	}

	bool GraphicsManager::initializeDevice(const VkSurfaceKHR& surface)
	{
		if (!isDeviceInitialized_ && (surface != VK_NULL_HANDLE))
		{
			physicalDevice_ = VkFactory::pickPhysicalDevice(instance_, surface, defaultExtensions);
			queueFamilyIndices_ = VkUtils::findQueueFamilies(physicalDevice_, surface);
			swapChainSupportDetails_ = VkUtils::querySwapChainSupport(physicalDevice_, surface);
			surfaceFormat_ = VkUtils::chooseSwapSurfaceFormat(swapChainSupportDetails_.formats);
			device_ = VkFactory::createDevice(physicalDevice_, queueFamilyIndices_, defaultExtensions, defaultLayers);

			isDeviceInitialized_ = true;

			return true;
		}
		return false;
	}

	Context* GraphicsManager::createContext(GLFWwindow* window)
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
		contexts_[index].initialize(this, window, surface, nullptr);
		return &contexts_[index];
	}
};
