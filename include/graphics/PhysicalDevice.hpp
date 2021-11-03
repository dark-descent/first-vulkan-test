#ifndef ENGINE_GRAPHICS_PHYSICAL_DEVICE_HPP
#define ENGINE_GRAPHICS_PHYSICAL_DEVICE_HPP

#include "framework.hpp"
#include "AbstractObject.hpp"


namespace NovaEngine::Graphics
{
	class Context;

	/**
	 * The PhysicalDevice class is a wrapper around an VkPhysicalDevice
	 */
	class PhysicalDevice : public AbstractObject<VkInstance, VkSurfaceKHR, const std::vector<const char*>&>
	{
	public:
		VkPhysicalDevice operator * () { return this->physicalDevice_; };

	protected:
		bool onInitialize(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions);
		bool onTerminate();
		// bool isDeviceSuitable(const std::vector<const char*>& requiredExtensions);

	private:
		PhysicalDevice();

		VkPhysicalDevice physicalDevice_;

		inline bool checkDeviceExtensionSupport(const std::vector<const char*>& deviceExtensions);
		inline bool isDeviceSuitable(VkSurfaceKHR surface, const std::vector<const char*>& requiredExtensions);
		// inline bool querySwapChainSupport(VkSurfaceKHR surface, SwapChainSupportDetails* details);

		friend class Context;
	};
};

#endif
