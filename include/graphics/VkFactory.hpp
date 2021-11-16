#ifndef NOVA_ENGINE_GRAPHICS_VK_FACTORY_HPP
#define NOVA_ENGINE_GRAPHICS_VK_FACTORY_HPP

#include "framework.hpp"
#include "graphics/VkUtils.hpp"

namespace NovaEngine::Graphics
{
	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;
		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);
			return attributeDescriptions;
		}
	};
	namespace Vk
	{
		template<typename A>
		class Wrapper
		{
		public:
			A first;
			Wrapper(A first) : first(first) {}
			Wrapper() : Wrapper(VK_NULL_HANDLE) {}

			virtual void destroy() = 0;

			A& operator*() { return first; }
			Wrapper<A>& operator=(const Wrapper<A>& other)
			{
				first = other.first;
				return *this;
			}
		};

		template<typename A, typename B>
		class Wrapper2 : public Wrapper<A>
		{
		public:
			B second;
			Wrapper2(A first, B second) : Wrapper<A>(first), second(second) {}
			Wrapper2() : Wrapper<A>(VK_NULL_HANDLE), second(VK_NULL_HANDLE) {}

			virtual void destroy() = 0;

			Wrapper2<A, B>& operator=(const Wrapper2<A, B>& other)
			{
				this->first = other.first;
				this->second = other.second;
				return *this;
			}
		};

		class Instance : public Wrapper2<VkInstance, VkDebugUtilsMessengerEXT>
		{
		public:
			Instance(VkInstance instance, VkDebugUtilsMessengerEXT debugExt) : Wrapper2<VkInstance, VkDebugUtilsMessengerEXT>(instance, debugExt) {}
			Instance() : Wrapper2<VkInstance, VkDebugUtilsMessengerEXT>() {}

			void destroy()
			{
#ifdef DEBUG
				VkUtils::destroyDebugUtilsMessengerEXT(first, second, nullptr);
#endif	
				vkDestroyInstance(first, nullptr);
			}
		};

		class Surface : public Wrapper2<VkSurfaceKHR, VkInstance>
		{
		public:
			Surface(VkSurfaceKHR surface, VkInstance instance) : Wrapper2<VkSurfaceKHR, VkInstance>(surface, instance) {}
			Surface() : Wrapper2<VkSurfaceKHR, VkInstance>() {}

			void destroy()
			{
				vkDestroySurfaceKHR(second, first, nullptr);
			}
		};

		class PhysicalDevice : public Wrapper<VkPhysicalDevice>
		{
		public:
			PhysicalDevice() : Wrapper<VkPhysicalDevice>() {}
			PhysicalDevice(VkPhysicalDevice dev) : Wrapper<VkPhysicalDevice>(dev) {}

			void destroy()
			{
			}
		};

		class Device : public Wrapper<VkDevice>
		{
		public:
			VkQueue graphicsQueue;
			VkQueue presentationQueue;

			Device() : Wrapper<VkDevice>(),
				graphicsQueue(VK_NULL_HANDLE),
				presentationQueue(VK_NULL_HANDLE)
			{}

			Device(VkDevice dev) : Wrapper<VkDevice>(dev),
				graphicsQueue(VK_NULL_HANDLE),
				presentationQueue(VK_NULL_HANDLE)
			{}

			void destroy()
			{
				vkDestroyDevice(first, nullptr);
			}
		};

		class RenderPass : public Wrapper2<VkRenderPass, VkDevice>
		{
		public:
			RenderPass() : Wrapper2<VkRenderPass, VkDevice>() {}
			RenderPass(VkRenderPass swapChain, VkDevice dev) : Wrapper2<VkRenderPass, VkDevice>(swapChain, dev) {}

			void destroy()
			{
				vkDestroyRenderPass(second, first, nullptr);
			}
		};

		class SwapChain : public Wrapper2<VkSwapchainKHR, VkDevice>
		{
		public:
			std::vector<VkImage> images;
			std::vector<VkImageView> imageViews;
			VkFormat format;
			VkExtent2D extent;
			std::vector<VkFramebuffer> frameBuffers;

			SwapChain() : Wrapper2<VkSwapchainKHR, VkDevice>(), images(), imageViews(), format(), extent(), frameBuffers() {}
			SwapChain(VkSwapchainKHR swapChain, VkDevice dev) : Wrapper2<VkSwapchainKHR, VkDevice>(swapChain, dev), images(), imageViews(), format(), extent(), frameBuffers() {}

			void createFrameBuffers(Vk::RenderPass& renderPass);

			void destroy()
			{
				for (auto imageView : imageViews)
					vkDestroyImageView(second, imageView, nullptr);

				vkDestroySwapchainKHR(second, first, nullptr);
			}

			void destroyFrameBuffers()
			{
				for (const auto& fb : frameBuffers)
					vkDestroyFramebuffer(second, fb, nullptr);
			}
		};

		class Pipeline : public Wrapper2<VkPipeline, VkDevice>
		{
		public:
			VkPipelineLayout layout;

			Pipeline() : Wrapper2<VkPipeline, VkDevice>(), layout(VK_NULL_HANDLE) {}
			Pipeline(VkPipeline pipeline, VkDevice dev) : Wrapper2<VkPipeline, VkDevice>(pipeline, dev), layout(VK_NULL_HANDLE) {}

			void destroy()
			{
				vkDestroyPipelineLayout(second, layout, nullptr);
				vkDestroyPipeline(second, first, nullptr);
			}
		};

		class CommandPool : public Wrapper2<VkCommandPool, VkDevice>
		{
		public:
			CommandPool() : Wrapper2<VkCommandPool, VkDevice>() {}
			CommandPool(VkCommandPool commandPool, VkDevice dev) : Wrapper2<VkCommandPool, VkDevice>(commandPool, dev) {}

			void destroy()
			{
				vkDestroyCommandPool(second, first, nullptr);
			}
		};

		class CommandBufferGroup : public Wrapper<VkDevice>
		{

		public:
			std::vector<VkCommandBuffer> buffers;

			CommandBufferGroup() : Wrapper<VkDevice>() {}
			CommandBufferGroup(VkDevice dev) : Wrapper<VkDevice>(dev) {}

			void destroy()
			{

			}
		};

		class SyncObjects : public Wrapper<VkDevice>
		{
		public:
			std::vector<VkCommandBuffer> buffers;
			std::vector<VkSemaphore> imageAvailableSemaphores;
			std::vector<VkSemaphore> renderFinishedSemaphores;
			std::vector<VkFence> inFlightFences;
			std::vector<VkFence> imagesInFlight;
			uint32_t maxFramesInFlight;

			SyncObjects() : Wrapper<VkDevice>(),
				buffers(),
				imageAvailableSemaphores(),
				renderFinishedSemaphores(),
				inFlightFences(),
				imagesInFlight(),
				maxFramesInFlight(1)
			{}

			SyncObjects(VkDevice dev) : Wrapper<VkDevice>(dev),
				buffers(),
				imageAvailableSemaphores(),
				renderFinishedSemaphores(),
				inFlightFences(),
				imagesInFlight(),
				maxFramesInFlight(1)
			{}

			void destroy()
			{
				for (size_t i = 0; i < maxFramesInFlight; i++)
				{
					vkDestroySemaphore(first, renderFinishedSemaphores[i], nullptr);
					vkDestroySemaphore(first, imageAvailableSemaphores[i], nullptr);
					vkDestroyFence(first, inFlightFences[i], nullptr);
				}
			}
		};
	};

	namespace VkFactory
	{
		namespace
		{

			std::vector<const char*> defaultLayers = {
#ifdef DEBUG
			"VK_LAYER_KHRONOS_validation",
#endif
			};

			const std::vector<const char*> defaultDeviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
		};

		Vk::Instance createInstance(const std::vector<const char*>& layers = defaultLayers);
		Vk::Surface createSurface(Vk::Instance& instance, GLFWwindow* window);
		Vk::PhysicalDevice pickPhysicalDevice(Vk::Instance& instance, Vk::Surface& surface, const std::vector<const char*>& extensions = defaultDeviceExtensions);
		Vk::Device createDevice(Vk::PhysicalDevice& physicalDevice, Vk::Surface& surface, const std::vector<const char*>& extensions = defaultDeviceExtensions, const std::vector<const char*>& layers = defaultLayers);
		Vk::SwapChain createSwapChain(Vk::PhysicalDevice& physicalDevice, Vk::Device& device, Vk::Surface& surface, GLFWwindow* window, Vk::SwapChain* oldSwapChain);
		Vk::RenderPass createRenderPass(Vk::Device& device, Vk::SwapChain& swapChain);
		Vk::Pipeline createPipline(Vk::Device& device, Vk::SwapChain& swapChain, Vk::RenderPass& renderPass);
		Vk::CommandPool createCommandPool(Vk::PhysicalDevice& physicalDevice, Vk::Device& device, Vk::Surface& surface);
		Vk::CommandBufferGroup createCommandBuffers(Vk::Device& device, Vk::SwapChain& swapChain, Vk::CommandPool& commandPool);
		Vk::SyncObjects createSyncObjects(Vk::Device& device, Vk::SwapChain& swapchain, size_t maxFramesInFlight);
		VkBuffer createVertexBuffer(Vk::Device& dev, uint64_t size);
	};
};

#endif
