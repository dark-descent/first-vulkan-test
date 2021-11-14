#include "graphics/GraphicsManager.hpp"

#include "Engine.hpp"

namespace NovaEngine::Graphics
{
	void GraphicsManager::onFrameBufferResizedHandler(GLFWwindow* window, int width, int height)
	{
		GraphicsManager* m = static_cast<GraphicsManager*>(glfwGetWindowUserPointer(window));


		if (m != nullptr)
		{
			m->draw();
			m->initSwapChain(true);

			for (size_t i = 0; i < m->commandBuffers_.buffers.size(); i++)
				m->recordCommands(i);
		}

		Logger::get()->info("frame buffer resized!");
	}

	bool GraphicsManager::onInitialize(GLFWwindow* window)
	{
		try
		{
			window_ = window;
			instance_ = VkFactory::createInstance();
			surface_ = VkFactory::createSurface(instance_, window_);
			physicalDevice_ = VkFactory::pickPhysicalDevice(instance_, surface_);
			device_ = VkFactory::createDevice(physicalDevice_, surface_);

			initSwapChain();

			syncObjects_ = VkFactory::createSyncObjects(device_, swapChain_, 2);

			glfwSetWindowUserPointer(window_, this);
			glfwSetFramebufferSizeCallback(window_, onFrameBufferResizedHandler);
		}
		catch (const std::runtime_error& err)
		{
			Logger::get()->error(err.what());
		}

		for (size_t i = 0; i < commandBuffers_.buffers.size(); i++)
			recordCommands(i);

		vkDeviceWaitIdle(*device_);

		return true;
	}

	bool GraphicsManager::onTerminate()
	{
		vkDeviceWaitIdle(*device_);
		syncObjects_.destroy();

		destroySwapChain();

		device_.destroy();
		physicalDevice_.destroy();
		surface_.destroy();
		instance_.destroy();

		return true;
	}

	void GraphicsManager::initSwapChain(bool recreate)
	{
		if (isInitialized() && recreate)
		{
			Logger::get()->info("Recreating swapchain...");
			vkDeviceWaitIdle(*device_);
			Vk::SwapChain newSwapChain = VkFactory::createSwapChain(physicalDevice_, device_, surface_, window_, &swapChain_);
			destroySwapChain();
			swapChain_ = newSwapChain;
		}
		else
		{
			swapChain_ = VkFactory::createSwapChain(physicalDevice_, device_, surface_, window_);
		}

		renderPass_ = VkFactory::createRenderPass(device_, swapChain_);
		swapChain_.createFrameBuffers(renderPass_);
		pipeline_ = VkFactory::createPipline(device_, swapChain_, renderPass_);
		commandPool_ = VkFactory::createCommandPool(physicalDevice_, device_, surface_);
		commandBuffers_ = VkFactory::createCommandBuffers(device_, swapChain_, commandPool_);
	}

	void GraphicsManager::destroySwapChain()
	{
		commandPool_.destroy();
		swapChain_.destroyFrameBuffers();
		pipeline_.destroy();
		renderPass_.destroy();
		swapChain_.destroy();
	}


	void GraphicsManager::recordCommands(size_t index)
	{
		VkCommandBuffer buf = commandBuffers_.buffers[index];

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(buf, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("failed to begin recording command buffer!");

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = *renderPass_;
		renderPassInfo.framebuffer = swapChain_.frameBuffers[index];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain_.extent;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(buf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(buf, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline_);

		vkCmdDraw(buf, 3, 1, 0, 0);

		vkCmdEndRenderPass(buf);

		if (vkEndCommandBuffer(buf) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void GraphicsManager::draw()
	{
		static uint32_t currentFrame = 0;

		vkWaitForFences(*device_, 1, &syncObjects_.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(*device_, *swapChain_, UINT64_MAX, syncObjects_.imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			Logger::get()->info("out of dat acquire!");
			initSwapChain(true);
			return;
		}
		else if (result == VK_SUBOPTIMAL_KHR)
		{
			Logger::get()->info("suboptimal acquire!");
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to aquire next image from swapchain!");
		}

		if (syncObjects_.imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(*device_, 1, &syncObjects_.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		syncObjects_.imagesInFlight[imageIndex] = syncObjects_.inFlightFences[currentFrame];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { syncObjects_.imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers_.buffers[imageIndex];

		VkSemaphore signalSemaphores[] = { syncObjects_.renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(*device_, 1, &syncObjects_.inFlightFences[currentFrame]);

		if (vkQueueSubmit(device_.graphicsQueue, 1, &submitInfo, syncObjects_.inFlightFences[currentFrame]) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer!");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { *swapChain_ };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(device_.presentationQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			Logger::get()->info("suboptimal acquire!");
			initSwapChain(true);
		}
		else if (result != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain image!");

		currentFrame = (currentFrame + 1) % syncObjects_.maxFramesInFlight;
	}
};
