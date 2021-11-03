#include "graphics/SwapChain.hpp"
#include "graphics/Context.hpp"

namespace NovaEngine::Graphics
{
	bool SwapChain::getSupportDetails(VkSurfaceKHR surface, VkPhysicalDevice device, SupportDetails* details)
	{
		if (surface == nullptr)
			return false;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details->capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, VK_NULL_HANDLE);

		if (formatCount != 0)
		{
			details->formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details->formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, VK_NULL_HANDLE);

		if (presentModeCount != 0)
		{
			details->presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details->presentModes.data());
		}

		return true;
	}

	VkSurfaceFormatKHR SwapChain::getSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		return availableFormats[0];
	}

	VkPresentModeKHR SwapChain::getPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return availablePresentMode;

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChain::getExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}


	SwapChain::SwapChain() : swapChain_(VK_NULL_HANDLE), device_(VK_NULL_HANDLE), imageFormat_(), extent_(), images_(), imageViews_() {  }


	bool SwapChain::onInitialize(GLFWwindow* window, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device)
	{
		SupportDetails supportDetails = {};

		if (!getSupportDetails(surface, physicalDevice, &supportDetails))
			return false;

		VkSurfaceFormatKHR surfaceFormat = getSurfaceFormat(supportDetails.formats);
		VkPresentModeKHR presentMode = getPresentMode(supportDetails.presentModes);
		extent_ = getExtent(window, supportDetails.capabilities);

		uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;

		if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount)
			imageCount = supportDetails.capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent_;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices;

		Context::getQueueFamilies(physicalDevice, surface, &indices);

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
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = supportDetails.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain_) == VK_SUCCESS)
		{
			device_ = device;
			imageFormat_ = surfaceFormat.format;

			vkGetSwapchainImagesKHR(device, swapChain_, &imageCount, nullptr);
			images_.resize(imageCount);
			imageViews_.resize(imageCount);
			vkGetSwapchainImagesKHR(device, swapChain_, &imageCount, images_.data());

			for (size_t i = 0; i < imageCount; i++)
			{
				VkImageViewCreateInfo createInfo = {};

				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = images_[i];
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

				if (vkCreateImageView(device, &createInfo, nullptr, &imageViews_[i]) != VK_SUCCESS)
					return false;
			}

			return true;
		}
		return true;
	}

	bool SwapChain::onTerminate()
	{
		for (const VkImageView& imageView : imageViews_)
			vkDestroyImageView(device_, imageView, nullptr);

		vkDestroySwapchainKHR(device_, swapChain_, nullptr);
		
		return true;
	}
};
