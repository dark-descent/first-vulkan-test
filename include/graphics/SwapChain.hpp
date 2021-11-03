#ifndef ENGINE_GRAPHICS_SWAP_CHAIN_HPP
#define ENGINE_GRAPHICS_SWAP_CHAIN_HPP

#include "framework.hpp"
#include "AbstractObject.hpp"


namespace NovaEngine::Graphics
{
	class Context;

	class SwapChain : public AbstractObject<GLFWwindow*, VkSurfaceKHR, VkPhysicalDevice, VkDevice>
	{
	public:
		struct SupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		static bool getSupportDetails(VkSurfaceKHR surface, VkPhysicalDevice device, SupportDetails* supportDetails);

		static VkSurfaceFormatKHR getSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR getPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		static VkExtent2D getExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);


	protected:
		bool onInitialize(GLFWwindow* window, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device);
		bool onTerminate();

	private:
		VkSwapchainKHR swapChain_;
		VkDevice device_;
		VkFormat imageFormat_;
		VkExtent2D extent_;
		std::vector<VkImage> images_;
		std::vector<VkImageView> imageViews_;

		SwapChain();

		friend class Context;
	};
};

#endif
