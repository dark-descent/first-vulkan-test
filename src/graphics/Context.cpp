#include "graphics/Context.hpp"
#include "graphics/VkFactory.hpp"
#include "graphics/VkUtils.hpp"
#include "graphics/GraphicsManager.hpp"
#include "Logger.hpp"

namespace NovaEngine::Graphics
{
	void Context::onFrameBufferResizedCallback(GLFWwindow* window, int width, int height)
	{
		Context* ctx = reinterpret_cast<Context*>(glfwGetWindowUserPointer(window));
		ctx->didResize_ = true;
	}

	bool Context::onInitialize(GraphicsManager* manager, GLFWwindow* window, VkQueue graphicsQueue, VkQueue presentQueue, VkSurfaceKHR surface, SwapChainOptions* options)
	{
		manager_ = manager;
		instance_ = manager->instance_;
		physicalDevice_ = manager->physicalDevice_;
		device_ = manager->device_;
		queueFamilies_ = &manager->queueFamilies_;
		graphicsQueue_ = graphicsQueue;
		presentQueue_ = presentQueue;
		window_ = window;
		surface_ = surface != VK_NULL_HANDLE ? surface : VkFactory::createSurface(manager->instance_, window);
		swapChainSupportDetails_ = VkUtils::querySwapChainSupport(manager->physicalDevice_, surface);
		surfaceFormat_ = VkUtils::chooseSwapSurfaceFormat(swapChainSupportDetails_.formats);
		renderPass_ = VkFactory::createRenderPass(device_, surfaceFormat_.format);
		swapChain_.initialize(this, options);
		commandPool_ = VkFactory::createCommandPool(physicalDevice_, device_, queueFamilies_->graphics.value().index);
		commandBuffers_ = VkFactory::createCommandBuffers(device_, commandPool_, swapChain_.imageCount_);
		syncObjects_ = VkFactory::createSyncObjects(device_, swapChain_, swapChain_.imageCount());

		glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
		glfwSetFramebufferSizeCallback(window, onFrameBufferResizedCallback);

		Logger::get()->info("Context initialized!\n");

		return true;
	}

	void Context::resizeSwapchain()
	{
		std::vector<VkFence> fences;
		
		for(const auto& f : syncObjects_.imagesInFlight)
			if(f != VK_NULL_HANDLE)
				fences.push_back(f);
		
		if(fences.size() > 0)
			vkWaitForFences(device_, fences.size(), fences.data(), VK_TRUE, UINT64_MAX);

		vkDestroyCommandPool(device_, commandPool_, nullptr);
		swapChain_.recreate();
		commandPool_ = VkFactory::createCommandPool(physicalDevice_, device_, queueFamilies_->graphics.value().index);
		commandBuffers_ = VkFactory::createCommandBuffers(device_, commandPool_, swapChain_.imageCount_);
	}

	void Context::destroy()
	{
		std::vector<VkFence> fences;
		
		for(const auto& f : syncObjects_.imagesInFlight)
			if(f != VK_NULL_HANDLE)
				fences.push_back(f);
		
		if(fences.size() > 0)
			vkWaitForFences(device_, fences.size(), fences.data(), VK_TRUE, UINT64_MAX);
			
		vkDestroyCommandPool(device_, commandPool_, nullptr);
		swapChain_.destroy();
		vkDestroySurfaceKHR(instance_, surface_, nullptr);
	}

	void Context::present()
	{
		static uint32_t currentFrame = 0;

		vkWaitForFences(device_, 1, &syncObjects_.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device_, swapChain_.swapChain_, UINT64_MAX, syncObjects_.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			resizeSwapchain();
			return;
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to aquire next image from swapchain!");
		}

		if (syncObjects_.imagesInFlight[imageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(device_, 1, &syncObjects_.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

		syncObjects_.imagesInFlight[imageIndex] = syncObjects_.inFlightFences[currentFrame];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { syncObjects_.imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers_[imageIndex];

		VkSemaphore signalSemaphores[] = { syncObjects_.renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device_, 1, &syncObjects_.inFlightFences[currentFrame]);

		record(imageIndex, [](VkCommandBuffer){});

		if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, syncObjects_.inFlightFences[currentFrame]) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain_.get() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(presentQueue_, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || didResize_)
		{
			resizeSwapchain();
			didResize_ = false;
			present();
		}
		else if (result != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain image!");

		currentFrame = (currentFrame + 1) % syncObjects_.maxFramesInFlight;
	}
};
