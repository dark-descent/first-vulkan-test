#ifndef NOVA_ENGINE_GRAPHICS_VK_FACTORY_HPP
#define NOVA_ENGINE_GRAPHICS_VK_FACTORY_HPP

#include "framework.hpp"
#include "graphics/VkUtils.hpp"
#include "graphics/SwapChain.hpp"

namespace NovaEngine::Graphics
{
	class SyncObjects
	{
		VkDevice& device_;

	public:
		std::vector<VkCommandBuffer> buffers;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkFence> imageAvailableFences;
		std::vector<VkSemaphore> renderFinishedSemaphores;

		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		uint32_t maxFramesInFlight;

		SyncObjects(VkDevice& device) :
			device_(device),
			buffers(),
			imageAvailableSemaphores(),
			imageAvailableFences(),
			renderFinishedSemaphores(),
			inFlightFences(),
			imagesInFlight(),
			maxFramesInFlight(1)
		{}

		void init(VkDevice device, SwapChain& swapchain, uint32_t framesInFlight)
		{
			maxFramesInFlight = framesInFlight;
			imageAvailableSemaphores.resize(maxFramesInFlight);
			imageAvailableFences.resize(maxFramesInFlight);
			renderFinishedSemaphores.resize(maxFramesInFlight);
			inFlightFences.resize(maxFramesInFlight);
			imagesInFlight.resize(swapchain.imageCount(), VK_NULL_HANDLE);

			VkSemaphoreCreateInfo semaphoreInfo = {};

			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < maxFramesInFlight; i++)
			{
				if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
					vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS ||
					vkCreateFence(device, &fenceInfo, nullptr, &imageAvailableFences[i]) != VK_SUCCESS)
					throw std::runtime_error("failed to create synchronization objects for a frame!");
			}

		}

		void destroy()
		{
			for (size_t i = 0; i < maxFramesInFlight; i++)
			{
				vkDestroySemaphore(device_, renderFinishedSemaphores[i], nullptr);
				vkDestroySemaphore(device_, imageAvailableSemaphores[i], nullptr);
				vkDestroyFence(device_, inFlightFences[i], nullptr);
			}
		}
	};

	namespace VkFactory
	{
		namespace
		{
			const std::vector<const char*> defaultDeviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
		};

		VkInstance createInstance(const std::vector<const char*>& layers, VkDebugUtilsMessengerEXT* debugExt = nullptr);
		VkSurfaceKHR createSurface(VkInstance& instance, GLFWwindow* window);
		VkPhysicalDevice pickPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface, const std::vector<const char*>& extensions);
		VkDevice createDevice(const VkPhysicalDevice& physicalDevice, QueueFamilies& queueFamilies, const std::vector<const char*>& extensions, const std::vector<const char*>& layers);
		VkRenderPass createRenderPass(const VkDevice& device, const VkFormat& format);
		VkCommandPool createCommandPool(const VkPhysicalDevice& physicalDevice, const VkDevice& device, uint32_t queueFamily);
		std::vector<VkCommandBuffer> createCommandBuffers(const VkDevice& device, const VkCommandPool& commandPool, uint32_t count);
		// SyncObjects createSyncObjects(VkDevice& device, SwapChain& swapchain, size_t maxFramesInFlight);
		// VkBuffer createVertexBuffer(Vk::Device& dev, uint64_t size);
	};
};

#endif
