#ifndef ENGINE_GRAPHICS_CONTEXT_HPP
#define ENGINE_GRAPHICS_CONTEXT_HPP

#include "framework.hpp"
#include "SwapChain.hpp"
#include "Initializable.hpp"
#include "graphics/VkUtils.hpp"
#include "graphics/VkFactory.hpp"

namespace NovaEngine::Graphics
{
	class SyncObjects;
	class GraphicsManager;

	class Context : public Initializable<GraphicsManager*, GLFWwindow*, VkQueue, VkQueue, VkSurfaceKHR, SwapChainOptions*>
	{
		static void onFrameBufferResizedCallback(GLFWwindow* window, int width, int height);

		GraphicsManager* manager_;
		VkInstance instance_;
		VkPhysicalDevice physicalDevice_;
		VkDevice device_;
		QueueFamilies* queueFamilies_;
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

	protected:
		bool onInitialize(GraphicsManager* manager, GLFWwindow* window, VkQueue graphicsQueue, VkQueue presentQueue, VkSurfaceKHR surface = VK_NULL_HANDLE, SwapChainOptions* options = nullptr);

	public:
		Context() : Initializable(),
			manager_(nullptr),
			instance_(VK_NULL_HANDLE),
			physicalDevice_(VK_NULL_HANDLE),
			device_(VK_NULL_HANDLE),
			queueFamilies_(nullptr),
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
			didResize_(false)
		{}

		GLFWwindow* window() { return window_; }
		const VkSurfaceKHR& surface() { return surface_; }
		SwapChain& swapChain() { return swapChain_; }

		void resizeSwapchain();

		void destroy();

		template<typename RecordCallback>
		void record(uint32_t frameIndex, RecordCallback recordCallback)
		{
			VkCommandBuffer buf = commandBuffers_[frameIndex];

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(buf, &beginInfo) != VK_SUCCESS)
				throw std::runtime_error("failed to begin recording command buffer!");

			VkClearValue clearColor = { { { 1.0f, 0.0f, 0.0f, 1.0f } } };
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

		void present();

		friend class GraphicsManager;
		friend class SwapChain;
	};
};

#endif
