#include "graphics/PhysicalDevice.hpp"
#include "graphics/Context.hpp"

namespace NovaEngine::Graphics
{

	bool PhysicalDevice::defaultConfigCallback(VkPhysicalDeviceProperties& properties, VkPhysicalDeviceFeatures& features, QueueFamilies& queueFamilies)
	{
		bool typeDiscrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		bool typeIntegrated = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;

		if ((typeDiscrete || typeIntegrated) && queueFamilies.graphicsFamily.has_value() && queueFamilies.presentFamily.has_value())
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
			// check for swapchain support
			if (!checkSwapchainSupport(device, ctx_->surface_))
				continue;

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamiliesList(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamiliesList.data());

			int i = 0;

			for (const auto& queueFamily : queueFamiliesList)
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					queueFamilies_.graphicsFamily = i;

				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, context()->surface(), &presentSupport);

				if (presentSupport)
					queueFamilies_.presentFamily = i;

				if (queueFamilies_.presentFamily.has_value() && queueFamilies_.graphicsFamily.has_value())
					break;

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

	bool PhysicalDevice::checkSwapchainSupport(VkPhysicalDevice dev, VkSurfaceKHR surface)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, availableExtensions.data());

		for (const VkExtensionProperties& extension : availableExtensions)
		{
			if (strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, extension.extensionName) == 0)
			{
				VkSurfaceCapabilitiesKHR capabilities;
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surface, &capabilities);

				uint32_t formatCount;
				vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, nullptr);

				uint32_t presentModeCount;
				vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &presentModeCount, nullptr);

				if (presentModeCount != 0 && formatCount != 0)
					return true;
			}
		}

		return false;
	}
};
