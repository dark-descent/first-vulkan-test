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
		float queuePriority = 1.0f;

		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
		queueCreateInfo.queueCount = config->queueCount;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;

		if (vkCreateDevice(*pDev, &createInfo, nullptr, &device_) == VK_SUCCESS)
		{
			for(size_t i = 0; i < config->queueCount; i++)
				vkGetDeviceQueue(device_, graphicsQueueIndex, i, &graphicsQueues_[i]);
			return true;
		}

		return false;
	}

	bool Device::onTerminate()
	{
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
};
