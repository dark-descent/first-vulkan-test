#include "graphics/VkFactory.hpp"
#include "graphics/VkUtils.hpp"
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

			static std::vector<char> readFile(const std::string& filename)
			{
				std::ifstream file(filename, std::ios::ate | std::ios::binary);

				if (!file.is_open()) {
					throw std::runtime_error("failed to open file!");
				}

				size_t fileSize = (size_t)file.tellg();
				std::vector<char> buffer(fileSize);

				file.seekg(0);
				file.read(buffer.data(), fileSize);

				file.close();

				return buffer;
			}

		};

		Vk::Instance createInstance(const std::vector<const char*>& layers)
		{
			if (!VkUtils::supportLayers(layers))
				throw std::runtime_error("No support for provided layers!");

			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Hello Triangle";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "No Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

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
				throw std::runtime_error("failed to create instance!");

			VkDebugUtilsMessengerEXT debugExt = VK_NULL_HANDLE;
#ifdef DEBUG
			if (VkUtils::createDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugExt) != VK_SUCCESS)
				throw std::runtime_error("failed to set up debug messenger!");
#endif

			return Vk::Instance(instance, debugExt);
		}

		Vk::Surface createSurface(Vk::Instance& instance, GLFWwindow* window)
		{
			VkSurfaceKHR surface;
			if (glfwCreateWindowSurface(*instance, window, nullptr, &surface) != VK_SUCCESS)
				throw std::runtime_error("failed to create window surface!");
			return Vk::Surface(surface, *instance);
		}

		Vk::PhysicalDevice pickPhysicalDevice(Vk::Instance& instance, Vk::Surface& surface, const std::vector<const char*>& extensions)
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

			if (deviceCount == 0)
			{
				throw std::runtime_error("failed to find GPUs with Vulkan support!");
			}

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());

			for (const auto& device : devices)
			{
				if (VkUtils::isDeviceSuitable(device, *surface, extensions))
				{
					return Vk::PhysicalDevice(device);
				}
			}

			throw std::runtime_error("failed to find a suitable GPU!");
		}

		Vk::Device createDevice(Vk::PhysicalDevice& physicalDevice, Vk::Surface& surface, const std::vector<const char*>& deviceExtensions, const std::vector<const char*>& validationLayers)
		{
			QueueFamilyIndices indices = VkUtils::findQueueFamilies(*physicalDevice, *surface);

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			float queuePriority = 1.0f;
			for (uint32_t queueFamily : uniqueQueueFamilies) {
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}

			VkPhysicalDeviceFeatures deviceFeatures = {};

			VkDeviceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pQueueCreateInfos = queueCreateInfos.data();

			createInfo.pEnabledFeatures = &deviceFeatures;

			createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifdef DEBUG
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
#else
			createInfo.enabledLayerCount = 0;
#endif

			VkDevice device;

			if (vkCreateDevice(*physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
				throw std::runtime_error("failed to create logical device!");

			Vk::Device d = Vk::Device(device);
			d.graphicsQueue = VkUtils::getGraphicsQueue(device, indices);
			d.presentationQueue = VkUtils::getPresentationQueue(device, indices);
			return d;
		}

		Vk::SwapChain createSwapChain(Vk::PhysicalDevice& physicalDevice, Vk::Device& device, Vk::Surface& surface, GLFWwindow* window, Vk::SwapChain* oldSwapChain)
		{
			SwapChainSupportDetails swapChainSupport = VkUtils::querySwapChainSupport(*physicalDevice, *surface);

			VkSurfaceFormatKHR surfaceFormat = VkUtils::chooseSwapSurfaceFormat(swapChainSupport.formats);
			VkPresentModeKHR presentMode = VkUtils::chooseSwapPresentMode(swapChainSupport.presentModes);
			VkExtent2D extent = VkUtils::chooseSwapExtent(swapChainSupport.capabilities, window);

			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
				imageCount = swapChainSupport.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = *surface;

			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			QueueFamilyIndices indices = VkUtils::findQueueFamilies(*physicalDevice, *surface);
			uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			if (indices.graphicsFamily != indices.presentFamily)
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}

			createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;

			createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : *(*oldSwapChain);

			VkSwapchainKHR swapChain;

			if (vkCreateSwapchainKHR(*device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
				throw std::runtime_error("failed to create swap chain!");

			Vk::SwapChain wrappedSwapChain(swapChain, *device);

			vkGetSwapchainImagesKHR(*device, swapChain, &imageCount, nullptr);
			wrappedSwapChain.images.resize(imageCount);
			vkGetSwapchainImagesKHR(*device, swapChain, &imageCount, wrappedSwapChain.images.data());

			wrappedSwapChain.format = surfaceFormat.format;
			wrappedSwapChain.extent = extent;

			wrappedSwapChain.imageViews.resize(imageCount);

			for (size_t i = 0; i < imageCount; i++)
			{
				VkImageViewCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = wrappedSwapChain.images[i];
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = surfaceFormat.format;
				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				if (vkCreateImageView(*device, &createInfo, nullptr, &wrappedSwapChain.imageViews[i]) != VK_SUCCESS)
					throw std::runtime_error("failed to create image views!");
			}

			return wrappedSwapChain;
		}

		Vk::RenderPass createRenderPass(Vk::Device& device, Vk::SwapChain& swapChain)
		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = swapChain.format;
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

			if (vkCreateRenderPass(*device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
				throw std::runtime_error("failed to create render pass!");

			return Vk::RenderPass(renderPass, *device);
		}

		Vk::Pipeline createPipline(Vk::Device& device, Vk::SwapChain& swapChain, Vk::RenderPass& renderPass)
		{
			auto vertShaderCode = readFile("assets/shaders/unlit.vert.spv");
			auto fragShaderCode = readFile("assets/shaders/unlit.frag.spv");

			VkShaderModule vertShaderModule = VkUtils::createShaderModule(*device, vertShaderCode);
			VkShaderModule fragShaderModule = VkUtils::createShaderModule(*device, fragShaderCode);

			VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = vertShaderModule;
			vertShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			vertexInputInfo.vertexAttributeDescriptionCount = 0;

			VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)swapChain.extent.width;
			viewport.height = (float)swapChain.extent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = { 0, 0 };
			scissor.extent = swapChain.extent;

			VkPipelineViewportStateCreateInfo viewportState = {};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			VkPipelineRasterizationStateCreateInfo rasterizer = {};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;

			VkPipelineMultisampleStateCreateInfo multisampling = {};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlending = {};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;

			VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 0;
			pipelineLayoutInfo.pushConstantRangeCount = 0;

			VkPipelineLayout layout;

			if (vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
				throw std::runtime_error("failed to create pipeline layout!");

			VkGraphicsPipelineCreateInfo pipelineInfo = {};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.layout = layout;
			pipelineInfo.renderPass = *renderPass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

			VkPipeline pipeline;

			if (vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
				throw std::runtime_error("failed to create graphics pipeline!");

			vkDestroyShaderModule(*device, fragShaderModule, nullptr);
			vkDestroyShaderModule(*device, vertShaderModule, nullptr);

			Vk::Pipeline p = Vk::Pipeline(pipeline, *device);
			p.layout = layout;
			return p;
		}

		Vk::CommandPool createCommandPool(Vk::PhysicalDevice& physicalDevice, Vk::Device& device, Vk::Surface& surface)
		{
			QueueFamilyIndices queueFamilyIndices = VkUtils::findQueueFamilies(*physicalDevice, *surface);

			VkCommandPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

			VkCommandPool commandPool;

			if (vkCreateCommandPool(*device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
				throw std::runtime_error("failed to create command pool!");

			return Vk::CommandPool(commandPool, *device);
		}

		Vk::CommandBufferGroup createCommandBuffers(Vk::Device& device, Vk::SwapChain& swapChain, Vk::CommandPool& commandPool)
		{
			Vk::CommandBufferGroup bufferGroup(*device);

			bufferGroup.buffers.resize(swapChain.frameBuffers.size());

			VkCommandBufferAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = *commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = (uint32_t)bufferGroup.buffers.size();

			if (vkAllocateCommandBuffers(*device, &allocInfo, bufferGroup.buffers.data()) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate command buffers!");

			return bufferGroup;
		}

		Vk::SyncObjects createSyncObjects(Vk::Device& device, Vk::SwapChain& swapchain, size_t maxFramesInFlight)
		{
			Vk::SyncObjects syncObjects(*device);

			syncObjects.imageAvailableSemaphores.resize(maxFramesInFlight);
			syncObjects.renderFinishedSemaphores.resize(maxFramesInFlight);
			syncObjects.inFlightFences.resize(maxFramesInFlight);
			syncObjects.imagesInFlight.resize(swapchain.images.size(), VK_NULL_HANDLE);

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < maxFramesInFlight; i++)
			{
				if (vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &syncObjects.imageAvailableSemaphores[i]) != VK_SUCCESS ||
					vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &syncObjects.renderFinishedSemaphores[i]) != VK_SUCCESS ||
					vkCreateFence(*device, &fenceInfo, nullptr, &syncObjects.inFlightFences[i]) != VK_SUCCESS)
					throw std::runtime_error("failed to create synchronization objects for a frame!");
			}

			syncObjects.maxFramesInFlight = maxFramesInFlight;

			return syncObjects;
		}
	};

	void Vk::SwapChain::createFrameBuffers(Vk::RenderPass& renderPass)
	{
		if (frameBuffers.size() > 0)
			return;

		frameBuffers.resize(imageViews.size());

		for (size_t i = 0; i < imageViews.size(); i++)
		{
			VkImageView attachments[] = { imageViews[i] };

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = *renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(second, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create framebuffer!");
		}
	}
};
