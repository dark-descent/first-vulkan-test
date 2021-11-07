#include "graphics/Device.hpp"
#include "graphics/Context.hpp"
#include "framework.hpp"

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

			if (vkCreateDevice(*pDev, &createInfo, nullptr, &device_) == VK_SUCCESS)
			{
				for (size_t i = 0; i < config->queueCount; i++)
					vkGetDeviceQueue(device_, graphicsQueueIndex, i, &graphicsQueues_[i]);

				presentQueue_ = graphicsQueues_[0];

				VkCommandPoolCreateInfo poolInfo = {};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();
				poolInfo.flags = 0;

				return vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) == VK_SUCCESS;
			}
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


			if (vkCreateDevice(*pDev, &createInfo, nullptr, &device_) == VK_SUCCESS)
			{
				for (size_t i = 0; i < config->queueCount; i++)
					vkGetDeviceQueue(device_, graphicsQueueIndex, i, &graphicsQueues_[i]);

				vkGetDeviceQueue(device_, presentQueueIndex, 0, &presentQueue_);

				VkCommandPoolCreateInfo poolInfo = {};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();
				poolInfo.flags = 0;

				return vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) == VK_SUCCESS;
			}

		}

		return false;
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
};
