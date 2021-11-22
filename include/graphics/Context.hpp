#ifndef ENGINE_GRAPHICS_CONTEXT_HPP
#define ENGINE_GRAPHICS_CONTEXT_HPP

#include "framework.hpp"
#include "SwapChain.hpp"
#include "Initializable.hpp"
#include "graphics/VkUtils.hpp"
#include "graphics/VkFactory.hpp"
#include "graphics/Color.hpp"

namespace NovaEngine::Graphics
{
	class SyncObjects;
	class GraphicsManager;

	struct ContextOptions
	{
		SwapChainOptions* swapChainOptions = nullptr;
		Color clearColor;
	};

	class Context : public Initializable<GraphicsManager*, GLFWwindow*, ContextOptions*>
	{
		static void onFrameBufferResizedCallback(GLFWwindow* window, int width, int height);

		static ContextOptions defaultOptions;

		size_t index_;
		GraphicsManager* manager_;
		VkInstance instance_;
		VkPhysicalDevice physicalDevice_;
		VkDevice device_;
		QueueFamilies queueFamilies_;
		GLFWwindow* window_;
		VkSurfaceKHR surface_;
		SwapChainSupportDetails swapChainSupportDetails_;
		VkSurfaceFormatKHR surfaceFormat_;
		SwapChain swapChain_;
		VkQueue graphicsQueue_;
		VkQueue presentQueue_;
		VkRenderPass renderPass_;
		VkCommandPool commandPool_;
		std::vector<VkCommandBuffer> commandBuffers_;
		SyncObjects syncObjects_;
		bool didResize_;
		size_t currentFrame_;
		Color clearColor_;

	protected:
		bool onInitialize(GraphicsManager* manager, GLFWwindow* window, ContextOptions* options = nullptr);

	public:
		Context(size_t index) : Initializable(),
			index_(index),
			manager_(nullptr),
			instance_(VK_NULL_HANDLE),
			physicalDevice_(VK_NULL_HANDLE),
			device_(VK_NULL_HANDLE),
			queueFamilies_(),
			window_(nullptr),
			surface_(VK_NULL_HANDLE),
			swapChainSupportDetails_(),
			surfaceFormat_(),
			swapChain_(),
			graphicsQueue_(VK_NULL_HANDLE),
			presentQueue_(VK_NULL_HANDLE),
			renderPass_(VK_NULL_HANDLE),
			commandPool_(VK_NULL_HANDLE),
			commandBuffers_(),
			syncObjects_(device_),
			didResize_(false),
			currentFrame_(0),
			clearColor_()
		{}

		GLFWwindow* window() { return window_; }
		const VkSurfaceKHR& surface() { return surface_; }
		SwapChain& swapChain() { return swapChain_; }

		void resizeSwapchain();

		void destroy();

		VkQueue getGraphicsQueue();
		VkQueue getPresentationQueue();

		template<typename RecordCallback>
		void record(uint32_t frameIndex, RecordCallback recordCallback)
		{
			VkCommandBuffer buf = commandBuffers_[frameIndex];

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(buf, &beginInfo) != VK_SUCCESS)
				throw std::runtime_error("failed to begin recording command buffer!");

			VkClearValue clearColor = { { clearColor_.getVkColor() } };
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass_;
			renderPassInfo.framebuffer = swapChain_.frameBuffers_[frameIndex];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChain_.extent_;
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(buf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			recordCallback(buf);

			// vkCmdBindPipeline(buf, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline_);

			// VkViewport viewport = {};
			// viewport.x = 0.0f;
			// viewport.y = 0.0f;
			// viewport.width = (float)swapChain_.extent.width;
			// viewport.height = (float)swapChain_.extent.height;
			// viewport.minDepth = 0.0f;
			// viewport.maxDepth = 1.0f;

			// vkCmdSetViewport(buf, 0, 1, &viewport);

			// VkRect2D siccors = {};
			// siccors.offset = { 0, 0 };
			// siccors.extent = swapChain_.extent;
			// vkCmdSetScissor(buf, 0, 1, &siccors);

			// VkBuffer vertexBuffers[] = { vertexBuffer };
			// VkDeviceSize offsets[] = { 0 };
			// vkCmdBindVertexBuffers(buf, 0, 1, vertexBuffers, offsets);

			// vkCmdDraw(buf, 3, 1, 0, 0);

			vkCmdEndRenderPass(buf);

			if (vkEndCommandBuffer(buf) != VK_SUCCESS)
				throw std::runtime_error("failed to record command buffer!");
		}

		template<typename WaitCallback>
		void present(WaitCallback onWaitCallback)
		{
			vkWaitForFences(device_, 1, &syncObjects_.inFlightFences[currentFrame_], VK_TRUE, UINT64_MAX);
			
			vkResetFences(device_, 1, &syncObjects_.imageAvailableFences[currentFrame_]);

			uint32_t imageIndex;
			VkResult result = vkAcquireNextImageKHR(device_, swapChain_.get(), UINT64_MAX, syncObjects_.imageAvailableSemaphores[currentFrame_], syncObjects_.imageAvailableFences[currentFrame_], &imageIndex);

			VkResult fenceResult = vkGetFenceStatus(device_, syncObjects_.imageAvailableFences[currentFrame_]);

			// if (fenceResult == VK_SUCCESS)
			// {
			// 	printf("okiiii :D...\n");
			// }
			// else 
			int i = 0;
			if (fenceResult == VK_NOT_READY)
			{
				while (fenceResult == VK_NOT_READY)
				{
					// printf("wait %i\n", ++i);
					if (didResize_)
					{
						printf("did resize\n");
						
						resizeSwapchain();
						didResize_ = false;
						vkResetFences(device_, 1, &syncObjects_.imageAvailableFences[currentFrame_]);
						return;
					}

					onWaitCallback();

					fenceResult = vkGetFenceStatus(device_, syncObjects_.imageAvailableFences[currentFrame_]);
				}
			}

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

			syncObjects_.imagesInFlight[imageIndex] = syncObjects_.inFlightFences[currentFrame_];

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore waitSemaphores[] = { syncObjects_.imageAvailableSemaphores[currentFrame_] };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffers_[imageIndex];

			VkSemaphore signalSemaphores[] = { syncObjects_.renderFinishedSemaphores[currentFrame_] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			vkResetFences(device_, 1, &syncObjects_.inFlightFences[currentFrame_]);

			record(imageIndex, [](VkCommandBuffer) {});

			if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, syncObjects_.inFlightFences[currentFrame_]) != VK_SUCCESS)
				throw std::runtime_error("failed to submit draw command buffer!");

			VkPresentInfoKHR presentInfo = {};
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
				printf("did resize 2...\n");
				resizeSwapchain();
				didResize_ = false;
				// present(onWaitCallback);
			}
			else if (result != VK_SUCCESS)
				throw std::runtime_error("failed to present swap chain image!");

			currentFrame_ = (currentFrame_ + 1) % syncObjects_.maxFramesInFlight;
		}

		void present()
		{
			present([]() {});
		}

		friend class GraphicsManager;
		friend class SwapChain;
	};
};

#endif
