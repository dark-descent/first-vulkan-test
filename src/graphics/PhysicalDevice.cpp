#include "graphics/PhysicalDevice.hpp"
#include "graphics/Context.hpp"
#include "graphics/SwapChain.hpp"

namespace NovaEngine::Graphics
{

	PhysicalDevice::PhysicalDevice() : physicalDevice_(VK_NULL_HANDLE)
	{

	}

	bool PhysicalDevice::onInitialize(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, VK_NULL_HANDLE);

		if (deviceCount == 0)
			return false;

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		auto checkCallback = [](VkPhysicalDevice dev) {
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceProperties(dev, &deviceProperties);
			vkGetPhysicalDeviceFeatures(dev, &deviceFeatures);

			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				return deviceFeatures.geometryShader != 0;

			return false;
		};

		for (auto device : devices)
		{
			if (checkCallback(device))
			{
				physicalDevice_ = device;
				break;
			}
		}

		CHECK_OR_RETURN(isDeviceSuitable(surface, requiredExtensions), "Device is not suitable!");

		return true;
	}

	bool PhysicalDevice::onTerminate()
	{
		return true;
	}

	bool PhysicalDevice::checkDeviceExtensionSupport(const std::vector<const char*>& deviceExtensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, VK_NULL_HANDLE);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	bool PhysicalDevice::isDeviceSuitable(VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions)
	{
		QueueFamilyIndices indices;
		Context::getQueueFamilies(physicalDevice_, surface, &indices);

		if (!checkDeviceExtensionSupport(requiredExtensions))
			return false;

		SwapChain::SupportDetails swapChainSupport = {};

		if (!SwapChain::getSupportDetails(surface, physicalDevice_, &swapChainSupport))
			return false;

		if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
			return false;

		return indices.isComplete();
	}
};
