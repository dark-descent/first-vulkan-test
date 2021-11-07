#include "graphics/Device.hpp"
#include "graphics/Context.hpp"
#include "framework.hpp"
#include "Logger.hpp"

namespace NovaEngine::Graphics
{
	DeviceConfig Device::defaultConfig = {
		.queueCount = 1
	};

	bool Device::onInitialize(DeviceConfig* config)
	{
		if (config == nullptr)
			config = &defaultConfig;

		graphicsQueues_.resize(config->queueCount);

		PhysicalDevice& pDev = context()->physicalDevice();
		const QueueFamilies& queueFamilies = pDev.queueFamilies();
		uint32_t graphicsQueueIndex = queueFamilies.graphicsFamily.value();
		uint32_t presentQueueIndex = queueFamilies.presentFamily.value();
		float queuePriority = 1.0f;

		VkPhysicalDeviceFeatures deviceFeatures = {};

		if (graphicsQueueIndex == presentQueueIndex)
		{
			VkDeviceQueueCreateInfo createInfos = {
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = graphicsQueueIndex,
				.queueCount = config->queueCount,
				.pQueuePriorities = &queuePriority,
			};

			const char* extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			// pCreateInfo->pQueueCreateInfos[1].queueFamilyIndex (=0) is not unique and was also used in pCreateInfo->pQueueCreateInfos[0]

			VkDeviceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.pQueueCreateInfos = &createInfos;
			createInfo.queueCreateInfoCount = 1;
			createInfo.pEnabledFeatures = &deviceFeatures;
			createInfo.enabledExtensionCount = 1;
			createInfo.ppEnabledExtensionNames = &extensions;

			if (vkCreateDevice(*pDev, &createInfo, nullptr, &device_) != VK_SUCCESS)
			{
				Logger::get()->error("Could not create device!");
				return false;
			}

			presentQueue_ = graphicsQueues_[0];
		}
		else
		{
			VkDeviceQueueCreateInfo createInfos[] = {
				/*graphicsQueueCreateInfo*/
				{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueFamilyIndex = graphicsQueueIndex,
					.queueCount = config->queueCount,
					.pQueuePriorities = &queuePriority,
				},
				/*presentQueueCreateInfo*/
				{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueFamilyIndex = presentQueueIndex,
					.queueCount = 1,
					.pQueuePriorities = &queuePriority
				},
			};

			const char* extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

			VkDeviceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.pQueueCreateInfos = createInfos;
			createInfo.queueCreateInfoCount = 2;
			createInfo.pEnabledFeatures = &deviceFeatures;
			createInfo.enabledExtensionCount = 1;
			createInfo.ppEnabledExtensionNames = &extensions;


			if (vkCreateDevice(*pDev, &createInfo, nullptr, &device_) != VK_SUCCESS)
			{
				Logger::get()->error("Could not create device!");
				return false;
			}

			vkGetDeviceQueue(device_, presentQueueIndex, 0, &presentQueue_);
		}

		for (size_t i = 0; i < config->queueCount; i++)
			vkGetDeviceQueue(device_, graphicsQueueIndex, i, &graphicsQueues_[i]);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS)
		{
			Logger::get()->error("Could not create CommandPool!");
			return false;
		}

		return true;
	}

	bool Device::onTerminate()
	{
		vkDestroyCommandPool(device_, commandPool_, nullptr);
		vkDestroyDevice(device_, nullptr);
		return true;
	}

	const std::vector<VkQueue>& Device::graphicsQueues()
	{
		return graphicsQueues_;
	}

	VkQueue Device::getGraphicsQueue(uint32_t index)
	{
		assert(index < graphicsQueues_.size());
		return graphicsQueues_[index];
	}

	VkCommandPool& Device::commandPool()
	{
		return commandPool_;
	}

	bool Device::initCommandBuffers()
	{
		if (commandBuffers_.size() > 0)
			return true;

		uint32_t size = static_cast<uint32_t>(context()->swapChain().images().size());
		commandBuffers_.resize(size);

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool_;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = size;

		return vkAllocateCommandBuffers(*context()->device(), &allocInfo, commandBuffers_.data()) == VK_SUCCESS;
	}

	VkCommandBuffer Device::getCommandBuffer(size_t index)
	{
		if (index >= commandBuffers_.size())
			return VK_NULL_HANDLE;
		return commandBuffers_[index];
	}


	bool Device::record(size_t frameIndex, RecordCallback callback)
	{
		VkCommandBuffer buf = getCommandBuffer(frameIndex);

		if (buf == VK_NULL_HANDLE)
		{
			Logger::get()->error("Could not record!");
			return false;
		}

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(buf, &beginInfo) != VK_SUCCESS)
		{
			Logger::get()->error("failed to begin recording command buffer!");
			return false;
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = context()->renderPass();
		renderPassInfo.framebuffer = context()->swapChain().framebuffers()[frameIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = context()->swapChain().extent();

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(buf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(buf, VK_PIPELINE_BIND_POINT_GRAPHICS, *context()->pipeline());
		vkCmdDraw(buf, 3, 1, 0, 0);

		callback();

		vkCmdEndRenderPass(buf);

		if(vkEndCommandBuffer(buf) != VK_SUCCESS)
		{
			Logger::get()->error("Failed to end command buffer!");
			return false;
		}

		return true;
	}
};
