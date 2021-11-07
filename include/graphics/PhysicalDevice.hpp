#ifndef ENGINE_GRAPHICS_PHYSICAL_DEVICE_HPP
#define ENGINE_GRAPHICS_PHYSICAL_DEVICE_HPP

#include "graphics/GfxObject.hpp"
#include "framework.hpp"

namespace NovaEngine::Graphics
{
	class Context;

	struct QueueFamilies
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
	};

	typedef bool(*PhysicalDeviceConfigCallback)(VkPhysicalDeviceProperties& properties, VkPhysicalDeviceFeatures& features, QueueFamilies& queueFamilies);

	class PhysicalDevice : public GfxObject<PhysicalDeviceConfigCallback>
	{
	private:
		static bool defaultConfigCallback(VkPhysicalDeviceProperties& properties, VkPhysicalDeviceFeatures& features, QueueFamilies& queueFamilies);
		static uint32_t getQueueFamilies(VkPhysicalDevice& dev);
		static bool checkSwapchainSupport(VkPhysicalDevice dev, VkSurfaceKHR surface);

		VkPhysicalDevice physicalDevice_;
		QueueFamilies queueFamilies_;
		GFX_CTOR(PhysicalDevice), physicalDevice_(VK_NULL_HANDLE), queueFamilies_() {}

	protected:
		bool onInitialize(PhysicalDeviceConfigCallback callback);
		bool onTerminate();

	public:
		const QueueFamilies& queueFamilies();
		VkPhysicalDevice operator*() { return physicalDevice_; }
	};
};

#endif
