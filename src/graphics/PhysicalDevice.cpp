#include "graphics/PhysicalDevice.hpp"
#include "graphics/Context.hpp"

namespace NovaEngine::Graphics
{
	bool PhysicalDevice::defaultConfigCallback(VkPhysicalDeviceProperties& properties, VkPhysicalDeviceFeatures& features, QueueFamilies& queueFamilies)
	{
		bool typeDiscrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		bool typeIntegrated = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
		bool typeCpu = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;

		if ((typeDiscrete || typeIntegrated || typeCpu) && queueFamilies.graphicsFamily.has_value())
			return true;

		return false;
	}

	bool PhysicalDevice::onInitialize(PhysicalDeviceConfigCallback configCallback)
	{
		if (configCallback == nullptr)
			configCallback = defaultConfigCallback;

		VkInstance inst = ctx_->instance_;

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(inst, &deviceCount, nullptr);

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(inst, &deviceCount, devices.data());

		for (auto device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamiliesList(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamiliesList.data());

			;

			int i = 0;

			for (const auto& queueFamily : queueFamiliesList)
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					queueFamilies_.graphicsFamily = i;
					break;
				}

				i++;
			}

			if (configCallback(deviceProperties, deviceFeatures, queueFamilies_))
			{
				physicalDevice_ = device;
				break;
			}
		}

		return physicalDevice_ != VK_NULL_HANDLE;
	}

	bool PhysicalDevice::onTerminate()
	{
		return true;
	}

	const QueueFamilies& PhysicalDevice::queueFamilies()
	{
		return queueFamilies_;
	}
};
