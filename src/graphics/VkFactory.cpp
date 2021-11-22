#include "graphics/VkFactory.hpp"
#include "graphics/VkUtils.hpp"
#include "graphics/SwapChain.hpp"
#include "Logger.hpp"

#include <exception>

namespace NovaEngine::Graphics
{

	namespace VkFactory
	{
		namespace
		{

#ifdef DEBUG
			static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
			{
				if (strncmp(pCallbackData->pMessage, "Device Extension: ", 18) == 0)
					return VK_FALSE;

				if ((messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) || (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT))
					Logger::get()->info(pCallbackData->pMessage);
				if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
					Logger::get()->warn(pCallbackData->pMessage);
				if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
					Logger::get()->error(pCallbackData->pMessage);

				return VK_FALSE;
			}
#endif

		};

		VkInstance createInstance(const std::vector<const char*>& layers, VkDebugUtilsMessengerEXT* debugExt)
		{
			if (!VkUtils::supportLayers(layers))
				throw std::runtime_error("No support for provided layers!");

			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Hello Triangle";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "No Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_2;

			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;

			auto extensions = VkUtils::getRequiredExtensions();
			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef DEBUG
			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
			debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugCreateInfo.pfnUserCallback = debugCallback;

			createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
			createInfo.ppEnabledLayerNames = layers.data();
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else			
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
#endif

			VkInstance instance = VK_NULL_HANDLE;

			if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
				throw std::runtime_error("Failed to create instance!");

#ifdef DEBUG
			if (VkUtils::createDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, debugExt) != VK_SUCCESS)
				throw std::runtime_error("Failed to set up debug messenger!");
#endif

			return instance;
		}

		VkSurfaceKHR createSurface(VkInstance& instance, GLFWwindow* window)
		{
			VkSurfaceKHR surface;
			if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
				throw std::runtime_error("Failed to create window surface!");
			return surface;
		}

		VkPhysicalDevice pickPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface, const std::vector<const char*>& extensions)
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

			if (deviceCount == 0)
				throw std::runtime_error("Failed to find GPUs with Vulkan support!");

			std::vector<VkPhysicalDevice> devices(deviceCount);

			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

			for (const auto& device : devices)
			{
				VkPhysicalDeviceProperties p = {};
				vkGetPhysicalDeviceProperties(device, &p);
				printf("found device %s...\n", p.deviceName);
			}

			for (const auto& device : devices)
				if (VkUtils::isDeviceSuitable(device, surface, extensions))
				{
					VkPhysicalDeviceProperties p = {};
					vkGetPhysicalDeviceProperties(device, &p);
					printf("choose device %s...\n", p.deviceName);
					return VkPhysicalDevice(device);
				}

			throw std::runtime_error("Failed to find a suitable GPU!");
		}

		VkDevice createDevice(const VkPhysicalDevice& physicalDevice, QueueFamilies& queueFamilies, const std::vector<const char*>& extensions, const std::vector<const char*>& layers)
		{
			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::vector<QueueFamilies::QueueInfo> uniqueQueueInfos;

			uniqueQueueInfos.push_back(queueFamilies.graphics.value());

			if (queueFamilies.graphics.value().index != queueFamilies.present.value().index)
				uniqueQueueInfos.push_back(queueFamilies.present.value());

			float queuePriority = 1.0f;
			for (auto& queueInfo : uniqueQueueInfos)
			{
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueInfo.index;
				queueCreateInfo.queueCount = queueInfo.maxQueues;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}

			VkPhysicalDeviceFeatures deviceFeatures = {};

			VkDeviceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pQueueCreateInfos = queueCreateInfos.data();

			createInfo.pEnabledFeatures = &deviceFeatures;

			createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef DEBUG
			createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
			createInfo.ppEnabledLayerNames = layers.data();
#else
			createInfo.enabledLayerCount = 0;
#endif

			VkDevice device;

			if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
				throw std::runtime_error("failed to create logical device!");

			return device;
		}

		VkRenderPass createRenderPass(const VkDevice& device, const VkFormat& format)
		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = format;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			VkRenderPass renderPass;

			if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
				throw std::runtime_error("failed to create render pass!");

			return renderPass;
		}

		VkCommandPool createCommandPool(const VkPhysicalDevice& physicalDevice, const VkDevice& device, uint32_t queueFamily)
		{
			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = queueFamily;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			VkCommandPool commandPool;

			if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
				throw std::runtime_error("failed to create command pool!");

			return commandPool;
		}

		std::vector<VkCommandBuffer> createCommandBuffers(const VkDevice& device, const VkCommandPool& commandPool, uint32_t count)
		{
			std::vector<VkCommandBuffer> buffers;

			buffers.resize(count);

			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = (uint32_t)count;

			if (vkAllocateCommandBuffers(device, &allocInfo, buffers.data()) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate command buffers!");

			return buffers;
		}

		// SyncObjects createSyncObjects(VkDevice& device, SwapChain& swapchain, size_t maxFramesInFlight)
		// {
		// 	SyncObjects syncObjects = SyncObjects(device);

			
		// 	return syncObjects;
		// }
	}
}
