#include "graphics/Context.hpp"
#include "graphics/VkFactory.hpp"
#include "graphics/GraphicsManager.hpp"

namespace NovaEngine::Graphics
{
	bool Context::onInitialize(GraphicsManager* manager, GLFWwindow* window, VkSurfaceKHR surface, SwapChainOptions* swapChainOptions)
	{
		manager_ = manager;
		window_ = window;
		surface_ = surface != VK_NULL_HANDLE ? surface : VkFactory::createSurface(manager->instance_, window);
		renderPass_ = VkFactory::createRenderPass(manager_->device_, manager_->surfaceFormat_.format);
		return swapChain_.initialize(this, swapChainOptions);
	}

	void Context::destroy()
	{
		swapChain_.destroy();
		vkDestroySurfaceKHR(manager_->instance_, surface_, nullptr);
	}
};
