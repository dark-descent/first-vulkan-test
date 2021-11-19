#include "graphics/SwapChain.hpp"
#include "graphics/GraphicsManager.hpp"
#include "graphics/VkFactory.hpp"
#include "graphics/VkUtils.hpp"
#include "Logger.hpp"

namespace NovaEngine::Graphics
{
	SwapChainOptions SwapChain::defaultOptions = {};

	bool SwapChain::onInitialize(Context* context, SwapChainOptions* options)
	{
		if (options == nullptr)
			options = &defaultOptions;

		options_ = *options;

		context_ = context;

		SwapChainSupportDetails& ssd = context_->swapChainSupportDetails_;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context_->physicalDevice_, context_->surface_, &ssd.capabilities);

		presentMode_ = VkUtils::chooseSwapPresentMode(ssd.presentModes, options_.vSyncEnabled);

		printf(options_.vSyncEnabled ? "vsync enable\n" : "vsync disabled\n");

		extent_ = VkUtils::chooseSwapExtent(ssd.capabilities, context_->window_);

		if (options_.minFrames < ssd.capabilities.minImageCount)
			imageCount_ = ssd.capabilities.minImageCount;
		else
			imageCount_ = options_.minFrames;


		if (ssd.capabilities.maxImageCount > 0 && imageCount_ > ssd.capabilities.maxImageCount)
			imageCount_ = ssd.capabilities.maxImageCount;

		createInfo_.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo_.surface = context_->surface_;

		createInfo_.minImageCount = imageCount_;
		createInfo_.imageFormat = context_->surfaceFormat_.format;
		createInfo_.imageColorSpace = context_->surfaceFormat_.colorSpace;
		createInfo_.imageArrayLayers = 1;
		createInfo_.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilies& queueFamilies = context_->queueFamilies_;

		uint32_t queueFamilyIndices[] = { queueFamilies.graphics.value().index, queueFamilies.present.value().index, };

		if (queueFamilies.graphics.value().index != queueFamilies.present.value().index)
		{
			createInfo_.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo_.queueFamilyIndexCount = 2;
			createInfo_.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo_.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo_.preTransform = ssd.capabilities.currentTransform;
		createInfo_.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo_.clipped = VK_TRUE;

		imageViewCreateInfo_.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo_.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo_.format = context_->surfaceFormat_.format;
		imageViewCreateInfo_.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo_.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo_.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo_.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo_.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo_.subresourceRange.levelCount = 1;
		imageViewCreateInfo_.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo_.subresourceRange.layerCount = 1;
		return recreate();
	}

	void SwapChain::createFrameBuffers()
	{
		frameBuffers_.resize(imageViews_.size());

		for (size_t i = 0; i < imageViews_.size(); i++)
		{
			VkImageView attachments[] = { imageViews_[i] };

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = context_->renderPass_;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = extent_.width;
			framebufferInfo.height = extent_.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(context_->device_, &framebufferInfo, nullptr, &frameBuffers_[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create framebuffer!");
		}
	}

	void SwapChain::destroyFrameBuffers()
	{
		for (auto& fb : frameBuffers_)
			vkDestroyFramebuffer(context_->device_, fb, nullptr);
	}

	bool SwapChain::recreate(SwapChainOptions* options)
	{
		const bool recreate = swapChain_ != VK_NULL_HANDLE;

		if(options != nullptr)
			options_ = *options;

		SwapChainSupportDetails& ssd = context_->swapChainSupportDetails_;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context_->physicalDevice_, context_->surface_, &ssd.capabilities);

		presentMode_ = VkUtils::chooseSwapPresentMode(ssd.presentModes, options_.vSyncEnabled);
		extent_ = ssd.capabilities.currentExtent;

		createInfo_.imageExtent = extent_;
		createInfo_.presentMode = presentMode_;
		createInfo_.oldSwapchain = swapChain_;

		VkDevice& device = context_->device_;

		if (recreate)
		{
			Logger::get()->info("Recreating swapchain...");

			VkSwapchainKHR oldSwapChain = swapChain_;

			if (vkCreateSwapchainKHR(device, &createInfo_, nullptr, &swapChain_) != VK_SUCCESS)
				throw std::runtime_error("failed to create swap chain!");

			destroy(oldSwapChain);
		}
		else
		{
			if (vkCreateSwapchainKHR(device, &createInfo_, nullptr, &swapChain_) != VK_SUCCESS)
				throw std::runtime_error("failed to create swap chain!");

			vkGetSwapchainImagesKHR(device, swapChain_, &imageCount_, nullptr);
			images_.resize(imageCount_);
			imageViews_.resize(imageCount_);
		}

		vkGetSwapchainImagesKHR(device, swapChain_, &imageCount_, images_.data());

		for (size_t i = 0; i < imageCount_; i++)
		{
			imageViewCreateInfo_.image = images_[i];
			if (vkCreateImageView(device, &imageViewCreateInfo_, nullptr, &imageViews_[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create image views!");
		}

		createFrameBuffers();

		return true;
	}

	void SwapChain::destroy(VkSwapchainKHR swapChain)
	{
		destroyFrameBuffers();

		for (size_t i = 0; i < imageCount_; i++)
			vkDestroyImageView(context_->device_, imageViews_[i], nullptr);

		vkDestroySwapchainKHR(context_->device_, swapChain == VK_NULL_HANDLE ? swapChain_ : swapChain, nullptr);
	}
}
