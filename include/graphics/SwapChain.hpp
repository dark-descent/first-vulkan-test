#ifndef ENGINE_GRAPHICS_SWAPCHAIN_HPP
#define ENGINE_GRAPHICS_SWAPCHAIN_HPP

#include "framework.hpp"
#include "Initializable.hpp"

namespace NovaEngine::Graphics
{
	class GraphicsManager;
	class Context;

	struct SwapChainOptions
	{
		bool vSyncEnabled = false;
		size_t maxFrames = 3;
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class SwapChain : public Initializable<Context*, SwapChainOptions*>
	{
		static SwapChainOptions defaultOptions;

		Context* context_;
		VkPresentModeKHR presentMode_;
		VkExtent2D extent_;
		VkSwapchainKHR swapChain_;
		uint32_t imageCount_;
		std::vector<VkImage> images_;
		std::vector<VkImageView> imageViews_;
		std::vector<VkFramebuffer> frameBuffers_;

		VkSwapchainCreateInfoKHR createInfo_;
		VkImageViewCreateInfo imageViewCreateInfo_;

		void createFrameBuffers();
		void destroyFrameBuffers();

	protected:
		bool onInitialize(Context* context, SwapChainOptions* options = &defaultOptions);

	public:
		SwapChain() : Initializable(),
			context_(nullptr),
			presentMode_(),
			extent_(),
			swapChain_(VK_NULL_HANDLE),
			imageCount_(0),
			images_(),
			imageViews_(),
			frameBuffers_(),
			createInfo_(),
			imageViewCreateInfo_()
		{}

		uint32_t imageCount() { return imageCount_; }

		bool recreate(SwapChainOptions* options = nullptr);
		void destroy(VkSwapchainKHR swapChain = VK_NULL_HANDLE);

		const VkSwapchainKHR& get() { return swapChain_; }

		friend class GraphicsManager;
		friend class Context;
	};
}

#endif
