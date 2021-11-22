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

	bool Context::onInitialize(GraphicsManager* manager, GLFWwindow* window, ContextOptions* options)
	{
		if (options == nullptr)
			options = &defaultOptions;

		window_ = window;
		manager_ = manager;
		instance_ = manager->instance_;
		surface_ = VkFactory::createSurface(instance_, window);
		physicalDevice_ = VkFactory::pickPhysicalDevice(instance_, surface_, GraphicsManager::defaultExtensions);
		queueFamilies_ = VkUtils::findQueueFamilies(physicalDevice_, surface_);
		device_ = VkFactory::createDevice(physicalDevice_, queueFamilies_, GraphicsManager::defaultExtensions, GraphicsManager::defaultLayers);

		graphicsQueue_ = getGraphicsQueue();
		presentQueue_ = getPresentationQueue();

		swapChainSupportDetails_ = VkUtils::querySwapChainSupport(physicalDevice_, surface_);
		surfaceFormat_ = VkUtils::chooseSwapSurfaceFormat(swapChainSupportDetails_.formats);
		renderPass_ = VkFactory::createRenderPass(device_, surfaceFormat_.format);

		swapChain_.initialize(this, options->swapChainOptions);

		commandPool_ = VkFactory::createCommandPool(physicalDevice_, device_, queueFamilies_.graphics.value().index);
		commandBuffers_ = VkFactory::createCommandBuffers(device_, commandPool_, swapChain_.imageCount_);

		syncObjects_.init(device_, swapChain_, swapChain_.imageCount() - 1);

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

		vkDeviceWaitIdle(device_);
		
		vkDestroyCommandPool(device_, commandPool_, nullptr);
		swapChain_.destroy();
		vkDestroySurfaceKHR(instance_, surface_, nullptr);

		printf("context destroyed!\n");
	}

	VkQueue Context::getGraphicsQueue()
	{
		QueueFamilies::QueueInfo& info = queueFamilies_.graphics.value();
		VkQueue queue;
		vkGetDeviceQueue(device_, info.index, 0, &queue);
		return queue;
	}

	VkQueue Context::getPresentationQueue()
	{
		QueueFamilies::QueueInfo& info = queueFamilies_.present.value();
		VkQueue queue;
		vkGetDeviceQueue(device_, info.index, 0, &queue);
		return queue;
	}
};
