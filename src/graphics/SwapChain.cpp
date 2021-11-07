#include "graphics/SwapChain.hpp"
#include "graphics/Context.hpp"

#include "framework.hpp"

namespace NovaEngine::Graphics
{
	bool SwapChain::onInitialize()
	{
		VkPhysicalDevice dev = *ctx_->physicalDevice();
		VkSurfaceKHR surface = ctx_->surface();

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surface, &capabilities_);

		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, nullptr);
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, formats.data());

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &presentModeCount, nullptr);
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &presentModeCount, presentModes.data());

		for (const auto& f : formats)
		{
			if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				format_ = f;
				break;
			}
		}

		if (!format_.has_value())
			format_ = formats[0];

		for (const auto& mode : presentModes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				presentMode_ = mode;
				break;
			}
		}

		if (!presentMode_.has_value())
			presentMode_ = VK_PRESENT_MODE_FIFO_KHR;

		if (capabilities_.currentExtent.width != UINT32_MAX)
		{
			extent_ = capabilities_.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(ctx_->window(), &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			extent_.width = std::clamp(actualExtent.width, capabilities_.minImageExtent.width, capabilities_.maxImageExtent.width);
			extent_.height = std::clamp(actualExtent.height, capabilities_.minImageExtent.height, capabilities_.maxImageExtent.height);
		}

		uint32_t imageCount = capabilities_.minImageCount + 1;

		if (capabilities_.maxImageCount > 0 && imageCount > capabilities_.maxImageCount)
			imageCount = capabilities_.maxImageCount;



		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = format_.value().format;
		createInfo.imageColorSpace = format_.value().colorSpace;
		createInfo.imageExtent = extent_;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		auto families = ctx_->physicalDevice().queueFamilies();

		uint32_t queueFamilyIndices[] = { families.graphicsFamily.value(), families.presentFamily.value() };

		if (families.graphicsFamily.value() == families.presentFamily.value())
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 1;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		createInfo.preTransform = capabilities_.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode_.value();
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		return vkCreateSwapchainKHR(*ctx_->device(), &createInfo, nullptr, &swapChain_) == VK_SUCCESS;
	}

	bool SwapChain::onTerminate()
	{
		vkDestroySwapchainKHR(*ctx_->device(), swapChain_, nullptr);
		return true;
	}


	VkSurfaceCapabilitiesKHR& SwapChain::capabilities()
	{
		return capabilities_;
	}

	VkSurfaceFormatKHR& SwapChain::format()
	{
		return format_.value();
	}

	VkPresentModeKHR& SwapChain::presentMode()
	{
		return presentMode_.value();
	}

	VkExtent2D& SwapChain::extent()
	{
		return extent_;
	}


};
