#ifndef NOVA_ENGINE_GRAPHICS_VK_FACTORY_HPP
#define NOVA_ENGINE_GRAPHICS_VK_FACTORY_HPP

#include "framework.hpp"
#include "graphics/VkUtils.hpp"

namespace NovaEngine::Graphics
{
	class SwapChain;

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

		SyncObjects& operator=(const SyncObjects& other)
		{
			device_ = other.device_;
			buffers = other.buffers;
			imageAvailableSemaphores = other.imageAvailableSemaphores;
			imageAvailableFences = other.imageAvailableFences;
			renderFinishedSemaphores = other.renderFinishedSemaphores;
			inFlightFences = other.inFlightFences;
			imagesInFlight = other.imagesInFlight;
			maxFramesInFlight = other.maxFramesInFlight;
			return *this;
		}

		void init()
		{
			
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
		SyncObjects createSyncObjects(VkDevice& device, SwapChain& swapchain, size_t maxFramesInFlight);
		// VkBuffer createVertexBuffer(Vk::Device& dev, uint64_t size);
	};
};

#endif
