#include "graphics/Context.hpp"
#include "graphics/VkFactory.hpp"
#include "graphics/VkUtils.hpp"
#include "graphics/GraphicsManager.hpp"
#include "Logger.hpp"

namespace NovaEngine::Graphics
{
	ContextOptions Context::defaultOptions = {};

	void Context::onFrameBufferResizedCallback(GLFWwindow* window, int width, int height)
	{
		Context* ctx = reinterpret_cast<Context*>(glfwGetWindowUserPointer(window));
		ctx->didResize_ = true;
	}

	bool Context::onInitialize(GraphicsManager* manager, GLFWwindow* window, VkQueue graphicsQueue, VkQueue presentQueue, VkSurfaceKHR surface, ContextOptions* options)
	{
		if(options == nullptr)
			options = &defaultOptions;

		manager_ = manager;
		instance_ = manager->instance_;
		physicalDevice_ = manager->physicalDevice_;
		device_ = manager->device_;
		graphicsQueue_ = graphicsQueue;
		presentQueue_ = presentQueue;
		window_ = window;
		surface_ = surface != VK_NULL_HANDLE ? surface : VkFactory::createSurface(instance_, window);
		queueFamilies_ = VkUtils::findQueueFamilies(physicalDevice_, surface_);
		swapChainSupportDetails_ = VkUtils::querySwapChainSupport(physicalDevice_, surface_);
		surfaceFormat_ = VkUtils::chooseSwapSurfaceFormat(swapChainSupportDetails_.formats);
		renderPass_ = VkFactory::createRenderPass(device_, surfaceFormat_.format);
		swapChain_.initialize(this, options->swapChainOptions);
		commandPool_ = VkFactory::createCommandPool(physicalDevice_, device_, queueFamilies_.graphics.value().index);
		commandBuffers_ = VkFactory::createCommandBuffers(device_, commandPool_, swapChain_.imageCount_);
		syncObjects_ = VkFactory::createSyncObjects(device_, swapChain_, swapChain_.imageCount());
		clearColor_ = options->clearColor;

		glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
		glfwSetFramebufferSizeCallback(window, onFrameBufferResizedCallback);

		Logger::get()->info("Context initialized!\n");

		return true;
	}

	void Context::resizeSwapchain()
	{
		std::vector<VkFence> fences;

		for (const auto& f : syncObjects_.imagesInFlight)
			if (f != VK_NULL_HANDLE)
				fences.push_back(f);

		if (fences.size() > 0)
			vkWaitForFences(device_, fences.size(), fences.data(), VK_TRUE, UINT64_MAX);

		vkDestroyCommandPool(device_, commandPool_, nullptr);
		swapChain_.recreate();
		commandPool_ = VkFactory::createCommandPool(physicalDevice_, device_, queueFamilies_.graphics.value().index);
		commandBuffers_ = VkFactory::createCommandBuffers(device_, commandPool_, swapChain_.imageCount_);
	}

	void Context::destroy()
	{
		std::vector<VkFence> fences;

		for (const auto& f : syncObjects_.imagesInFlight)
			if (f != VK_NULL_HANDLE)
				fences.push_back(f);

		if (fences.size() > 0)
			vkWaitForFences(device_, fences.size(), fences.data(), VK_TRUE, UINT64_MAX);

		vkDestroyCommandPool(device_, commandPool_, nullptr);
		swapChain_.destroy();
		vkDestroySurfaceKHR(instance_, surface_, nullptr);
	}
};
