#ifndef ENGINE_GRAPHICS_SWAPCHAIN_HPP
#define ENGINE_GRAPHICS_SWAPCHAIN_HPP

#include "graphics/GfxObject.hpp"
#include "framework.hpp"

namespace NovaEngine::Graphics
{
	class SwapChain : public GfxObject<>
	{
	private:
		VkSurfaceCapabilitiesKHR capabilities_;
		std::optional<VkSurfaceFormatKHR> format_;
		std::optional<VkPresentModeKHR> presentMode_;
		VkExtent2D extent_; 
		VkSwapchainKHR swapChain_;

		GFX_CTOR(SwapChain),
			capabilities_(),
			format_(),
			presentMode_()
		{}

	protected:
		bool onInitialize();
		bool onTerminate();

	public:
		VkSurfaceCapabilitiesKHR& capabilities();
		VkSurfaceFormatKHR& format();
		VkPresentModeKHR& presentMode();
		VkExtent2D& extent();

		VkSwapchainKHR& operator*() { return swapChain_; }

	};
};

#endif