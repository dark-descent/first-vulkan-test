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

		VkDevice d = *ctx_->device();

		if (vkCreateSwapchainKHR(d, &createInfo, nullptr, &swapChain_) != VK_SUCCESS)
			return false;

		vkGetSwapchainImagesKHR(d, swapChain_, &imageCount, nullptr);
		images_.resize(imageCount);
		vkGetSwapchainImagesKHR(d, swapChain_, &imageCount, images_.data());

		imageViews_.resize(imageCount);
		for (size_t i = 0; i < imageCount; i++)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = images_[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = format().format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(d, &createInfo, nullptr, &imageViews_[i]) != VK_SUCCESS)
				return false;
		}

		return true;
	}

	bool SwapChain::onTerminate()
	{
		for (VkFramebuffer b : framebuffers_)
			vkDestroyFramebuffer(*ctx_->device(), b, nullptr);
	
		for (const VkImageView& v : imageViews_)
			vkDestroyImageView(*ctx_->device(), v, nullptr);

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

	bool SwapChain::initFrameBuffers()
	{
		framebuffers_.resize(imageViews_.size());

		for (size_t i = 0; i < imageViews_.size(); i++) {
			VkImageView attachments[] = {
				imageViews_[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = context()->renderPass();
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = extent_.width;
			framebufferInfo.height = extent_.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(*context()->device(), &framebufferInfo, nullptr, &framebuffers_[i]) != VK_SUCCESS)
				return false;
		}

		return true;
	}
};
